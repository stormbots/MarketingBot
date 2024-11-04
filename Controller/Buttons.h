
//Bunch of stuff related to the GPIO

#include <PCAL6416A-SOLDERED.h>
#include "Pins.h"
#include <Encoder.h>
#include <elapsedMillis.h>

#ifndef buttons_h
#define buttons_h


namespace Buttons{
  /** Track time since last meaningful action*/
  elapsedMillis _idleTimer;

  PCAL6416A exright; 
  PCAL6416A exleft; 

  struct JoystickAxesConfig{
    int min;
    int max;
    // int idle;
    int idleMin;
    int idleMax;
  };
  JoystickAxesConfig configLeftX={ 
    .min=131,
    .max=886,
    .idleMin=500,
    .idleMax=543
  };
  // normalizeAnalog(Buttons::leftX(),131,(543+500)/2 ,886,50)
  JoystickAxesConfig configLeftY={ 
    .min=163,
    .max=684,
    .idleMin=493,
    .idleMax=535
  };
  // normalizeAnalog(Buttons::leftY(),163,(530+493)/2 ,684,50)

  struct JoystickReadings{
    int x;
    int y;
  };

  Encoder rollerWheel(Pins::WheelEncoder.a,Pins::WheelEncoder.b);

  void init(){
    
    //Set up the right side for primary buttons
    exleft.begin();
    exright.begin(0x21);

    exright.pinModePCAL(PCAL6416A_A0, INPUT_PULLUP);
    exright.pinModePCAL(PCAL6416A_A1, INPUT_PULLUP);
    exright.pinModePCAL(PCAL6416A_A2, INPUT_PULLUP);
    exright.pinModePCAL(PCAL6416A_A3, INPUT_PULLUP);
    exright.pinModePCAL(PCAL6416A_A4, INPUT_PULLUP);
    exright.pinModePCAL(PCAL6416A_A5, INPUT_PULLUP);
    exright.pinModePCAL(PCAL6416A_A6, INPUT_PULLUP);
    exright.pinModePCAL(PCAL6416A_A7, INPUT_PULLUP);

    exright.pinModePCAL(PCAL6416A_B0, INPUT_PULLUP);
    exright.pinModePCAL(PCAL6416A_B1, INPUT_PULLUP);
    exright.pinModePCAL(PCAL6416A_B2, INPUT_PULLUP);
    exright.pinModePCAL(PCAL6416A_B3, INPUT_PULLUP);
    exright.pinModePCAL(PCAL6416A_B4, INPUT_PULLUP);
    exright.pinModePCAL(PCAL6416A_B5, INPUT_PULLUP);
    exright.pinModePCAL(PCAL6416A_B6, INPUT_PULLUP);
    exright.pinModePCAL(PCAL6416A_B7, INPUT_PULLUP);

    exleft.pinModePCAL(PCAL6416A_A1, INPUT_PULLUP);
    exleft.pinModePCAL(PCAL6416A_A0, INPUT_PULLUP);
    exleft.pinModePCAL(PCAL6416A_A2, INPUT_PULLUP);
    exleft.pinModePCAL(PCAL6416A_A3, INPUT_PULLUP);
    exleft.pinModePCAL(PCAL6416A_A4, INPUT_PULLUP);
    exleft.pinModePCAL(PCAL6416A_A5, INPUT_PULLUP);
    exleft.pinModePCAL(PCAL6416A_A6, INPUT_PULLUP);
    exleft.pinModePCAL(PCAL6416A_A7, INPUT_PULLUP);

    exleft.pinModePCAL(PCAL6416A_B0, INPUT_PULLUP);
    exleft.pinModePCAL(PCAL6416A_B1, INPUT_PULLUP);
    exleft.pinModePCAL(PCAL6416A_B2, INPUT_PULLUP);
    exleft.pinModePCAL(PCAL6416A_B3, INPUT_PULLUP);
    exleft.pinModePCAL(PCAL6416A_B4, INPUT_PULLUP);
    exleft.pinModePCAL(PCAL6416A_B5, INPUT_PULLUP);
    exleft.pinModePCAL(PCAL6416A_B6, INPUT_PULLUP);
    exleft.pinModePCAL(PCAL6416A_B7, INPUT_PULLUP);

    pinMode(Pins::home,INPUT_PULLUP);
  }

  unsigned long int idleTimer(){
    return _idleTimer;
  }

  unsigned long int home2(){
    //NOTE Static function variables are only initialized once
    // This lets it keep "global" states but only function scope access
    static elapsedMillis press=0;
    if(!digitalRead(Pins::home)){ //pressed
      _idleTimer=0;
      return press;
    }
    press=0;
    return 0;
  }

  bool dpadDown(){
    return !exleft.digitalReadPCAL(PCAL6416A_A0);
  }
  bool dpadUp(){
    return !exleft.digitalReadPCAL(PCAL6416A_A3);
  }
  bool dpadLeft(){
    return !exleft.digitalReadPCAL(PCAL6416A_A4);
  }
  bool dpadRight(){
    return !exleft.digitalReadPCAL(PCAL6416A_A1);
  }
  
