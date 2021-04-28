
#include "MiniPID.h"
#include <elapsedMillis.h>
#include <PulsePosition.h>
#include <Bounce.h>
#include <Encoder.h>

/* Hardware: */
#define REVOLVER_MOTOR_PIN 10

#define DUMP_VALVE_PIN 16

#define INDEX_LOCK_PIN 18

#define FIRING_PLATE_PIN 17

#define ELEVATION_MOTOR_PIN 9

#define TERM_AUX_1 19
#define TERM_AUX_2 20
#define TERM_AUX_3 21

#define ANGLE_TOP_SWITCH_PIN 5
#define ANGLE_BOTTOM_SWITCH_PIN 6

#define INDEX_SWITCH_PIN 7

#define BUILT_IN_LED 13

#define RADIO_IN_PIN 23
#define RADIO_OUT_PIN 22

#define ANGLE_ENCODER_A_PIN 0
#define ANGLE_ENCODER_B_PIN 15
#define ANGLE_ENCODER_ABSOLUTE_PIN 1

//Degrees measured from horizon
#define ANGLE_MIN_DEGREES 0
#define ANGLE_MAX_DEGREES 90
#define ANGLE_ENCODER_RANGE 4096

// Named values for actuators
#define INDEX_LOCKED true
#define INDEX_UNLOCKED false
#define DUMP_VALVE_OPEN true
#define DUMP_VALVE_CLOSED false
#define FIRING_PLATE_OPEN true
#define FIRING_PLATE_CLOSED false
#define INDEX_SWITCHED_PRESSED false


PulsePositionOutput radioCannonOutput;
PulsePositionInput radioCannonInput;

MiniPID pid = MiniPID(1.0,0.0,0.0);

Encoder angleEncoder(ANGLE_ENCODER_A_PIN, ANGLE_ENCODER_B_PIN);

Bounce indexSwitch = Bounce(INDEX_SWITCH_PIN,10);
Bounce angleTopSwitch = Bounce(ANGLE_TOP_SWITCH_PIN ,10);
Bounce angleBottomSwitch = Bounce(ANGLE_BOTTOM_SWITCH_PIN,10);

elapsedMillis verticalDriveTimer;
elapsedMillis timer;
elapsedMillis cannonHeartbeat;
bool has_been_enabled = false;

