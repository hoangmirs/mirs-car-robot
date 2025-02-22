#ifndef MECANUM_MOTOR_H
#define MECANUM_MOTOR_H

#include <Arduino.h>

class MecanumMotor {
public:
    // Constants for movement directions
    static const int Forward = 92;       // forward
    static const int Backward = 163;     // back
    static const int Turn_Left = 149;    // left translation
    static const int Turn_Right = 106;   // right translation
    static const int Top_Left = 20;      // upper left mobile
    static const int Bottom_Left = 129;  // lower left mobile
    static const int Top_Right = 72;     // upper right mobile
    static const int Bottom_Right = 34;  // lower right move
    static const int Stop = 0;           // stop
    static const int Contrarotate = 172; // counterclockwise rotation
    static const int Clockwise = 83;     // rotate clockwise

    MecanumMotor(uint8_t pwm1_pin, uint8_t pwm2_pin, uint8_t shcp_pin,
                 uint8_t en_pin, uint8_t data_pin, uint8_t stcp_pin);
    void begin();
    void drive(int direction, int speed);

private:
    uint8_t _pwm1_pin;
    uint8_t _pwm2_pin;
    uint8_t _shcp_pin;
    uint8_t _en_pin;
    uint8_t _data_pin;
    uint8_t _stcp_pin;
};

#endif
