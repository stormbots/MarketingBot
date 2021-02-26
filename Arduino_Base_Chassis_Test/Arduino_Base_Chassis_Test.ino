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
  
  float tempChannelVar1 = myIn.read(1);
  myOut.write(1,tempChannelVar1);
  float tempChannelVar2 = myIn.read(2);
  myOut.write(2,tempChannelVar2);
  float tempChannelVar3 = myIn.read(3);
  myOut.write(3,tempChannelVar3);
  float tempChannelVar4 = myIn.read(4);
  myOut.write(4,tempChannelVar4);
  float tempChannelVar5 = myIn.read(5);
  myOut.write(5,tempChannelVar4);
  float tempChannelVar6 = myIn.read(6);
  myOut.write(6,tempChannelVar6);
  
  
}

void loop() {
  // put your main code here, to run repeatedly:
  float leftMotorSpeed = 1500;
  float rightMotorSpeed = 1500;
  
  
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
    myOut.write(7,1000);
  }

  //Slow Mode
  else if(chassisPowerValue <= 1750){
   throttleValue = map(throttleValue, 1000,2000,1250,1750);
   turningValue = map(turningValue, 1500,2000,1500,1750);   
   myOut.write(7,1500);
 }

 else{   
   myOut.write(7,2000);
 }
 
  turningValue = map(turningValue,1000,2000,-2000,2000);
  
  leftMotorSpeed= throttleValue +turningValue;
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
  leftMotorServo.writeMicroseconds(leftMotorSpeed);
  rightMotorServo.writeMicroseconds(rightMotorSpeed);
  //SERVO TO PPM Test Version(only shows leds)
  //leftMotorSpeed= map(leftMotorSpeed,1000,2000,0,255);
  //rightMotorSpeed = map(rightMotorSpeed,1000,2000,0,255);
  //analogWrite(3,leftMotorSpeed);
  //analogWrite(4,rightMotorSpeed);
  
  
}
