#ifndef BOT_H
#define BOT_H
#include <DataPackets.h>
#include "Pins.h"
#include <Encoder.h>
#include <elapsedMillis.h>

namespace bot{
  //TODO: Make these a config struct that can be passed to/from the controller
  //This will make for more robust integration and readjustment
  int maxVelocity=10*12 ; //Inches Per Second
  int encoderTicksPerInch=100; //TODO: Find this actual value.
  elapsedMillis velocityTimer=0;


  Encoder encLeft = Encoder(Pins::quad1.a,Pins::quad1.b);
  Encoder encRight = Encoder(Pins::quad2.a,Pins::quad2.b);

  /** Controller/input system should calculate these values and provide
  *  target speed in inches/second. 
  */
  void tankDrive(ChassisSpeeds speeds){
    //Clamp to system maximum 
    //TODO: This value changes when we shift
    int left = constrain (left,-maxVelocity,maxVelocity);
    int right = constrain (right,-maxVelocity,maxVelocity);

    //TODO: Quadrature handling and feed forward
    //For now, incorrectly assume simple linear system.
    int leftoutput = map(speeds.left, -maxVelocity,maxVelocity, -255,255);
    int rightoutput = map(speeds.right, -maxVelocity,maxVelocity, -255,255);

    //TODO: Feedback loop handling
    leftoutput += 0;
    rightoutput += 0;

    // Constrain to maximum possible for the system
    analogWrite(Pins::motors.m1, leftoutput);
    analogWrite(Pins::motors.m2, rightoutput);
  }

  /** Shift the chassis to the appropriate gearing */
  void shift(ChassisGear gear){
    digitalWrite(Pins::pnuematics.p1, gear==ChassisGear::High? 1:0 ); 
  }

  //TODO: Filter encoders a bit instead of reading directly
  /** Read the current speed of the chassis */
  ChassisSpeeds currentSpeed(){
    ChassisSpeeds speeds;
    unsigned long int timer = velocityTimer;
    speeds.left = encLeft.read()*timer/1000/encoderTicksPerInch;
    speeds.right = encRight.read()*timer/1000/encoderTicksPerInch;
    velocityTimer=0;
    return speeds;
  }

  int batteryVoltage(){
      //Read a lipo battery connected to the Feather.
      // chassisTelemetryData.data.batteryVoltage = analogRead(A7) //configured for lipo
      // *2 //double reading due to voltage divider
      // *3.3 //multiply by reference voltage
      // *10 // convert from volt to decivolt
      // /1024 // Divide by ADC steps to get voltage
      // ;
      //TODO: Add board support to properly scale down 12V inputs to 
      return 0; 
  }

  int currentPressure(){
    //TODO: Not currently implimented.
    return 0;
  }

}




#endif