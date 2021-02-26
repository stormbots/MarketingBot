
#include <PulsePosition.h>
#include <Servo.h>
#include <Motor.h>

Servo leftMotorServo1;
Servo rightMotorServo1;
Servo leftMotorServo2;
Servo rightMotorServo2;
Servo leftMotorServo3;
Servo rightMotorServo3;
PulsePositionOutput myOut;
PulsePositionInput myIn;



void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  myOut.begin(9);
  myIn.begin(//whatever pin it is on the robot);
  leftMotorServo1.attach(//whatever pin it is on the robot);
  leftMotorServo2.attach(//whatever pin it is on the robot);
  leftMotorServo3.attach(//whatever pin it is on the robot);
  rightMotorServo1.attach(//whatever pin it is on the robot);
  rightMotorServo2.attach(//whatever pin it is on the robot);
  rightMotorServo3.attach(//whatever pin it is on the robot);
  DualPWM leftMotors(//pins for the left servos);
  DualPWM rightMotors(//pins for the right servos);
  // dont forget to add the rest of the servos/motor library for final
}

void loop() {
  // put your main code here, to run repeatedly:
  float leftMotorSpeed = 1500;
  float rightMotorSpeed = 1500;

  int chassisPowerMode = 3;
 
  float throttleValue = myIn.read(2);
  float turningValue= myIn.read(4); 
  float chassisPowerValue = myIn.read(5);
   
  throttleValue = constrain(throttleValue,1000,2000);
  turningValue = constrain(turningValue,1000,2000);
  chassisPowerValue = constrain(chassisPowerValue,1000,2000);
  
  //Disabled Mode
  if (chassisPowerValue <=1250){
    throttleValue = 1500;
    turningValue = 1500;
    chassisPowerMode =1;
  }

  //Slow Mode
  else if(chassisPowerValue <= 1750){
   throttleValue = map(throttleValue, 1000,2000,1250,1750);
   turningValue = map(turningValue, 1500,2000,1500,1750);
   chassisPowerMode =2;
 }
 else{
   chassisPowerMode = 3;
 }
 
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
 
  //SERVO TO PPM ||DON'T USE ON TEST SETUP||
  leftMotorSpeed= map(leftMotorSpeed,1000,2000,0,255);
  rightMotorSpeed = map(rightMotorSpeed,1000,2000,0,255);
  analogWrite(//whatever pin the motor is ,leftMotorSpeed);
  analogWrite(//whatever pin the motor is,rightMotorSpeed);
  
}
