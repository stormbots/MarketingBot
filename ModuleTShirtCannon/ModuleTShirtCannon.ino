
#include "MiniPID.h"
#include <elapsedMillis.h>
#include <PulsePosition.h>
#include <Bounce.h>
#include <Encoder.h>

/* Hardware: */
#define REVOLVER_MOTOR_PIN 1

#define MAG_ENCODER_A_PIN 1
#define MAG_ENCODER_B_PIN 2

#define DUMP_VALVE_A_PIN 2
#define DUMP_VALVE_B_PIN 2

#define INDEX_LOCK_A_PIN 2
#define INDEX_LOCK_B_PIN 2

#define FIRING_PLATE_A_PIN 3
#define FIRING_PLATE_B_PIN 3

#define ELEVATION_MOTOR_PIN 3

#define INDEX_LOCK_PIN 3

#define ANGLE_TOP_SWITCH_PIN 3
#define ANGLE_BOTTON_SWITCH_PIN 3

#define BUILT_IN_LED 13

#define RADIO_IN_PIN 22
#define RADIO_OUT_PIN 23

#define ANGLE_ENCODER_A_PIN 4
#define ANGLE_ENCODER_B_PIN 4
//Degrees measured from horizon
#define ANGLE_MIN_DEGREES 0
#define ANGLE_MAX_DEGREES 90
#define ANGLE_ENCODER_RANGE 4096

PulsePositionOutput radioCannonOutput;
PulsePositionInput radioCannonInput;

Bounce indexSwitch = Bounce(INDEX_LOCK_PIN,10);
MiniPID pid = MiniPID(1.0,0.0,0.0);

Encoder angleEncoder(pin1, pin2);

Bounce angleTopSwitch = Bounce(ELEVATION_SWITCH_PIN ,10);
Bounce angleBottomSwitch = Bounce(DEPRESSION_SWITCH_PIN,10);
elapsedMillis verticalDriveTimer;
elapsedMillis timer;
elapsedMillis cannonHeartbeat;
void setup(){
  pinMode(BUILT_IN_LED, OUTPUT);
  radioCannonOutput.begin(RADIO_OUT_PIN);
  radioCannonInput.begin(RADIO_IN_PIN);

  //Configure angle PID
  pid.setOutputLimits(-127,127);
  pid.setMaxIOutput(20);
	// pid.setSetpointRange(double angle per cycle)

  //wait to make sure our sensors are online before initializing
  delay(30); 
  //TODO: read sensor pulses and set position from Absolute encoder reading

}


void loop(){
  /* Check our system heartbeat */
  if (cannonHeartbeat > 1000){
    cannonHeartbeat=0;
    digitalWrite(BUILT_IN_LED, !digitalRead(BUILT_IN_LED));
  }

  /*********************************************/
  /** Read input signals and parse them out */
  /*********************************************/
  //TODO: Read radio inputs
  //TODO: Read safety signals somehow
  bool cannonTrigger = radioCannonInput.read(6) <= 1250;
  double targetAngle = radioCannonInput.read(3);
  targetAngle = clamp(targetAngle,1000,2000);
  targetAngle = map(targetAngle,1000,2000,ANGLE_MIN_DEGREES,ANGLE_MAX_DEGREES);

  
  /*********************************************/
  /********* Process Cannon Elevation  *********/
  /*********************************************/
  double sensorAngle = angleEncoder.read(); //TODO: Read properly from mag encoder
  sensorAngle = map(sensorAngle,0,ANGLE_ENCODER_RANGE,ANGLE_MIN_DEGREES,ANGLE_MAX_DEGREES);
  angleTopSwitch.update();
  angleBottomSwitch.update();
  //NOTE: PID returns +/- range, so offset by motor neutral
  double angleMotorOutput=127+pid.getOutput(sensorAngle,targetAngle);
 
  if (angleTopSwitch.read() == false){
    angleEncoder.write(ANGLE_ENCODER_RANGE);
    if (angleMotorOutput > 127){
      angleMotorOutput = 127;
    }
  }
  else if(angleBottomSwitch.read() == false){
    angleEncoder.write(0);
    if (angleMotorOutput < 127){
      angleMotorOutput = 127;
    }
  }
  analogWrite(ELEVATION_MOTOR_PIN,angleMotorOutput);


  /*****************************************/
  /********* Process State Machine *********/
  /****************************************/
  //Manages solenoids, simple switches, simple motors
  run_state_machine(cannonTrigger);
  
  delay(10);
}

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
enum State state = STARTUP;
enum State last_state=RESET;

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
      digitalWrite(FIRING_PLATE_A_PIN,true);
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
      digitalWrite(FIRING_PLATE_A_PIN,true);
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
      digitalWrite(FIRING_PLATE_A_PIN,false);
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
      digitalWrite(FIRING_PLATE_A_PIN,false);
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
      digitalWrite(FIRING_PLATE_A_PIN,true);
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
      digitalWrite(FIRING_PLATE_A_PIN,true);
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
      digitalWrite(FIRING_PLATE_A_PIN,true);
      //dump valve closed
      digitalWrite(DUMP_VALVE_A_PIN,false);
      //if switch is tripped. Fallback safety timer.
      indexSwitch.update();
      if(indexSwitch.read()==false || timer>3000){
        state=PRESSURIZING;
      }
    break;
    default:
      //things got wacky
      state=STARTUP;
    break;
  }
 

  if (last_state != state){
    timer=0;
    last_state=state;
  }
}
