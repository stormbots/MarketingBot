#include <ElapsedMillis.h>
#include <PulsePosition.h>
#include <Bounce.h>

/* Hardware: */
#define REVOLVER_MOTOR_PIN 1

#define ANGLE_MOTOR_PIN 3

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

Bounce indexSwitch=Bounce(INDEX_LOCK_PIN,10);


void setup(){
  pinMode(BUILT_IN_LED, OUTPUT);
}


void loop(){
  /* Check our system heartbeat */
  if(heartbeat > 1000){
    heartbeat=0;
    digitalWrite(BUILT_IN_LED,!digitalRead(BUILT_IN_LED));
  }

  //TODO: Read radio inputs   
  //TODO: Read safety signals somehow

  /* Process */
  

  run_state_machine();

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
//recovery
  seal pressureChamber (rebuild pressure, background)
  disengage cylinderClamp (some wait time)
  release indexLock (wait some time)
//reloading
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
};
State state=State.STARTUP;
State last_state=State.RESET;
ElapsedMillis timer;
void run_state_machine(){
  
  //Do pre-state machine loop process
  indexSwitch.update();


  switch(state){
    case STARTUP:
      //run our code here
      //lock index, 
      digitalWrite(INDEX_LOCK_A_PIN,true);
      //open pressure chamber
      //power ON motor?
      analogWrite(REVOLVER_MOTOR_PIN, 32?);

      if(switch.read()==false){
        state=PRESSURIZING;
      }
    break;
    case PRESSURIZING:
      // do state things
      if(timer > 3000){
        state = IDLE;
      }
    break;
    case IDLE:
    break;  
  default:
    //someone did something real weird
    state=STARTUP;
  }


  //do post-state cleanup and checks
  if(last_state != state){
    timer=0;
    last_state=state;
  }
}