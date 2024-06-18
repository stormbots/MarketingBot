#ifndef GamepadQT_h
#define GamepadQT_h

#include <Adafruit_seesaw.h>

class GamepadQT{
  private: 
  //masks for button placements
  uint32_t mask_x=1<<6;
  uint32_t mask_y=1<<2;
  uint32_t mask_a=1<<5;
  uint32_t mask_b=1<<1;
  uint32_t mask_select=1<<0;
  uint32_t mask_start=1<<16;
  uint32_t button_mask = mask_x|mask_y|mask_a|mask_b|mask_select|mask_start;

  int address = 0x50;


  Adafruit_seesaw ss;
  public:

  struct QTButtons{
    float jx;
    float jy;
    //fancy struct to allow XORing buttons for edge capture
    union{
      struct{
        boolean start:1;
        boolean select:1;
        boolean a:1;
        boolean b:1;
        boolean x:1;
        boolean y:1;
      };
      int buttons:5;
    };
  };

  GamepadQT(){
    bool success=ss.begin(address);
    if(success){
      Serial.println("Gamepadqt online");
    }
  }

  QTButtons read(){
    uint32_t response=ss.digitalReadBulk(button_mask);
    uint32_t jx= ss.analogRead(14);
    uint32_t jy= ss.analogRead(15);

    QTButtons temp={
      (float) map(jx,0,1023,-1,1),
      (float) map(jy,0,1023,-1,1),
      (response&mask_start) !=0,
      (response&mask_select) !=0,
      (response&mask_a) !=0,
      (response&mask_b) !=0,
      (response&mask_x) !=0,
      (response&mask_y) !=0,
    };
    return temp;
  }
};


#endif