//Radio, heartbeat, and motor libraries
#include <PulsePosition.h>
#include <elapsedMillis.h>
#include <Servo.h>

//Defining Teensy Pins
#define RADIO_IN_PIN 22
#define RADIO_OUT_PIN 23

#define MOTOR_LEFT_PIN 10
#define MOTOR_RIGHT_PIN 9

#define SHIFTER_PIN_A 4
#define SHIFTER_PIN_B 3

#define BUILT_IN_LED 13

//Defining default gear
#define HIGH_GEAR true
#define LOW_GEAR (!HIGH_GEAR)

//Declaring radio input and output
PulsePositionOutput radioOutput;
PulsePositionInput radioInput;

//Declaring heartbeat
elapsedMillis heartbeat;

//Declare Motors
Servo motorLeft;
Servo motorRight;

void setup() {
  //Begging Serial log
  Serial.begin(9600);

  //Start up radio 
  radioOutput.begin(RADIO_OUT_PIN);
  radioInput.begin(RADIO_IN_PIN);
  
  //Set up pinmodes
  pinMode(13,OUTPUT);
  pinMode(SHIFTER_PIN_A,OUTPUT);
  pinMode(SHIFTER_PIN_B,OUTPUT);

  //Put bot into gear
  digitalWrite(SHIFTER_PIN_A, LOW_GEAR);
  digitalWrite(SHIFTER_PIN_B, !digitalRead(SHIFTER_PIN_A));

  //Zero out motors on startup
  motorLeft.attach(MOTOR_LEFT_PIN);
  motorLeft.writeMicroseconds(1500);
  motorRight.attach(MOTOR_RIGHT_PIN);
  motorRight.writeMicroseconds(1500);
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

  //Declare motor speed variable
  float leftMotorSpeed = 1500;
  float rightMotorSpeed = 1500;

  //Declare control and shifting values using radio
  float throttleValue = radioInput.read(2);
  float turningValue= radioInput.read(1); 
  float shiftValue = radioInput.read(7);

  //Constrain values to avoid weirdness
  throttleValue = constrain(throttleValue,1000,2000);
  turningValue = constrain(turningValue,1000,2000);
  shiftValue = constrain(shiftValue,1000,2000);
  
  //Copy radio signals and pass them on to the module
  if(radioInput.available() < 8){
    return;
  }
  for(int i=1;i<=8; i++){
    radioOutput.write(i,radioInput.read(i));
  }
  
  //Overwrite throttle values being sent to the module , 
  //as these may be modified by Chassis
  radioOutput.write(2,throttleValue);
  radioOutput.write(1,turningValue);
  radioOutput.write(7,shiftValue);

  //Arcade Drive Logic//
  turningValue = map(turningValue,1000,2000, 500,-500);
  
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
    leftMotorSpeed =  map(leftMotorSpeed,1000,2000,2000,1000);
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
  
  /* Check our system heartbeat */
  if(heartbeat > 1000){
    heartbeat=0;
    digitalWrite(BUILT_IN_LED,!digitalRead(BUILT_IN_LED));
  }
}
