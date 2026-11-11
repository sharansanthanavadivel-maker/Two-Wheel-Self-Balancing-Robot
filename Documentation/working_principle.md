# Working principle

1. MPU6050 measures linear acceleration and angular velocity.
2. The accelerometer provides a gravity-referenced tilt estimate.
3. The gyroscope provides short-term angular rate.
4. A complementary filter combines the two measurements to estimate pitch.
5. The PID controller compares estimated pitch with the target upright angle.
6. The controller output is limited to the motor command range and applied to the L298N using direction GPIOs and PWM enable pins.
7. If the robot tilts beyond the configured safety limit or IMU communication fails, the motors are stopped.

The implementation is a baseline balancing controller. It does not include wheel encoders, odometry, wireless control, battery management or measured performance claims.
