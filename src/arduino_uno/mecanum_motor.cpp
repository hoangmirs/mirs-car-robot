#include "mecanum_motor.h"

MecanumMotor::MecanumMotor(uint8_t pwm1_pin, uint8_t pwm2_pin, uint8_t shcp_pin,
                          uint8_t en_pin, uint8_t data_pin, uint8_t stcp_pin) {
    _pwm1_pin = pwm1_pin;
    _pwm2_pin = pwm2_pin;
    _shcp_pin = shcp_pin;
    _en_pin = en_pin;
    _data_pin = data_pin;
    _stcp_pin = stcp_pin;
}

void MecanumMotor::begin() {
    pinMode(_shcp_pin, OUTPUT);
    pinMode(_en_pin, OUTPUT);
    pinMode(_data_pin, OUTPUT);
    pinMode(_stcp_pin, OUTPUT);
    pinMode(_pwm1_pin, OUTPUT);
    pinMode(_pwm2_pin, OUTPUT);
}

void MecanumMotor::drive(int direction, int speed) {
    digitalWrite(_en_pin, LOW);
    analogWrite(_pwm1_pin, speed);
    analogWrite(_pwm2_pin, speed);

    digitalWrite(_stcp_pin, LOW);
    shiftOut(_data_pin, _shcp_pin, MSBFIRST, direction);
    digitalWrite(_stcp_pin, HIGH);
}
