#ifndef PINNAMES_H
#define PINNAMES_H

#include "./Pins.h"

//Teensy Pins
#define CYLINDER_MOTOR_PIN Pins::motors.m2     

#define FIRING_PIN_1 Pins::pnuematics.p3
#define FIRING_PIN_2 Pins::pnuematics.p4

//Solenoid that locks the chamber in place
#define CYLINDER_LOCK_PIN Pins::pnuematics.p1

#define FIRING_PLATE_PIN Pins::pnuematics.p2

#define ELEVATION_MOTOR_PIN Pins::motors.m1

//Empty Pins for Controls Use
//#define TERM_AUX_1 18
#define SIREN_PIN 20
//#define TERM_AUX_3 21

#define BUILT_IN_LED 13

//Signal pins for A/B share pins with gadgeteer port
#define ENCODER_1_A_PIN Pins::quad1.a
#define ENCODER_1_B_PIN Pins::quad1.b
#define ENCODER_2_A_PIN Pins::quad2.a
#define ENCODER_2_B_PIN Pins::quad2.b

//Gadgeteer ports share A/B pin with other header
// #define GADGETEER_PORT_1_PWM 1
// #define GADGETEER_1_ENC_A_PIN ENCODER_1_A_PIN
// #define GADGETEER_1_ENC_B_PIN ENCODER_1_B_PIN
// #define GADGETEER_2_PWM_PIN 12
// #define GADGETEER_2_ENC_A_PIN ENCODER_2_A_PIN
// #define GADGETEER_2_ENC_B_PIN ENCODER_2_B_PIN

#define ABSOLUTE_ENCODER_PIN ENCODER_1_B_PIN //pwm pin
#define BARREL_ABSOLUTE_ENCODER_PIN GADGETEER_2_PWM_PIN //pwm pin

#endif
