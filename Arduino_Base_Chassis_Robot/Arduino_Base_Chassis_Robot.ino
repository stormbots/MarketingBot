#include <PulsePosition.h>
#include <elapsedMillis.h>
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

#define MOTOR_LEFT_PIN 10
#define MOTOR_RIGHT_PIN 9
Servo motorLeft;
Servo motorRight;

#define HIGH_GEAR true
#define LOW_GEAR (!HIGH_GEAR)

// Double shifter removed pin count, not currently available
#define LIGHT_RELAY_PIN 17

#define SHIFTER_PIN_A 4
#define SHIFTER_PIN_B 3

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
  pinMode(SHIFTER_PIN_A,OUTPUT);
  pinMode(SHIFTER_PIN_B,OUTPUT);
  digitalWrite(SHIFTER_PIN_A, LOW_GEAR);
  digitalWrite(SHIFTER_PIN_B, !digitalRead(SHIFTER_PIN_A));

  motorLeft.attach(MOTOR_LEFT_PIN);
  motorLeft.writeMicroseconds(1500);
  motorRight.attach(MOTOR_RIGHT_PIN);
  motorRight.writeMicroseconds(1500);

  light_setup();
}

void loop() {

  /*Debug for Shifting Actuators*/
  //(SHIFTER_PIN_A,HIGH_GEAR);
  //digitalWrite(SHIFTER_PIN_B,LOW_GEAR);
  //delay(3000);
  //digitalWrite(SHIFTER_PIN_A,LOW_GEAR);
  //digitalWrite(SHIFTER_PIN_B,HIGH_GEAR);
  //delay(3000);
  //'return;
 


  
  float leftMotorSpeed = 1500;
  float rightMotorSpeed = 1500;
  
  // float ??? = radioInput.read(1); EMPTY
  float throttleValue = radioInput.read(2);
  // float ??? = radioInput.read(3); TAKEN BY CANNON ELEVATION
  float turningValue= radioInput.read(1); 
  float shiftValue = radioInput.read(7);
  //float lightSetting =radioInput.read(6); TAKEN BY TRIGGER
  // ONLY ^^^^^^^^ WORK WITH CURRENT RECIEVER
  // float ??? = radioInput.read(7);
  float chassisEnable= radioInput.read(8);
  chassisEnable = 1000;

  throttleValue = constrain(throttleValue,1000,2000);
  turningValue = constrain(turningValue,1000,2000);
  chassisEnable = constrain(chassisEnable,1000,2000);
  //lightSetting = constrain(lightSetting,1000,2000);
  shiftValue = constrain(shiftValue,1000,2000);
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
  if(radioInput.available() < 8){
    return;
  }
  for(int i=1;i<=8; i++){
    radioOutput.write(i,radioInput.read(i));
  }
  //Write out throttle values, as these may be modified by Chassis
  radioOutput.write(2,throttleValue);
  radioOutput.write(4,turningValue);
  radioOutput.write(7,shiftValue);
  /** Generate arcade drive left/right outputs */
  turningValue = map(turningValue,1000,2000, 500,-500);
  
  leftMotorSpeed= throttleValue + turningValue;
  rightMotorSpeed = throttleValue - turningValue; 
  {
    //convert our ranges
    float vLeft  = map(leftMotorSpeed,1000,2000,-1,1);
    float vRight = map(rightMotorSpeed,1000,2000,-1,1);    
    float vMax = abs(max(vLeft,vRight));    if(vMax > 1){
      leftMotorSpeed =  map(vLeft/vMax,-1,1,1000,2000);
      rightMotorSpeed =  map(vRight/vMax,-1,1,1000,2000);
    }    leftMotorSpeed =  map(leftMotorSpeed,1000,2000,2000,1000);
  }
  
  /* Shift depending on switch*/
  if (shiftValue <= 1500){
    //shift to low if switch is low
    digitalWrite(SHIFTER_PIN_A,LOW_GEAR);
    digitalWrite(SHIFTER_PIN_B,HIGH_GEAR);
  }
  else {
    digitalWrite(SHIFTER_PIN_A,HIGH_GEAR);
    digitalWrite(SHIFTER_PIN_B,LOW_GEAR);
  }
  
  /* Write to Motors */
  motorLeft.writeMicroseconds(leftMotorSpeed);
  motorRight.writeMicroseconds(rightMotorSpeed);
  Serial.print((int)leftMotorSpeed);
  Serial.print("  ");
  Serial.print((int)rightMotorSpeed);
  Serial.println("");
  
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
  //light_loop(lightSetting);

  /* Check our system heartbeat */
  if(heartbeat > 1000){
    heartbeat=0;
    digitalWrite(BUILT_IN_LED,!digitalRead(BUILT_IN_LED));
  }
}
