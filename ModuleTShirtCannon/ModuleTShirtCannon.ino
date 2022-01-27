
#include "MiniPID.h"
#include <elapsedMillis.h>
#include <PulsePosition.h>
#include <Bounce.h>
#include <Encoder.h>
#include <Servo.h>

/* Hardware: */
#define REVOLVER_MOTOR_PIN 10
Servo revolverServo;
#define FIRING_PIN_1 4
#define FIRING_PIN_2 5

#define INDEX_LOCK_PIN 7 

#define FIRING_PLATE_PIN 6

#define ELEVATION_MOTOR_PIN 9
Servo  elevationServo;

//#define TERM_AUX_1 19
//#define TERM_AUX_2 20
//#define TERM_AUX_3 21

#define ANGLE_TOP_SWITCH_PIN 21
#define ANGLE_BOTTOM_SWITCH_PIN 18

#define INDEX_SWITCH_PIN 20

#define BUILT_IN_LED 13

#define RADIO_IN_PIN 23
#define RADIO_OUT_PIN 22

#define ANGLE_ENCODER_A_PIN 0
#define ANGLE_ENCODER_B_PIN 11
#define ANGLE_ENCODER_ABSOLUTE_PIN 1

//Degrees measured from horizon
#define ANGLE_MIN_DEGREES 0
#define ANGLE_MAX_DEGREES 90
#define ANGLE_ENCODER_RANGE 4096

// Named values for actuators
#define INDEX_LOCKED LOW
#define INDEX_UNLOCKED HIGH
#define FIRING_PIN_1_OPEN HIGH
#define FIRING_PIN_1_CLOSED LOW
#define FIRING_PIN_2_OPEN HIGH
#define FIRING_PIN_2_CLOSED LOW
#define FIRING_PLATE_OPEN HIGH
#define FIRING_PLATE_CLOSED LOW
#define INDEX_SWITCH_PRESSED LOW

#define NEUTRAL_OUTPUT 1500
#define FORWARD_OUTPUT 75

PulsePositionOutput radioCannonOutput;
PulsePositionInput radioCannonInput;

MiniPID pid = MiniPID(0.0,0.0,0.0);

Encoder angleEncoder(ANGLE_ENCODER_A_PIN, ANGLE_ENCODER_B_PIN);

Bounce indexSwitch = Bounce(INDEX_SWITCH_PIN,10);
Bounce angleTopSwitch = Bounce(ANGLE_TOP_SWITCH_PIN ,10);
Bounce angleBottomSwitch = Bounce(ANGLE_BOTTOM_SWITCH_PIN,10);

elapsedMillis verticalDriveTimer;
elapsedMillis timer;
elapsedMillis cannonHeartbeat;
bool has_been_enabled = false;

