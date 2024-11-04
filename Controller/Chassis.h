
#ifndef chassis_h
#define chassis_h
#include <MarketingBotDataPackets.h>


namespace Chassis{

  //TODO: Query the drive system for the speed it stores.
  //TODO: If bot can go faster than this, need more bits for the base memory type
  float MAXSPEED=127; // In ChassisSpeeds units of in/s 
  
  /** Arduino map but for floats */
  float lerp(float x, float in_min, float in_max, float out_min, float out_max){
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
  }

  /** Rescale ChassisSpeeds to the maximum value */
  ChassisSpeeds normalizeChassisSpeeds(ChassisSpeeds input, float maximum){
    float maxval = max( abs(input.left), abs(input.right) );

    if(maxval>maximum){
      float scalar = maximum/maxval;
      input.left *= scalar;
      input.right *= scalar;
    }
    return input;
  }

  /**
    Convert relative output to wheel speeds
    @param left [-1000...1000]
    @param right [-1000...1000]
  */
  ChassisSpeeds tankDrive(int left, int right){
    ChassisSpeeds output;
    output.left = lerp(left,-1000,1000,-MAXSPEED,MAXSPEED);
    output.right = lerp(right,-1000,1000,-MAXSPEED,MAXSPEED);
    output = normalizeChassisSpeeds(output,MAXSPEED);
    return output;
  }

  /**
    Convert relative input into wheel speeds
    @param throttle throttle; [-1000...1000], positive forward
    @param rotation turn bias; [-1000...1000], Positive CCW (angular positive)
  */
  ChassisSpeeds arcadeDrive( int throttle, int rotation){
    int left = throttle - rotation;
    int right = throttle + rotation;
    return tankDrive(left,right);
  }



}


#endif