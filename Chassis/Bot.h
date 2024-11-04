#ifndef BOT_H
#define BOT_H
#include <MarketingBotDataPackets.h>
#include "Pins.h"
#include <Encoder.h>
#include <elapsedMillis.h>
#include <Servo.h>

namespace bot {

  /** Shared configuration object that can provide chassis tuning data to remote objects,
    * or be used to set/override values for testing or other odd scenarios
    */
  ChassisConfig configLow = {
    .gear = ChassisGear::Low,
    .encoderRatio = 2073 / (int)(8 * 3.14),//2073 ticks/revolution
    .maxForwardVelocity = 14 * 12,
    .maxAngularVelocity = 14,
    .kf = 0,
    .ks = 0,
    .kp = 0,
    .ki = 0,  //Unused in code
    .kd = 0  //unused in code
  };
  // ChassisConfig configHigh; //TODO Impliment when High Gear is relevant
  // ChassisConfig configCustom; //TODO Save non-default configs this way, enabling things like "low speed" or "no spin" control

  //TODO: Make these a config struct that can be passed to/from the controller
  //This will make for more robust integration and readjustment
  int maxVelocity = 10 * 12;      //Inches Per Second
  int encoderTicksPerInch = 100;  //TODO: Find this actual value.
  elapsedMillis velocityTimer = 0;


  Encoder encRight = Encoder(Pins::quad2.a, Pins::quad2.b);
  Encoder encLeft = Encoder(Pins::quad1.a, Pins::quad1.b);

  Servo motorLeft;
  Servo motorRight;

  struct WheelDistance{
      int left;
      int right;
  };

  int shifterPin = Pins::pnuematics.p1;  //TODO no hardware on bot at this time

  /// Get the config for the current gear
  ChassisConfig getConfig(){
    // TODO: Impliment getConfig at some point
    return configLow;
  }

  /** Controller/input system should calculate these values and provide
    *  target speed in inches/second. 
    */
  void tankDrive(ChassisSpeeds speeds) {
    ChassisConfig config = getConfig();

    //Clamp to system maximum
    int left = constrain(left, -config.maxForwardVelocity, config.maxForwardVelocity);
    int right = constrain(right, -config.maxForwardVelocity, config.maxForwardVelocity);

    //Convert our target speed to a servo write. 
    int leftoutput = map(speeds.left, -config.maxForwardVelocity, config.maxForwardVelocity, 1000, 2000);
    int rightoutput = map(speeds.right, -config.maxForwardVelocity, config.maxForwardVelocity, 1000, 2000);

    //TODO: Determine appropriate constraints for turn-oriented systems. 

    //TODO: Feedback loop handling
    leftoutput += 0;
    rightoutput += 0;

    // Constrain to maximum possible for the system
    leftoutput = constrain(leftoutput,1000,2000);
    rightoutput = constrain(rightoutput,1000,2000);

    // Invert the appropriate side
    leftoutput = map(leftoutput,1000,2000,2000,1000);

    //Send out to the motors
    motorLeft.writeMicroseconds(leftoutput);
    motorRight.writeMicroseconds(rightoutput);
  }

  /** Shift the chassis to the appropriate gearing */
  void shift(ChassisGear gear) {
    digitalWrite(Pins::pnuematics.p1, gear == ChassisGear::High ? 1 : 0);
  }


  //TODO: Filter encoders a bit instead of reading directly
  /** Read the current speed of the chassis */
  ChassisSpeeds currentSpeed() {
    // static float scalar = 1000/bot::encLeft.read()/bot::configLow.encoderRatio;
    //correct in inches//
    static WheelDistance lastDistance;
    static WheelDistance distance;
    static elapsedMillis timer;
    static ChassisSpeeds speeds;
    WheelDistance delta;
    if(timer<100)return speeds; //Prevent jumpyness by doing this only 100 times per second
    distance.left=encLeft.read();
    distance.right=encRight.read();
    delta.left = encLeft.read() - lastDistance.left;
    delta.right= encRight.read()- lastDistance.right;
    lastDistance=distance;
    
    // distance/time                      // basic equation
    // ticks/ticksperinch / (timer/1000) // split to our minimal terms
    // ticks/ticksperinch*1000/timer      // streamline the math
    // ticks*1/(ticksperinch*1000/timer)  // Isolate slow divisions for caching
    float scalar=1000/bot::configLow.encoderRatio/(float)timer;
    speeds.left = delta.left*scalar;
    speeds.right = delta.right*scalar;

    // Serial.printf("[%4i %4i]",speeds.left,speeds.right);
    // Serial.printf("[%4i %4i]",delta.left,delta.right);
    // Serial.printf("[%3i]",(int)timer);
    // Serial.println();


    timer=0;
    return speeds;
  }

  /// Return battery voltage in volts (not currently implimented)
  int batteryVoltage() {
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

  /// Return system air pressure (not currently implimented)
  int currentPressure() {
    //TODO: Not currently implimented.
    return 0;
  }



  ///  Assign pin details for various IO and hardware
  void init() {

    //Configure the servos handling the motors
    motorLeft.attach(Pins::motors.m1);
    motorRight.attach(Pins::motors.m2);

    pinMode(Pins::pnuematics.p1, OUTPUT);
    shift(ChassisGear::Low);

    tankDrive((ChassisSpeeds){0,0});
  }

}




#endif