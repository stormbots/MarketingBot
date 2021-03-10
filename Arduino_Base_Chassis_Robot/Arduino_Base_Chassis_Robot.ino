
#include <PulsePosition.h>
#include <Servo.h>

//LED Pins
//These assignments cannot change: Hardware defined
/*
2 	LED Strip #1
14	LED Strip #2
7	  LED Strip #3
8	  LED Strip #4
6	  LED Strip #5
20	LED Strip #6
21	LED Strip #7
5 	LED Strip #8
//*/

//Available pins for PulsePosition
/* Teensy 4.0+4.1
9, 10, 11, 12, 13, 15, 18, 19
//*/
/* Teensy 3.2+3.1 
9, 10, 22, 23
//*/

/* Teensy 3.2 PWM pins remaining 
10, 9, 4, 3, 23, 22, 
//*/
/* Teensy 4.0 PWM pins remaining
0, 1, 3, 4, 9, 10, 11, 12, 13, 15, 18, 19, 22, 23,
24, 25, 28, 29, 33, 34, 35, 36, 37, 38, 39
//*/

// Pins chosen for maximum compatibility with 3.2 and 4.0, whichever we use
#define RADIO_IN_PIN 22
#define RADIO_OUT_PIN 23

#define MOTOR_LEFT_1_PIN 9
#define MOTOR_LEFT_2_PIN 9
#define MOTOR_LEFT_3_PIN 10
#define MOTOR_RIGHT_1_PIN 3
#define MOTOR_RIGHT_2_PIN 3
#define MOTOR_RIGHT_3_PIN 4


PulsePositionOutput radioOutput;
PulsePositionInput radioInput;
/*****************************************/

//TODO: Make controller sketch



/*****************************************/



void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  radioOutput.begin(RADIO_OUT_PIN);
  radioInput.begin(RADIO_IN_PIN);

  light_setup();
}

void loop() {
  // put your main code here, to run repeatedly:
  float leftMotorSpeed = 1500;
  float rightMotorSpeed = 1500;

  // float ??? = radioInput.read(1);
  float throttleValue = radioInput.read(2);
  // float ??? = radioInput.read(3);
  float turningValue= radioInput.read(4); 
  float chassisPowerValue = radioInput.read(5);
  // float ??? = radioInput.read(6);
  // float ??? = radioInput.read(7);
  // float ??? = radioInput.read(8);
  // float ??? = radioInput.read(3);
   
  throttleValue = constrain(throttleValue,1000,2000);
  turningValue = constrain(turningValue,1000,2000);
  chassisPowerValue = constrain(chassisPowerValue,1000,2000);
  
  /** Handle Low-power and disable switch */
  if (chassisPowerValue <=1250){
    //Disable drivetrain
    throttleValue = 1500;
    turningValue = 1500;
  }
  else if(chassisPowerValue <= 1750){
  //Slow Mode
    throttleValue = map(throttleValue, 1000,2000,1250,1750);
    turningValue = map(turningValue, 1500,2000,1500,1750);
  }
  else{
    //Leave signal alone
  }

  //Copy radio signal to output
  //TODO : Add this.


  /** Generate arcade drive left/right outputs */
  turningValue = map(turningValue,1000,2000,-2000,2000);
  leftMotorSpeed= throttleValue + turningValue;
  rightMotorSpeed = throttleValue - turningValue; 
  {
    //convert our ranges
    float vLeft  = map(leftMotorSpeed,1000,2000,-1,1);
    float vRight = map(rightMotorSpeed,1000,2000,-1,1);
    float vMax = abs(max(vLeft,vRight));
    if(vMax > 1){
      leftMotorSpeed =  map(vLeft/vMax,-1,1,1000,2000);
      rightMotorSpeed =  map(vRight/vMax,-1,1,1000,2000);
    }
  }
 
  /* operate drive base */
  analogWrite(MOTOR_LEFT_1_PIN,leftMotorSpeed);
  analogWrite(MOTOR_LEFT_2_PIN,leftMotorSpeed);
  analogWrite(MOTOR_LEFT_3_PIN,leftMotorSpeed);
  analogWrite(MOTOR_RIGHT_1_PIN,leftMotorSpeed);
  analogWrite(MOTOR_RIGHT_2_PIN,leftMotorSpeed);
  analogWrite(MOTOR_RIGHT_3_PIN,leftMotorSpeed);
  
  /* Run LED strips */
  light_loop();
}
