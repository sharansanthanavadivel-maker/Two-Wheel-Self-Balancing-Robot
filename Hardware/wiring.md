# Wiring guide

**Disconnect motor battery while wiring.** Keep all grounds common.

## MPU6050 to Arduino Uno
| MPU6050 | Arduino Uno |
|---|---|
| VCC | 5V only if breakout explicitly supports 5V; otherwise 3.3V |
| GND | GND |
| SDA | A4 |
| SCL | A5 |

The bare MPU6050 IC is not 5V tolerant. Many GY-521 modules include regulation/level shifting, but verify your board.

## L298N to Arduino Uno
| L298N | Arduino Uno |
|---|---|
| ENA | D5 (PWM) |
| IN1 | D7 |
| IN2 | D8 |
| ENB | D6 (PWM) |
| IN3 | D9 |
| IN4 | D10 |
| GND | GND |
| Motor supply `+12V` / `Vs` | Appropriate motor battery positive |
| `5V` logic | Follow the exact L298N module's regulator/jumper instructions |

- Left motor: OUT1 and OUT2.
- Right motor: OUT3 and OUT4.
- Arduino GND, L298N GND and battery negative must be common.
- Remove ENA/ENB jumpers if controlling enable pins with Arduino PWM. If jumpers remain installed, PWM control from D5/D6 will not work as intended.
- Do not power motors from the Arduino 5V pin.
- Check the L298N module's voltage drop, current rating and thermal limits.

## Mechanical orientation
Mount the MPU6050 firmly near the chassis centre. The supplied code assumes pitch is calculated from accelerometer X and Z and gyro Y is the pitch rate. If the board is rotated, change the axes/sign in `accelTiltDegrees()` and gyro-rate calculation. Confirm the correction direction with wheels raised before floor testing.