void setup(){
  pinMode(BUILT_IN_LED, OUTPUT);
  pinMode(INDEX_SWITCH_PIN, INPUT);
  //radioCannonOutput.begin(RADIO_OUT_PIN); //unused, but wired to controller board
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

  // //DEBUG// Hard coded values to determine actuator polarity
  // digitalWrite(REVOLVER_MOTOR_PIN,true);
  // digitalWrite(INDEX_LOCK_PIN,true);
  // analogWrite(REVOLVER_MOTOR_PIN,127+32);
  // digitalWrite(FIRING_PLATE_PIN,true);
  // digitalWrite(DUMP_VALVE_PIN,true);
  //return; //Use when debugging to prevent other actuator changes



  /*********************************************/
  /** Read input signals and parse them out */
  /*********************************************/
  //TODO: Read radio inputs
  //TODO: Read safety signals somehow
    

  bool cannonTrigger = radioCannonInput.read(6) <= 1250;
  double targetAngle = radioCannonInput.read(3);
  
  targetAngle = map(targetAngle,1000,2000,ANGLE_MIN_DEGREES,ANGLE_MAX_DEGREES);

  //
  //TODO: Do not run actuators until we've enabled the robot at least once
  //
  //This section relies on a more robust Disable muxing on the
  //reciever
  //if(!has_been_enabled)return;


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
  if(indexSwitch.fallingEdge()|| indexSwitch.risingEdge() && indexSwitch.read()==INDEX_SWITCHED_PRESSED){
    Serial.println("Index Switch Pressed");
  }

  switch(state){
    case STARTUP:
      //index locked
      digitalWrite(INDEX_LOCK_PIN,INDEX_LOCKED);
      //revolver motor on
      analogWrite(REVOLVER_MOTOR_PIN,127+32);
      //firing plate open
      digitalWrite(FIRING_PLATE_PIN,FIRING_PLATE_OPEN);
      //dump valve closed
      digitalWrite(DUMP_VALVE_PIN,DUMP_VALVE_CLOSED);
      //if the indexSwitch is tripped move to PRESSURIZING
      if (indexSwitch.read() == INDEX_SWITCHED_PRESSED){
        state=PRESSURIZING;
      }
    break;
    case PRESSURIZING:
      //index locked
      digitalWrite(INDEX_LOCK_PIN,INDEX_LOCKED);
      //revolver motor off
      analogWrite(REVOLVER_MOTOR_PIN,127);
      //firing plate open
      digitalWrite(FIRING_PLATE_PIN,FIRING_PLATE_OPEN);
      //dump valve closed
      digitalWrite(DUMP_VALVE_PIN,DUMP_VALVE_CLOSED);
      //if the trigger for the cannon is not pressed and the timer has expired move to IDLE
      if (cannonTrigger == false && timer>3000){
        state=IDLE;
      }
    break;
    case IDLE:
      //index locked
      digitalWrite(INDEX_LOCK_PIN,INDEX_LOCKED);
      //revolver motor off
      analogWrite(REVOLVER_MOTOR_PIN,127);
      //firing plate closed
      digitalWrite(FIRING_PLATE_PIN,FIRING_PLATE_CLOSED);
      //dump valve closed
      digitalWrite(DUMP_VALVE_PIN,DUMP_VALVE_CLOSED);
      // if the trigger for the cannon is pressed move to FIRING
      if (cannonTrigger){
        state = FIRING;
      }
    break;
    case FIRING:
      //index locked
      digitalWrite(INDEX_LOCK_PIN,INDEX_LOCKED);
      //revolver motor off
      analogWrite(REVOLVER_MOTOR_PIN,127);
      //firing plate closed
      digitalWrite(FIRING_PLATE_PIN,FIRING_PLATE_CLOSED);
      //dump valve open
      digitalWrite(DUMP_VALVE_PIN,DUMP_VALVE_OPEN);
      // if the timer expires advance to RECOVERY
      if (timer > 3000){
        state = RECOVERY;
      }
    break;
    case RECOVERY:
      // index unlocked
      digitalWrite(INDEX_LOCK_PIN,INDEX_UNLOCKED);
      //revolver motor off
      analogWrite(REVOLVER_MOTOR_PIN,127);
      //firing plate open
      digitalWrite(FIRING_PLATE_PIN,FIRING_PLATE_OPEN);
      // dump valve closed
      digitalWrite(DUMP_VALVE_PIN,DUMP_VALVE_CLOSED);
      // if timer expires advance to RELOAD_UNLOCKED
      if (timer > 3000){
        state=RELOAD_UNLOCKED;
      }
    break;
    case RELOAD_UNLOCKED:
      //index unlocked
      digitalWrite(INDEX_LOCK_PIN,INDEX_UNLOCKED);
      //revolver motor on
      analogWrite(REVOLVER_MOTOR_PIN,127+32);//placeholder value 1/8 speed
      //firing plate open
      digitalWrite(FIRING_PLATE_PIN,FIRING_PLATE_OPEN);
      //dump valve closed
      digitalWrite(DUMP_VALVE_PIN,DUMP_VALVE_CLOSED);
      // if timer expires advance to RELOAD_LOCKED
      if (timer > 3000){
        state=RELOAD_LOCKED;
      }
    break;
    case RELOAD_LOCKED:
      //index locked
      digitalWrite(INDEX_LOCK_PIN,INDEX_LOCKED);
      //revolver motor on
      analogWrite(REVOLVER_MOTOR_PIN,127+32);//placeholder value 1/8 speed
      //firing plate open
      digitalWrite(FIRING_PLATE_PIN,FIRING_PLATE_OPEN);
      //dump valve closed
      digitalWrite(DUMP_VALVE_PIN,DUMP_VALVE_CLOSED);
      //if the indexSwitch is tripped or time expires move to PRESSURIZING
      if(indexSwitch.read()==INDEX_SWITCHED_PRESSED || timer>3000){
        state=PRESSURIZING;
      }
    break;
    default:
      //things got wacky
      state=STARTUP;
    break;
  }
 

  if (last_state != state){
    Serial.print("Entering state ");
    switch(state){
      case STARTUP : Serial.println("Startup");break;;
      case PRESSURIZING : Serial.println("Pressurizing");break;;
      case IDLE : Serial.println("Idle");break;;
      case FIRING : Serial.println("Firing");break;;
      case RECOVERY : Serial.println("Recovery");break;;
      case RELOAD_UNLOCKED : Serial.println("Reload_unlocked");break;;
      case RELOAD_LOCKED : Serial.println("Reload_locked");break;;
      case RESET : Serial.println("reset");break;;
    }
    timer=0;
    last_state=state;
  }
}
