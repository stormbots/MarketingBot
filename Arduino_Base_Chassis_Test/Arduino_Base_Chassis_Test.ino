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
  
  int chassisPowerMode = 3;

  float throttleValue = myIn.read(2);
  float turningValue= myIn.read(4);
  float chassisPower = myIn.read(5);
  
  throttleValue = constrain(throttleValue,1000,2000);
  turningValue = constrain(turningValue,1000,2000);
  chassisPower = constrain(chassisPower,1000,2000);
  
  //Disabled Mode
  if (chassisPower <=1250){
    throttleValue = 1500;
    turningValue = 1500;
    chassisPowerMode =1;
  }

  //Slow Mode
  else if(chassisPower <= 1750){
  
   // this will retain full acceleration with a lower maximum speed
   throttleValue = constrain(throttleValue,1250,1750);
   turningValue = constrain(turningValue,1250,1750);
   //this will just cut everything in half
   if (throttleValue >= 1500){
    throttleValue = map(throttleValue, 1500,2000,1500,1750);
   }
   else{
    throttleValue = map(throttleValue, 1000, 1499,1250,1499);
   }
   if (turningValue >= 1500){
    turningValue = map(turningValue, 1500,2000,1500,1750);
   }
   else{
    turningValue = map(turningValue, 1000, 1499,1250,1499);
   }
   chassisPowerMode =2;
 }
 else{
   chassisPowerMode = 3;
 }
  
 //send the values on to the top
 
  
  turningValue = map(turningValue,1000,2000,-2000,2000);
  
  leftMotorValue= throttleValue +turningValue;
  rightMotorValue = throttleValue - turningValue;


 
 {
    //convert our ranges
    float vLeft  = map(leftMotorValue,1000,2000,-1,1);
    float vRight = map(rightMotorValue,1000,2000,-1,1);
    float vMax = abs(max(vLeft,vRight));
    if(vMax > 1){
      leftMotorValue =  map(vLeft/vMax,-1,1,1000,2000);
      rightMotorValue =  map(vRight/vMax,-1,1,1000,2000);
    }
 }
  
  leftMotorServo.writeMicroseconds(leftMotorValue);
  rightMotorServo.writeMicroseconds(rightMotorValue);
  //2 or 3 motors, pwm not servo
  // pwm is 0 to 255
  // servo is 0 to 180
  // can use the servo.writeMicroseconds to avoid the map in text
}
