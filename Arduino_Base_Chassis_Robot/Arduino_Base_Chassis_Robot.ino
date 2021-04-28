#include <PulsePosition.h>
#include <elapsedMillis.h>

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

#define MOTOR_LEFT_PIN 10
#define MOTOR_RIGHT_PIN 9

#define HIGH_GEAR true
#define LOW_GEAR (!HIGH_GEAR)

#define LIGHT_RELAY_PIN 4

#define SHIFTER_PIN 3

PulsePositionOutput radioOutput;
PulsePositionInput radioInput;
/*****************************************/
//TODO: Make controller sketch
/*****************************************/

#define BUILT_IN_LED 13

elapsedMillis heartbeat;

void setup() {
  Serial.begin(9600);
  
  radioOutput.begin(RADIO_OUT_PIN);
  radioInput.begin(RADIO_IN_PIN);
  pinMode(LIGHT_RELAY_PIN,OUTPUT);
  pinMode(13,OUTPUT);
  digitalWrite(SHIFTER_PIN, LOW_GEAR);

  light_setup();
}

void loop() {
  float leftMotorSpeed = 1500;
  float rightMotorSpeed = 1500;

  // float ??? = radioInput.read(1);
  float throttleValue = radioInput.read(2);
  // float ??? = radioInput.read(3);
  float turningValue= radioInput.read(4); 
   // float ??? = radioInput.read(5);
  float lightSetting =radioInput.read(6);
  // float ??? = radioInput.read(7);
  float chassisEnable= radioInput.read(8);
  chassisEnable = 1000;

  throttleValue = constrain(throttleValue,1000,2000);
  turningValue = constrain(turningValue,1000,2000);
  chassisEnable = constrain(chassisEnable,1000,2000);
  lightSetting = constrain(lightSetting,1000,2000);

  /** Handle Low-power and disable switch */
//  if (chassisEnable <=1250){
//    //Disable drivetrain
//    throttleValue = 1500;
//    turningValue = 1500;
//  }
//  else if(chassisEnable <= 1750){
//    //Half speed
//    throttleValue = map(throttleValue, 1000,2000,1250,1750);
//    turningValue = map(turningValue, 1000,2000,1250,1750);
//  }
 
  /** Copy our radio signals out to the current module */
  for(int i=1;i<=8; i++){
    radioOutput.write(i,radioInput.read(i));
  }
  //Write out throttle values, as these may be modified by Chassis
  radioOutput.write(2,throttleValue);
  radioOutput.write(4,turningValue);

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
 
  /* Write to Motors */
  leftMotorSpeed = constrain(leftMotorSpeed,1000,2000);
  rightMotorSpeed = constrain(rightMotorSpeed,1000,2000);
  leftMotorSpeed = map(leftMotorSpeed, 1000, 2000, 0, 255);
  rightMotorSpeed = map(rightMotorSpeed, 1000, 2000, 0, 255);
  analogWrite(MOTOR_LEFT_PIN,leftMotorSpeed);
  analogWrite(MOTOR_RIGHT_PIN,rightMotorSpeed);
  Serial.println(leftMotorSpeed);
  Serial.println(rightMotorSpeed);
  
  /* Run LED strips */
  //Disable leds to reduce power draw
  if (radioInput.read(6) < 1125){
    //turns off leds
    digitalWrite(LIGHT_RELAY_PIN,LOW);
  }
  else{
    //turns leds on
    digitalWrite(LIGHT_RELAY_PIN,HIGH);
  }
  light_loop(lightSetting);

  /* Check our system heartbeat */
  if(heartbeat > 1000){
    heartbeat=0;
    digitalWrite(BUILT_IN_LED,!digitalRead(BUILT_IN_LED));
  }
}
