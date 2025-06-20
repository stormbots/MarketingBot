#ifndef BOT_H
#define BOT_H
#include <MarketingBotDataPackets.h>
#include "Pins.h"
#include <Encoder.h>
#include <elapsedMillis.h>
#include <Servo.h>
#include "SlewRateLimiter.h"

namespace bot {

  //Tested hypothetical max speed of the bot is 251 inches/second

  /** Shared configuration object that can provide chassis tuning data to remote objects,
    * or be used to set/override values for testing or other odd scenarios
    */
  ChassisConfig configLow = {
    .gear = ChassisGear::Low,
    .encoderRatio = 2073 / (int)(8 * 3.14),//2073 ticks/revolution
    .maxForwardVelocity = 14 * 12, //251 is maximum speed on blocks, with no wheel resistance
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

  SlewRateLimiter leftSlew(0,200);
  SlewRateLimiter rightSlew(0,200);

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

  // ChassisSpeeds slewChassisSpeeds(ChassisSpeeds input){
  //   return {slewRateLimit(input.left),slewRateLimit(input.right)};
  // }

  /** Controller/input system should calculate these values and provide
    *  target speed in inches/second. 
    */
  void tankDrive(ChassisSpeeds speeds) {
    ChassisConfig config = getConfig();

    //Clamp to system maximum
    int left = constrain(left, -config.maxForwardVelocity, config.maxForwardVelocity);
    int right = constrain(right, -config.maxForwardVelocity, config.maxForwardVelocity);

    //Apply our rate limits
    left = leftSlew.calculate(left);
    right = rightSlew.calculate(right);

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
    // rightoutput = map(rightoutput,1000,2000,2000,1000);
    leftoutput = map(leftoutput,1000,2000,2000,1000);

    //Send out to the motors
    motorLeft.writeMicroseconds(leftoutput);
    motorRight.writeMicroseconds(rightoutput);
  }

  /** Directly set servo output; For debugging purposes only; 
  @param leftus 1000...2000; 2000 being forward
  @param rightus 1000...2000; 2000 being forward
  */
  void tankDriveRaw(int leftus,int rightus){
    rightus = map(rightus,1000,2000,2000,1000);
    motorLeft.writeMicroseconds(leftus);
    motorRight.writeMicroseconds(rightus);
  }

  /** Shift the chassis to the appropriate gearing */
  void shift(ChassisGear gear) {
    digitalWrite(Pins::pnuematics.p1, gear == ChassisGear::High ? 1 : 0);
  }


int maxleft=0;
int maxright=0;


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


    maxleft = max(maxleft, delta.left*scalar);
    maxright = max(maxright, delta.right*scalar);
  

    // Serial.printf("[%4i %4i]",speeds.left,speeds.right);
    // Serial.printf("[%4i %4i]",delta.left,delta.right);
    // Serial.printf("[%3i]",(int)timer);
    // Serial.println();


    timer=0;
    return speeds;
  }

  /// Return battery voltage in volts (not currently implimented)
  float batteryVoltage() {
    //Read a lipo battery connected to the Feather.
    // chassisTelemetryData.data.batteryVoltage = analogRead(A7) //configured for lipo
    // *2 //double reading due to voltage divider
    // *3.3 //multiply by reference voltage
    // *10 // convert from volt to decivolt
    // /1024 // Divide by ADC steps to get voltage
    // ;
    //TODO: iAdd board support to properly scale down 12V inputs to
    return 15;
  }

  /// Return system air pressure (not currently implimented)
  int currentPressure() {
    //TODO: Not currently implimented.
    return 0;
  }

  void enable(){
    motorLeft.attach(Pins::motors.m1);
    motorRight.attach(Pins::motors.m2);
  }

  void disable(){
    motorLeft.detach();
    motorRight.detach();
  }

  ///  Assign pin details for various IO and hardware
  void init() {

    //Configure the servos handling the motors
    // motorLeft.attach(Pins::motors.m1);
    // motorRight.attach(Pins::motors.m2);

    pinMode(Pins::pnuematics.p1, OUTPUT);
    shift(ChassisGear::Low);

    tankDrive((ChassisSpeeds){0,0});
  }

}




#endif