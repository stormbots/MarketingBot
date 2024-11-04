
#include <PCAL6416A-SOLDERED.h>

#ifndef pins_h
#define pins_h

namespace Pins{

  struct {
    // int down=PCAL6416A_A0;
    // int right=PCAL6416A_A1;
    // int up=PCAL6416A_A3;
    // int left=PCAL6416A_A4;
    int down=0;
    int right=1;
    int up=3;
    int left=4;

    int l1=2;
    int l2=15;
    // int l3=PCAL6416A_??;
    int l4=9;

    int select1=14;
    int select2=13;
  } LeftExp;

  struct {
    int a=PCAL6416A_B7;
    int b=PCAL6416A_B2;
    int x=PCAL6416A_B5;
    int y=PCAL6416A_B3;

    int r1=PCAL6416A_B4;
    int r2=PCAL6416A_B6;
    // int r3=PCAL6416A_??;
    int r4=PCAL6416A_A7;

    int select1=0;
    int select2=1;

    //TODO not implimented on hardware
    // int pov_up=PCAL6416A_A2;
    // int pov_down=PCAL6416A_A6;
    // int pov_left=PCAL6416A_A5;
    // int pov_right=PCAL6416A_A3;
    // int pov_press=PCAL6416A_A4;
  } RightExp;

  struct{
    uint8_t x=A2;
    uint8_t y=A3;
  }LeftJoystick;

  uint8_t home=5; //the enable button

  struct{
    uint8_t a=12;
    uint8_t b=13;
  }WheelEncoder;

}



#endif

