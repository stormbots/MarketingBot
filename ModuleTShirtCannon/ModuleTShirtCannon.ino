#include <ElapsedMillis.h>
#include <PulsePosition.h>
#include <Bounce.h>
/* Hardware: */
#define REVOLVER_MOTOR_PIN 1

#define MAG_ENCODER_A_PIN 1
#define MAG_ENCODER_A_PIN 2

#define DUMP_VALVE_A_PIN 2
#define DUMP_VALVE_B_PIN 2

#define INDEX_LOCK_A_PIN 2
#define INDEX_LOCK_B_PIN 2

#define CYLINDER_CLAMP_A_PIN 3
#define CYLINDER_CLAMP_B_PIN 3

#define INDEX_LOCK_PIN 3
#define BUILT_IN_LED 13

Bounce indexSwitch = Bounce(INDEX_LOCK_PIN,10);
void setup(){
  pinMode(BUILT_IN_LED, OUTPUT);
}


void loop(){
  /* Check our system heartbeat */
  if (heartbeat > 1000){
    heartbeat=0;
    digitalWrite(BUILT_IN_LED, !digitalRead(BUILT_IN_LED));
  }

  //TODO: Read radio inputs
  //TODO: Read safety signals somehow

  /* Process */
  bool cannonTrigger = radioInput.read(6) <= 1250;


  //Adjust angle and manage PIDs
  //TODO

  run_state_machine(cannonTrigger);
}

// hardware:

// up/down window motor, heavy reduction
// has absolute position sensor mag encoder
// limit on up and down

// indexing revolver, 8 barrels
// pins for hard stop
// hopefully limit switch

// pressure regulated by humans, and only decreases


/**
  assume indexed and firing

  engage cylinderClamp
  wait for pressure (timer probably)
  open pressureChamber pressure (if it's sufficiently pressurized)
  wait for pressureChamber to empty (timer again)
  seal pressureChamber (rebuild pressure, background)
  disengage cylinderClamp (some wait time)
  release indexLock (wait some time)
  start revolver
  (wait some time to clear lock)
  engage indexLock
  wait for indexSwitch || max runtime exceeded


*/



enum State{
  STARTUP,
  PRESSURIZING,
  IDLE,
  FIRING,
  RECOVERY,
  RELOAD_UNLOCKED,
  RELOAD_LOCKED,
  RESET
}
ElapsedMillis timer;
State state = State.STARTUP;
State last_state=Stat.RESET;
void run_state_machine(bool cannonTrigger){

  indexSwitch.update();
  switch(state){
    case STARTUP:
      //run code here
      //lock index
      digitalWrite(INDEX_LOCK_A_PIN,true);
      //motor on???
      analogWrite(REVOLVER_MOTOR_PIN,127+32);
      // open clamps
      digitalWrite(CYLINDER_CLAMP_A_PIN,true);
      //switch to pressurizing when it is ready to fire/ indexswitch is activated
      indexSwitch.update();
      if (indexSwitch.read()== false){
        state=PRESSURIZING;
      }
    break;
    case PRESSURIZING:
      //index locked
      digitalWrite(INDEX_LOCK_A_PIN,true);
      //motor off
      analogWrite(REVOLVER_MOTOR_PIN,127);
      //cylinder clamps open
      digitalWrite(CYLINDER_CLAMP_A_PIN,true);
      //dump valve closed
      digitalWrite(DUMP_VALVE_A_PIN,false);
      //Make sure we've waited long enough to pressurize,
      //and that a human has put the controller in the correct spot to fire.
      if (cannonTrigger == false && timer>3000){
        state=IDLE;
      }
    break;
    case IDLE:
      //index locked
      digitalWrite(INDEX_LOCK_A_PIN,true);
      //motor off
      analogWrite(REVOLVER_MOTOR_PIN,127);
      //cylinder clamps closed
      digitalWrite(CYLINDER_CLAMP_A_PIN,false);
      //dump valve closed
      digitalWrite(DUMP_VALVE_A_PIN,false);
      // if the trigger switch if flicked progress to firing
      if (cannonTrigger){
        state = FIRING;
      }
    break;
    case FIRING:
      //index locked
      digitalWrite(INDEX_LOCK_A_PIN,true);
      //motor off
      analogWrite(REVOLVER_MOTOR_PIN,127);
      //cylinder clamps closed
      digitalWrite(CYLINDER_CLAMP_A_PIN,false);
      //dump valve open
      digitalWrite(DUMP_VALVE_A_PIN,true);
      // if the timer is up for firing and the switch is flicked back to avoid accidents advance to recovery
      if (timer > 3000){
        state = RECOVERY;
      }
    break;
    case RECOVERY:
      // index unlocked
      digitalWrite(INDEX_LOCK_A_PIN,false);
      // motor off
      analogWrite(REVOLVER_MOTOR_PIN,127);
      //cylinder clamps open
      digitalWrite(CYLINDER_CLAMP_A_PIN,true);
      // dump valve closed
      digitalWrite(DUMP_VALVE_A_PIN,false);
      // if timer expires advance, value is placeholder
      if (timer > 3000){
        state=RELOAD_UNLOCKED;
      }
    break;
    case RELOAD_UNLOCKED:
      //index unlocked
      digitalWrite(INDEX_LOCK_A_PIN,false);
      //motor on 
      analogWrite(REVOLVER_MOTOR_PIN,127+32);//placeholder value 1/8 speed
      //cylinder clamps open
      digitalWrite(CYLINDER_CLAMP_A_PIN,true);
      //dump valve closed
      digitalWrite(DUMP_VALVE_A_PIN,false);
      // if timer expires advance, value is placeholder
      if (timer > 3000){
        state=RELOAD_LOCKED;
      }
    break;
    case RELOAD_LOCKED:
      //index locked
      digitalWrite(INDEX_LOCK_A_PIN,true);
      //motor on
      analogWrite(REVOLVER_MOTOR_PIN,127+32);//placeholder value 1/8 speed
      //cylinder clamps open
      digitalWrite(CYLINDER_CLAMP_A_PIN,true);
      //dump valve closed
      digitalWrite(DUMP_VALVE_A_PIN,false);
      //if switch is tripped. Fallback safety timer.
      if(indexSwitch.read()==false || timer>3000){
        state=PRESSURIZING;
      }
  }
  default:
    //things got wack
    state=STARTUP

  if (last_state != state){
    timer=0;
    last_state=state;
  }
}
