
#include "GamepadQT.h"

class Inputs{
private:

  GamepadQT gp=GamepadQT();

public:
  GamepadQT::QTButtons previous;
  GamepadQT::QTButtons buttons;

  void setup(){

  }

  void loop(){
    previous=buttons;
    buttons=gp.read();
    delay(10);
  }

  GamepadQT::QTButtons read(){
    return buttons;
  }
};


