/*
 * Two-Wheel Self-Balancing Robot
 * Arduino Uno + MPU6050 + L298N
 *
 * Uses only the built-in Wire library.
 * IMPORTANT: Verify IMU orientation, motor polarity and angle offset.
 * Initial gains are conservative starting values, not universal settings.
 */
#include <Wire.h>

// ---------------- Pin mapping: Arduino Uno ----------------
const uint8_t MPU_ADDR = 0x68;
const uint8_t LEFT_EN  = 5;   // PWM
const uint8_t LEFT_IN1 = 7;
const uint8_t LEFT_IN2 = 8;
const uint8_t RIGHT_EN = 6;   // PWM
const uint8_t RIGHT_IN1 = 9;
const uint8_t RIGHT_IN2 = 10;

// ---------------- Sensor and control configuration ----------------
const float RAD_TO_DEG_F = 57.2957795f;
const float ANGLE_OFFSET_DEG = 0.0f;  // calibrate for your assembled chassis
const float FALL_ANGLE_DEG = 35.0f;   // stop motors beyond this tilt
const float STARTUP_UPRIGHT_LIMIT_DEG = 12.0f;
const uint16_t CONTROL_PERIOD_US = 5000; // 200 Hz nominal loop

// PID starting values: tune on the actual robot.
float Kp = 18.0f;
float Ki = 0.5f;
float Kd = 0.8f;
float targetAngleDeg = 0.0f;

// MPU6050 scale factors for ±2g and ±250 deg/s.
const float ACCEL_LSB_PER_G = 16384.0f;
const float GYRO_LSB_PER_DPS = 131.0f;
const float G = 9.80665f;

float gyroBiasY = 0.0f;
float angleDeg = 0.0f;
float integral = 0.0f;
float previousError = 0.0f;
unsigned long lastLoopUs = 0;
unsigned long lastTelemetryMs = 0;
bool sensorReady = false;
bool calibrated = false;

// Set these true/false after checking whether each wheel moves in the
// direction that corrects a forward/backward fall.
const bool LEFT_MOTOR_INVERTED = false;
const bool RIGHT_MOTOR_INVERTED = false;

struct ImuSample {
  int16_t ax, ay, az;
  int16_t gx, gy, gz;
};

bool writeRegister(uint8_t reg, uint8_t value) {
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(reg);
  Wire.write(value);
  return Wire.endTransmission() == 0;
}

bool readImu(ImuSample &s) {
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x3B); // ACCEL_XOUT_H
  if (Wire.endTransmission(false) != 0) return false;
  if (Wire.requestFrom(MPU_ADDR, (uint8_t)14, (uint8_t)true) != 14) return false;

  s.ax = (int16_t)((Wire.read() << 8) | Wire.read());
  s.ay = (int16_t)((Wire.read() << 8) | Wire.read());
  s.az = (int16_t)((Wire.read() << 8) | Wire.read());
  (void)Wire.read(); (void)Wire.read(); // temperature
  s.gx = (int16_t)((Wire.read() << 8) | Wire.read());
  s.gy = (int16_t)((Wire.read() << 8) | Wire.read());
  s.gz = (int16_t)((Wire.read() << 8) | Wire.read());
  return true;
}

float accelTiltDegrees(const ImuSample &s) {
  // Assumes robot pitch is measured around the Y axis.
  // Change axes/sign if your MPU6050 is mounted differently.
  return atan2f((float)s.ax, (float)s.az) * RAD_TO_DEG_F;
}

bool initMpu() {
  // Wake MPU6050; set gyro ±250 dps and accel ±2g.
  if (!writeRegister(0x6B, 0x00)) return false;
  delay(100);
  if (!writeRegister(0x1B, 0x00)) return false;
  if (!writeRegister(0x1C, 0x00)) return false;
  // DLPF config for reduced high-frequency noise.
  if (!writeRegister(0x1A, 0x03)) return false;
  return true;
}

bool calibrateGyro() {
  const uint16_t samples = 600;
  long sumGy = 0;
  ImuSample s;
  Serial.println(F("Keep robot still and upright: calibrating gyro..."));
  for (uint16_t i = 0; i < samples; ++i) {
    if (!readImu(s)) return false;
    sumGy += s.gy;
    delay(3);
  }
  gyroBiasY = (float)sumGy / samples;
  if (!readImu(s)) return false;
  angleDeg = accelTiltDegrees(s) - ANGLE_OFFSET_DEG;
  Serial.print(F("Gyro Y bias raw: "));
  Serial.println(gyroBiasY);
  Serial.print(F("Initial tilt deg: "));
  Serial.println(angleDeg);
  return true;
}

