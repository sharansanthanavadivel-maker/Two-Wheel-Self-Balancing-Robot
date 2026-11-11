# Two-Wheel Self-Balancing Robot

**Platform:** Arduino Uno  
**IMU:** MPU6050 (I2C)  
**Motor driver:** L298N dual H-bridge  
**Language:** Embedded C / Arduino C++

## Project overview
A two-wheel self-balancing robot using MPU6050 inertial feedback and a PID controller. The Arduino reads accelerometer and gyroscope data, estimates body tilt with a complementary filter, and adjusts the two DC motors through an L298N driver to keep the chassis near its calibrated upright angle.

This repository is based on the project description in the resume: *PID control in Embedded C driven by IMU feedback, with DC motor drivers and extensive controller tuning.* The exact hardware below (Arduino Uno, MPU6050 and L298N) was specified separately for this repository.

## Features
- MPU6050 raw accelerometer and gyroscope acquisition over I2C
- Gyroscope bias calibration at startup
- Complementary-filter angle estimation
- PID feedback control with output limiting and integral limiting
- Independent left/right motor direction and PWM control
- Tilt safety cutoff and startup-upright check
- Serial telemetry for tuning

## Repository structure
- `Arduino/SelfBalancingRobot.ino` — complete integrated firmware (Wire library only)
- `Documentation/` — operating, calibration and tuning guides
- `Hardware/` — wiring and bill of materials

## Hardware
- Arduino Uno
- MPU6050 breakout (GY-521 or equivalent)
- L298N dual H-bridge module
- Two geared DC motors and wheels
- Robot chassis, battery suitable for the motors and driver
- Jumper wires and common ground

## Wiring
See `Hardware/wiring.md`. Confirm your particular L298N module's ENA/ENB jumper arrangement and motor supply requirements before powering it.

## Getting started
1. Assemble the wiring with the motor battery disconnected.
2. Install Arduino IDE.
3. Open `Arduino/SelfBalancingRobot.ino`.
4. Select **Arduino Uno** and the correct serial port.
5. Upload while the robot is supported so its wheels cannot drive away.
6. Open Serial Monitor at **115200 baud**.
7. Keep the robot upright and still during startup gyro calibration.
8. Follow `Documentation/calibration_and_tuning.md` before attempting a floor test.

## Important safety notes
- Start with wheels off the ground and a current-limited/suitable motor supply.
- Keep hands, hair and loose objects away from wheels and gears.
- The robot must be physically supported during first power-up and tuning.
- Tune gains gradually. Incorrect gains can cause sudden acceleration or violent oscillation.
- The MPU6050 board orientation and motor polarity must be checked.
- The tilt cutoff is a software safeguard, not a substitute for a physical power switch.
- This firmware is a starter implementation and has not been compiled or validated on the user's physical hardware. Hardware-specific tuning is required.

## Resume alignment
The implementation demonstrates the resume-listed concepts: Embedded C, IMU feedback, PID control, DC motor driver control and controller tuning. Do not claim measured performance or hardware validation unless you have personally tested and measured it.