  bool select(){
    return !exleft.digitalReadPCAL(Pins::LeftExp.select1);
  }
  bool select2(){
    return !exleft.digitalReadPCAL(Pins::LeftExp.select2);
  }
  bool start(){
    return !exright.digitalReadPCAL(Pins::RightExp.select1);
  }
  bool start2(){
    return !exright.digitalReadPCAL(Pins::RightExp.select2);
  }


  bool a(){
    return !exright.digitalReadPCAL(PCAL6416A_B7);
  }
  bool b(){
    return !exright.digitalReadPCAL(PCAL6416A_B2);
  }
  bool x(){
    return !exright.digitalReadPCAL(PCAL6416A_B5);
  }
  bool y(){
    return !exright.digitalReadPCAL(PCAL6416A_B3);
  }

  bool l1(){
    return !exleft.digitalReadPCAL(Pins::LeftExp.l1);
  }
  bool l2(){
    return !exleft.digitalReadPCAL(Pins::LeftExp.l2);
  }
  bool l4(){
    return !exleft.digitalReadPCAL(Pins::LeftExp.l4);
  }
  bool r1(){
    return !exright.digitalReadPCAL(Pins::RightExp.r1); //ok
  }
  bool r2(){
    return !exright.digitalReadPCAL(Pins::RightExp.r2); //ok
  }
  bool r4(){
    return !exright.digitalReadPCAL(Pins::RightExp.r4); //ok
  }



  int leftXRaw(){
    return analogRead(Pins::LeftJoystick.x);
  }

  int leftYRaw(){
    return analogRead(Pins::LeftJoystick.y);
  }



  /** Compare raw readings to the idles, 
  * @param Raw joystick readings to consider
  * @return if joystick is out of deadband
  */
  boolean deadandInputs(JoystickReadings input){
    bool xok = input.x < configLeftX.idleMin || input.x > configLeftX.idleMax;
    bool yok = input.y < configLeftY.idleMin || input.y > configLeftY.idleMax;
    return xok==false && yok==false;
  }

  /** Rescale an input to +/-1000 with zero at idle*/
  int normalizeAnalog(int input, JoystickAxesConfig config){
    input = constrain(input, config.min, config.max);
    int idle=(config.idleMax+config.idleMin)/2;
    if(input<idle) return map(input,idle,config.min,0,-1000);
    if(input>idle) return map(input,idle,config.max,0, 1000);
    return 0;
  }

  /** Handle reading the Left Analog stick
  * @return JoystickReadings with range +/-1000 per axis
  */
  JoystickReadings LeftStick(){
    JoystickReadings stick;
    stick.x=leftXRaw();
    stick.y=leftYRaw();
    if(deadandInputs(stick)){
      stick.x=0;
      stick.y=0;
      return stick;
    }

    _idleTimer=0;
    stick.x = normalizeAnalog( stick.x, configLeftX );
    stick.y = normalizeAnalog( stick.y, configLeftY );
    return stick;
  }



  /** Print a basic line of controller status info*/
  void printDebug(){
    Serial.println();
    Serial.printf(
      "[%s%s%s%s]",
      Buttons::dpadDown()?"^":" ",
      Buttons::dpadUp()?"v":" ",
      Buttons::dpadLeft()?"<":" ",
      Buttons::dpadRight()?">":" "
    );

    Serial.printf(
      "[%s%s%s%s]",
      Buttons::select()?"1":" ",
      Buttons::select2()?"2":" ",
      Buttons::start()?"1":" ",
      Buttons::start2()?"2":" "
    );

    Serial.printf(
      "[%s%s%s%s]",
      Buttons::a()?"a":" ",
      Buttons::b()?"b":" ",
      Buttons::x()?"x":" ",
      Buttons::y()?"y":" "
    );

    Serial.printf(
      "[L%s%s%sR%s%s%s]",
      Buttons::l1()?"1":" ",
      Buttons::l2()?"2":" ",
      Buttons::l4()?"4":" ",
      Buttons::r1()?"1":" ",
      Buttons::r2()?"2":" ",
      Buttons::r4()?"4":" "
    );
    // Serial.printf(
    //   "[%s %4i]",
    //   Buttons::home()?"H":" ",
    //   constrain(Buttons::homeHoldTimer(),0,9999)
    // );
    Serial.printf(
      "[%s %4i]",
      Buttons::home2()?"H":" ",
      constrain(Buttons::home2(),0,9999)
    );


    Serial.printf(
      "[X%4i Y%4i]",
      Buttons::LeftStick().x,
      Buttons::LeftStick().y
    );

    Serial.printf(
      "[| %4i]",
      Buttons::rollerWheel.read()
    );

    Serial.printf(
      "[z %4i]",
      constrain(Buttons::idleTimer(),0,99999)
    );



    delay(100);
  }
}


#endif