#ifndef SLEWRATELIMITER_H
#define SLEWRATELIMITER_H

#include <elapsedMillis.h>


class SlewRateLimiter{
  private:
  float rate=0;
  elapsedMillis timer=0;  
  float last_value=0;

  public:
  SlewRateLimiter(float initial, float slewrate){
    last_value = initial;
    rate=slewrate;
    timer=0;
  }

  void setRate(float value){
    rate=value;
  }

  void set(int value){
    last_value=value;
  }

  float calculate(float value){
    float diff= rate*timer/1000.0;
    value = constrain(value,last_value-diff,last_value+diff);
    timer=0;
    last_value = value;
    return value;
  }
};

#endif