void setOneMotor(uint8_t en, uint8_t in1, uint8_t in2,
                 int command, bool inverted) {
  if (inverted) command = -command;
  command = constrain(command, -255, 255);
  if (command > 0) {
    digitalWrite(in1, HIGH);
    digitalWrite(in2, LOW);
    analogWrite(en, command);
  } else if (command < 0) {
    digitalWrite(in1, LOW);
    digitalWrite(in2, HIGH);
    analogWrite(en, -command);
  } else {
    analogWrite(en, 0);
    digitalWrite(in1, LOW);
    digitalWrite(in2, LOW);
  }
}

void stopMotors() {
  setOneMotor(LEFT_EN, LEFT_IN1, LEFT_IN2, 0, false);
  setOneMotor(RIGHT_EN, RIGHT_IN1, RIGHT_IN2, 0, false);
}

void driveMotors(int command) {
  // Equal command is a basic balancing scaffold. Trim can be added after
  // confirming both motors and wheel diameters are mechanically matched.
  setOneMotor(LEFT_EN, LEFT_IN1, LEFT_IN2, command, LEFT_MOTOR_INVERTED);
  setOneMotor(RIGHT_EN, RIGHT_IN1, RIGHT_IN2, command, RIGHT_MOTOR_INVERTED);
}

void resetPid() {
  integral = 0.0f;
  previousError = 0.0f;
}

void setup() {
  pinMode(LEFT_EN, OUTPUT); pinMode(LEFT_IN1, OUTPUT); pinMode(LEFT_IN2, OUTPUT);
  pinMode(RIGHT_EN, OUTPUT); pinMode(RIGHT_IN1, OUTPUT); pinMode(RIGHT_IN2, OUTPUT);
  stopMotors();

  Serial.begin(115200);
  Wire.begin();
  Wire.setClock(400000);

  sensorReady = initMpu();
  if (!sensorReady) {
    Serial.println(F("ERROR: MPU6050 not responding at address 0x68."));
    stopMotors();
    while (1) { delay(500); }
  }

  calibrated = calibrateGyro();
  if (!calibrated) {
    Serial.println(F("ERROR: IMU read failed during calibration."));
    stopMotors();
    while (1) { delay(500); }
  }

  if (fabs(angleDeg) > STARTUP_UPRIGHT_LIMIT_DEG) {
    Serial.println(F("Place robot upright and reset."));
    stopMotors();
    while (1) { delay(500); }
  }

  lastLoopUs = micros();
  Serial.println(F("Ready. Lift wheels clear for initial testing."));
}

void loop() {
  unsigned long nowUs = micros();
  if ((uint32_t)(nowUs - lastLoopUs) < CONTROL_PERIOD_US) return;

  float dt = (nowUs - lastLoopUs) / 1000000.0f;
  lastLoopUs = nowUs;
  if (dt <= 0.0f || dt > 0.03f) {
    stopMotors();
    resetPid();
    return;
  }

  ImuSample sample;
  if (!readImu(sample)) {
    stopMotors();
    resetPid();
    Serial.println(F("ERROR: IMU read failure; motors stopped."));
    return;
  }

  float accelAngle = accelTiltDegrees(sample) - ANGLE_OFFSET_DEG;
  float gyroRateDps = ((float)sample.gy - gyroBiasY) / GYRO_LSB_PER_DPS;

  // Complementary filter: gyro responds quickly; accelerometer corrects drift.
  const float alpha = 0.98f;
  angleDeg = alpha * (angleDeg + gyroRateDps * dt)
           + (1.0f - alpha) * accelAngle;

  if (fabs(angleDeg) > FALL_ANGLE_DEG) {
    stopMotors();
    resetPid();
    Serial.println(F("SAFETY STOP: tilt limit exceeded. Reset upright."));
    while (1) { delay(100); }
  }

  float error = targetAngleDeg - angleDeg;
  integral += error * dt;
  integral = constrain(integral, -80.0f, 80.0f);
  float derivative = (error - previousError) / dt;
  previousError = error;

  float output = Kp * error + Ki * integral + Kd * derivative;
  int motorCommand = (int)constrain(output, -255.0f, 255.0f);

  // Small deadband helps prevent constant motor buzzing near balance.
  if (fabs(error) < 0.35f && fabs(gyroRateDps) < 2.0f) motorCommand = 0;

  driveMotors(motorCommand);

  if (millis() - lastTelemetryMs >= 100) {
    lastTelemetryMs = millis();
    Serial.print(F("angle=")); Serial.print(angleDeg, 2);
    Serial.print(F(",gyro_dps=")); Serial.print(gyroRateDps, 2);
    Serial.print(F(",error=")); Serial.print(error, 2);
    Serial.print(F(",pid=")); Serial.println(motorCommand);
  }
}
