# Calibration and PID tuning

## Before power-up
1. Check all wiring and common ground.
2. Confirm the MPU6050 breakout's voltage compatibility.
3. Confirm motor polarity and L298N enable jumper configuration.
4. Keep wheels raised and chassis physically supported.

## IMU calibration
1. Place the robot upright and completely still.
2. Reset/power the Arduino.
3. At startup, the firmware samples gyro Y 600 times to estimate bias.
4. Read the initial tilt printed to Serial Monitor (115200 baud).
5. If the chassis is physically upright but the estimated angle is not near zero, adjust `ANGLE_OFFSET_DEG` and/or sensor axes in the code.

## Motor direction check
With wheels raised, use a very low safe gain/output test. When the chassis is tilted forward, the wheels must move in the direction that brings the base under the centre of mass. If correction is opposite, invert the relevant motor direction using `LEFT_MOTOR_INVERTED` and `RIGHT_MOTOR_INVERTED`, or correct the sensor sign. Never proceed to floor testing until this is correct.

## PID tuning
The included Kp=18.0, Ki=0.5, Kd=0.8 are placeholders only.
1. Start with Ki=0 and low Kp.
2. Increase Kp until the robot begins to respond strongly, but stop before sustained oscillation.
3. Add Kd gradually to damp oscillation.
4. Add only a small Ki if a persistent steady-state bias remains.
5. Change one gain at a time and record results.
6. Use a physical kill switch and keep a person ready to support the chassis.

## Safety behaviour
- Startup requires the robot to be within `STARTUP_UPRIGHT_LIMIT_DEG`.
- If tilt exceeds `FALL_ANGLE_DEG`, motors stop and firmware remains latched until reset.
- IMU read errors stop the motors.
- Software safeguards do not replace a physical power disconnect.
