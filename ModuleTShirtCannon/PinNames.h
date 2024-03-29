
//Teensy Pins
#define CYLINDER_MOTOR_PIN 10     

#define FIRING_PIN_1 4
#define FIRING_PIN_2 5

//Solenoid that locks the chamber in place
#define CYLINDER_LOCK_PIN 7 

#define FIRING_PLATE_PIN 6

#define ELEVATION_MOTOR_PIN 9

//Empty Pins for Controls Use
//#define TERM_AUX_1 18
#define SIREN_PIN 20
//#define TERM_AUX_3 21

#define BUILT_IN_LED 13

//RADIO INPUT 23
#define RADIO_IN_PIN 23

#define RADIO_OUT_PIN 22

//Signal pins for A/B share pins with gadgeteer port
#define ENCODER_1_A_PIN 0
#define ENCODER_1_B_PIN 15
#define ENCODER_2_A_PIN 11
#define ENCODER_2_B_PIN 8

//Gadgeteer ports share A/B pin with other header
#define GADGETEER_PORT_1_PWM 1
#define GADGETEER_1_ENC_A_PIN ENCODER_1_A_PIN
#define GADGETEER_1_ENC_B_PIN ENCODER_1_B_PIN
#define GADGETEER_2_PWM_PIN 12
#define GADGETEER_2_ENC_A_PIN ENCODER_2_A_PIN
#define GADGETEER_2_ENC_B_PIN ENCODER_2_B_PIN

#define ABSOLUTE_ENCODER_PIN ENCODER_1_A_PIN //pwm pin
#define BARREL_ABSOLUTE_ENCODER_PIN GADGETEER_2_PWM_PIN //pwm pin


