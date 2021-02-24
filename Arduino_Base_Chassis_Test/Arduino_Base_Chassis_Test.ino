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
  float leftMotorValue = 1500;
  float rightMotorValue = 1500;


  int throttleValue = myIn.read(2);
  int turningValue= myIn.read(4);
  turningValue = map(turningValue,1000,2000,-2000,2000);
  Serial.println(throttleValue);
  Serial.println(turningValue);
  leftMotorValue= throttleValue +turningValue;
  rightMotorValue = throttleValue - turningValue;
 
  //replace this safety with normalized value
  leftMotorValue = (leftMotorValue + 1000) / (4000 +1000);
  rightMotorValue = (rightMotorValue + 1000) / (4000 + 1000);
  //Serial.println(leftMotorValue);
  //Serial.println(rightMotorValue);
  leftMotorValue= map(leftMotorValue,0,1,1000,2000);
  rightMotorValue= map(rightMotorValue,0,1,1000,2000);
  //Serial.println(leftMotorValue);
  //Serial.println(rightMotorValue);
  leftMotorServo.writeMicroseconds(leftMotorValue);
  rightMotorServo.writeMicroseconds(rightMotorValue);
  //2 or 3 motors, pwm not servo
  // pwm is 0 to 255
  // servo is 0 to 180
  // can use the servo.writeMicroseconds to avoid the map in text
  
}
