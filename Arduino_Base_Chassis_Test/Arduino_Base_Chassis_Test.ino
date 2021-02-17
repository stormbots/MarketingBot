#include <PulsePosition.h>
#include <Servo.h>
#include <Motor.h>
Servo leftMotorServo;
Servo rightMotorServo;
PulsePositionOutput myOut;
PulsePositionInput myIn;



void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  myOut.begin(9);
  myIn.begin(22);
  leftMotorServo.attach(20);
  rightMotorServo.attach(21);
  // dont forget to add the rest of the servos/motor library for final
}

void loop() {
  // put your main code here, to run repeatedly:
  int leftMotorValue = 90;
  int rightMotorValue = 90;

  int turnValue= 0;
  int throttleValue = myIn.read(2);
  int turningValue= myIn.read(4);
  leftMotorValue= map(throttleValue,1000,2000,0,180);
  rightMotorValue = map(throttleValue,1000,2000,0,180);
  turnValue = map(turningValue,1000,2000,-180,180);
  leftMotorValue= leftMotorValue + turnValue;
  rightMotorValue = rightMotorValue - turnValue; 
  if (leftMotorValue > 180){
    leftMotorValue= 180;
  }
  else if (leftMotorValue < 0){
    leftMotorValue = 0;
  }
  if (rightMotorValue > 180){
    rightMotorValue= 180;
  }
  else if (rightMotorValue < 0){
    rightMotorValue = 0;
  }
  leftMotorServo.write(leftMotorValue);
  rightMotorServo.write(rightMotorValue);
  
  
}