void setup(){
  Serial.begin(9600);
  pinMode(BUILT_IN_LED, OUTPUT);
  pinMode(INDEX_SWITCH_PIN, INPUT);
  pinMode(INDEX_LOCK_PIN, OUTPUT);
  pinMode(FIRING_PIN_1,OUTPUT);
  pinMode(FIRING_PIN_2, OUTPUT);
  pinMode(FIRING_PLATE_PIN,OUTPUT);
  
  //radioCannonOutput.begin(RADIO_OUT_PIN); //unused, but wired to controller board
  radioCannonInput.begin(RADIO_IN_PIN);

  //Configure angle PID
  pid.setOutputLimits(-500,500);
  pid.setMaxIOutput(20);//change
	// pid.setSetpointRange(double angle per cycle)

  elevationServo.attach(ELEVATION_MOTOR_PIN);
  elevationServo.writeMicroseconds(NEUTRAL_OUTPUT);
  revolverServo.attach(REVOLVER_MOTOR_PIN);
  revolverServo.writeMicroseconds(NEUTRAL_OUTPUT);
  
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
  // revolverServo.writeMicroseconds(1500+100);
  // digitalWrite(FIRING_PLATE_PIN,true);
  // digitalWrite(FIRING_PIN_1,true);
  //return; //Use when debugging to prevent other actuator changes
 


  /*********************************************/
  /** Read input signals and parse them out */
  /*********************************************/
  //TODO: Read radio inputs
  //TODO: Read safety signals somehow
    

  bool cannonTrigger = radioCannonInput.read(8) >= 1500;
  //Serial.println(radioCannonInput.read(8));
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
  double angleMotorOutput=NEUTRAL_OUTPUT+pid.getOutput(sensorAngle,targetAngle);
 
  if (angleTopSwitch.read() == false){
    angleEncoder.write(ANGLE_ENCODER_RANGE);
    if (angleMotorOutput > NEUTRAL_OUTPUT){
      angleMotorOutput = NEUTRAL_OUTPUT;
    }
  }
  else if(angleBottomSwitch.read() == false){
    angleEncoder.write(0);
    if (angleMotorOutput < NEUTRAL_OUTPUT){
      angleMotorOutput = NEUTRAL_OUTPUT;
    }
  }
  elevationServo.writeMicroseconds(angleMotorOutput);

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
  //Serial.println(cannonTrigger);
  indexSwitch.update();
  if((indexSwitch.fallingEdge()|| indexSwitch.risingEdge()) && indexSwitch.read()==INDEX_SWITCH_PRESSED){
    Serial.println("Index Switch Pressed");
  }

  switch(state){
    case STARTUP:
      //index locked
      digitalWrite(INDEX_LOCK_PIN,INDEX_LOCKED);
      //revolver motor on
      revolverServo.writeMicroseconds(NEUTRAL_OUTPUT+FORWARD_OUTPUT);
      //firing plate open
      digitalWrite(FIRING_PLATE_PIN,FIRING_PLATE_CLOSED);
      //Firing valve closed
      digitalWrite(FIRING_PIN_1,FIRING_PIN_1_CLOSED);
      digitalWrite(FIRING_PIN_2,FIRING_PIN_2_OPEN);
      //if the indexSwitch is tripped move to PRESSURIZING
      if (indexSwitch.read() == INDEX_SWITCH_PRESSED){
        state=PRESSURIZING;
      }
    break;
    case PRESSURIZING:
      //index locked
      digitalWrite(INDEX_LOCK_PIN,INDEX_LOCKED);
      //revolver motor off
      revolverServo.writeMicroseconds(NEUTRAL_OUTPUT);
      //firing plate open
      digitalWrite(FIRING_PLATE_PIN,FIRING_PLATE_CLOSED);
      //dump valve closed
      digitalWrite(FIRING_PIN_1,FIRING_PIN_1_CLOSED);
      digitalWrite(FIRING_PIN_2,FIRING_PIN_2_OPEN);
      //if the trigger for the cannon is not pressed and the timer has expired move to IDLE
      
      if (cannonTrigger == false && timer>3000){
        state=IDLE;
      }
    break;
    case IDLE:
      //index locked
      digitalWrite(INDEX_LOCK_PIN,INDEX_LOCKED);
      //revolver motor off
      revolverServo.writeMicroseconds(NEUTRAL_OUTPUT);
      //firing plate closed
      digitalWrite(FIRING_PLATE_PIN,FIRING_PLATE_CLOSED);
      //dump valve closed
      digitalWrite(FIRING_PIN_1,FIRING_PIN_1_CLOSED);
      digitalWrite(FIRING_PIN_2,FIRING_PIN_2_OPEN);
      // if the trigger for the cannon is pressed move to FIRING
      if (cannonTrigger){
        state = FIRING;
      }
    break;
    case FIRING:
      //index locked
      digitalWrite(INDEX_LOCK_PIN,INDEX_LOCKED);
      //Serial.println(digitalRead(INDEX_LOCK_PIN));
      //revolver motor off
      revolverServo.writeMicroseconds(NEUTRAL_OUTPUT);
      //firing plate closed
      digitalWrite(FIRING_PLATE_PIN,FIRING_PLATE_CLOSED);
      //Serial.println(digitalRead(FIRING_PLATE_PIN));
      //dump valve open
      digitalWrite(FIRING_PIN_1,FIRING_PIN_1_OPEN);
      digitalWrite(FIRING_PIN_2,FIRING_PIN_2_CLOSED);
      // if the timer expires advance to RECOVERY
      if (timer > 1000){
        state = RECOVERY;
      }
    break;
    case RECOVERY:
      // index unlocked
      digitalWrite(INDEX_LOCK_PIN,INDEX_UNLOCKED);
      //revolver motor off
      revolverServo.writeMicroseconds(NEUTRAL_OUTPUT);
      //firing plate open
      digitalWrite(FIRING_PLATE_PIN,FIRING_PLATE_CLOSED);
      //Serial.println(digitalRead(FIRING_PLATE_PIN));
      // dump valve closed
      digitalWrite(FIRING_PIN_1,FIRING_PIN_1_CLOSED);
      digitalWrite(FIRING_PIN_2,FIRING_PIN_2_OPEN);
      // if timer expires advance to RELOAD_UNLOCKED
      if (timer > 3000){
        state=RELOAD_UNLOCKED;
      }
    break;
    case RELOAD_UNLOCKED:
      //index unlocked
      digitalWrite(INDEX_LOCK_PIN,INDEX_UNLOCKED);
      //Serial.println(digitalRead(FIRING_PLATE_PIN));
      //revolver motor on
      revolverServo.writeMicroseconds(NEUTRAL_OUTPUT+FORWARD_OUTPUT);
      //firing plate open
      digitalWrite(FIRING_PLATE_PIN,FIRING_PLATE_OPEN);
      delay(1000);
      //dump valve closed
      digitalWrite(FIRING_PIN_1,FIRING_PIN_1_CLOSED);
      digitalWrite(FIRING_PIN_2,FIRING_PIN_2_OPEN);
      // if timer expires advance to RELOAD_LOCKED
      if (timer > 25){
        state=RELOAD_LOCKED;
      }
    break;
    case RELOAD_LOCKED:
      //index locked
      digitalWrite(INDEX_LOCK_PIN,INDEX_LOCKED);
      //Serial.println(digitalRead(INDEX_LOCK_PIN));
      //revolver motor on
      revolverServo.writeMicroseconds(NEUTRAL_OUTPUT+FORWARD_OUTPUT);
      //firing plate open
      digitalWrite(FIRING_PLATE_PIN,FIRING_PLATE_OPEN);
      
      //dump valve closed
      digitalWrite(FIRING_PIN_1,FIRING_PIN_1_CLOSED);
      digitalWrite(FIRING_PIN_2,FIRING_PIN_2_OPEN);
      //if the indexSwitch is tripped or time expires move to PRESSURIZING
      if(timer>300){
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
