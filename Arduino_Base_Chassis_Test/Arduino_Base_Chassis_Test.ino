#include <PulsePosition.h>
#include <Servo.h>

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
  float leftMotorSpeed = 1500;
  float rightMotorSpeed = 1500;
  
  float throttleValue = myIn.read(2);
  float turningValue= myIn.read(4);
  float chassisPowerValue = myIn.read(5);
  
  throttleValue = constrain(throttleValue,1000,2000);
  turningValue = constrain(turningValue,1000,2000);
  chassisPowerValue = constrain(chassisPowerValue,1000,2000);
  
  //Check 3 position switch for Enable/ for Disabled Mode
  if (chassisPowerValue <=1250){
    throttleValue = 1500;
    turningValue = 1500;
  }
  //Slow Mode
  else if(chassisPowerValue <= 1750){
   throttleValue = map(throttleValue, 1000,2000,1250,1750);
   turningValue = map(turningValue, 1500,2000,1500,1750);   
  }
  
  //Do Math for converting Arcade Drive to Tank Drive

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
