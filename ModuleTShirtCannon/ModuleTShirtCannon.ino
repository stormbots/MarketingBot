//Libraries 
#include "MiniPID.h"
#include <elapsedMillis.h>
#include <PulsePosition.h>
#include <Bounce.h>
#include <Encoder.h>
#include <Servo.h>

//Teensy Pins

#define CYLINDER_MOTOR_PIN 10     

#define FIRING_PIN_1 4
#define FIRING_PIN_2 5

//Solenoid that locks the chamber in place
#define CYLINDER_LOCK_PIN 7 

#define FIRING_PLATE_PIN 6

#define ELEVATION_MOTOR_PIN 9

//Empty Pins for Controls Use
#define TERM_AUX_1 18
#define TERM_AUX_2 20
#define TERM_AUX_3 21

#define BUILT_IN_LED 13

#define RADIO_IN_PIN 23
#define RADIO_OUT_PIN 22

#define ELEVATION_ENCODER_PIN_1 0 //TODO Check that this is the correct pin number
#define ELEVATION_ENCODER_PIN_2 11  //TODO Check that this is the correct pin number

//Degrees measured from horizon
#define ELEVATION_MIN_DEGREES 0
#define ELEVATION_MAX_DEGREES 90
#define ELEVATION_ENCODER_RANGE 4096

// Named constants for solenoids
#define INDEX_LOCKED LOW
#define INDEX_UNLOCKED HIGH

#define FIRING_PIN_1_OPEN HIGH
#define FIRING_PIN_1_CLOSED LOW
#define FIRING_PIN_2_OPEN HIGH
#define FIRING_PIN_2_CLOSED LOW

#define FIRING_PLATE_OPEN HIGH
#define FIRING_PLATE_CLOSED LOW

//Constants for motors
#define NEUTRAL_OUTPUT 1500
#define FORWARD_OUTPUT 75//Increase Forward Output to Increase Motor Speeds

//Radios
PulsePositionOutput radioCannonOutput;
PulsePositionInput radioCannonInput;

//PID for elevationServo
MiniPID elevationPID = MiniPID(0.0,0.0,0.0);//TODO needs tuning

//Encoder for the elevationServo
Encoder elevationEncoder(ELEVATION_ENCODER_PIN_1, ELEVATION_ENCODER_PIN_2);

//Declare Motors
Servo cylinderServo;
Servo elevationServo;

//Timers
elapsedMillis verticalDriveTimer;
elapsedMillis timer;
elapsedMillis cannonHeartbeat;

//Booleans
bool has_been_enabled = false;
bool cannonTrigger = false;

void setup(){
  //Starts Serial For Debug
  Serial.begin(9600);// Keep at 9600 baum

  //Setting Pin Modes
  pinMode(BUILT_IN_LED, OUTPUT);
  pinMode(INDEX_SWITCH_PIN, INPUT);
  pinMode(CYLINDER_LOCK_PIN, OUTPUT);
  pinMode(FIRING_PIN_1,OUTPUT);
  pinMode(FIRING_PIN_2, OUTPUT);
  pinMode(FIRING_PLATE_PIN,OUTPUT);

  //Begin Reading Radio
  radioCannonInput.begin(RADIO_IN_PIN);

  //Configure elevationPID
  elevationPID.setOutputLimits(-500,500);
  elevationPID.setMaxIOutput(20);//TODO needs tuning
	//elevationPID.setSetpointRange(double angle per cycle)

  //Set Motors to 0 On Startup
  elevationServo.attach(ELEVATION_MOTOR_PIN);
  elevationServo.writeMicroseconds(NEUTRAL_OUTPUT);
  cylinderServo.attach(CYLINDER_MOTOR_PIN);
  cylinderServo.writeMicroseconds(NEUTRAL_OUTPUT);
  
  //Wait to Allow Everything On Startup
  delay(30); 
}


void loop(){
  //Heartbeat
  if (cannonHeartbeat > 1000){
    cannonHeartbeat=0;
    digitalWrite(BUILT_IN_LED, !digitalRead(BUILT_IN_LED));
  }

  //Read Radio Channels
  bool cannonTrigger = radioCannonInput.read(8) >= 1500;
  double targetAngle = radioCannonInput.read(3);

  //Map/Lerp the Radio Signal to Angles
  targetAngle = map(targetAngle,1000,2000,ELEVATION_MIN_DEGREES,ELEVATION_MAX_DEGREES);

  //Read from Encoder
  double sensorAngle = elevationEncoder.read(); 

  //Map/Lerp value from Encoder to Angles
  sensorAngle = map(sensorAngle,0,ELEVATION_ENCODER_RANGE,ELEVATION_MIN_DEGREES,ELEVATION_MAX_DEGREES);

  //Write to elevationServo using PID
  double angleMotorOutput=NEUTRAL_OUTPUT+elevationPID.getOutput(sensorAngle,targetAngle);
  elevationServo.writeMicroseconds(angleMotorOutput);

  //Run State Machine
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
  switch(state){
    case STARTUP:
      
      //The Index should be Locked
      digitalWrite(CYLINDER_LOCK_PIN,INDEX_LOCKED);
      
      //Cylinder Motor should be On
      cylinderServo.writeMicroseconds(NEUTRAL_OUTPUT+FORWARD_OUTPUT);
      
      //Firing Plate should be Closed
      digitalWrite(FIRING_PLATE_PIN,FIRING_PLATE_CLOSED);
      
      //Not Firing
      digitalWrite(FIRING_PIN_1,FIRING_PIN_1_CLOSED);
      digitalWrite(FIRING_PIN_2,FIRING_PIN_2_OPEN);
      
      state=PRESSURIZING;
      }
    break;
    case PRESSURIZING:
      //index locked
      digitalWrite(CYLINDER_LOCK_PIN,INDEX_LOCKED);
      //revolver motor off
      cylinderServo.writeMicroseconds(NEUTRAL_OUTPUT);
      //firing plate open
      digitalWrite(FIRING_PLATE_PIN,FIRING_PLATE_CLOSED);
      
      //Not Firing
      digitalWrite(FIRING_PIN_1,FIRING_PIN_1_CLOSED);
      digitalWrite(FIRING_PIN_2,FIRING_PIN_2_OPEN);
    
      //If not 
      if (cannonTrigger == false && timer>3000){
        state=IDLE;
      }
    break;
    case IDLE:
      //index locked
      digitalWrite(CYLINDER_LOCK_PIN,INDEX_LOCKED);
      //revolver motor off
      cylinderServo.writeMicroseconds(NEUTRAL_OUTPUT);
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
      digitalWrite(CYLINDER_LOCK_PIN,INDEX_LOCKED);
      //Serial.println(digitalRead(CYLINDER_LOCK_PIN));
      //revolver motor off
      cylinderServo.writeMicroseconds(NEUTRAL_OUTPUT);
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
      digitalWrite(CYLINDER_LOCK_PIN,INDEX_UNLOCKED);
      //revolver motor off
      cylinderServo.writeMicroseconds(NEUTRAL_OUTPUT);
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
      digitalWrite(CYLINDER_LOCK_PIN,INDEX_UNLOCKED);
      //Serial.println(digitalRead(FIRING_PLATE_PIN));
      //revolver motor on
      cylinderServo.writeMicroseconds(NEUTRAL_OUTPUT+FORWARD_OUTPUT);
      //firing plate open
      digitalWrite(FIRING_PLATE_PIN,FIRING_PLATE_OPEN);
      delay(1000);
      
      //dump valve closed
      digitalWrite(FIRING_PIN_1,FIRING_PIN_1_CLOSED);
      digitalWrite(FIRING_PIN_2,FIRING_PIN_2_OPEN);
      // if timer expires advance to RELOAD_LOCKED
      Serial.println(timer);
      if (timer > 200){
        state=RELOAD_LOCKED;
      }
    break;
    case RELOAD_LOCKED:
      //index locked
      digitalWrite(CYLINDER_LOCK_PIN,INDEX_LOCKED);
      //Serial.println(digitalRead());
      //revolver motor on
      cylinderServo.writeMicroseconds(NEUTRAL_OUTPUT+FORWARD_OUTPUT);
      //firing plate open
      digitalWrite(FIRING_PLATE_PIN,FIRING_PLATE_OPEN);
      
      //dump valve closed
      digitalWrite(FIRING_PIN_1,FIRING_PIN_1_CLOSED);
      digitalWrite(FIRING_PIN_2,FIRING_PIN_2_OPEN);
      //if the indexSwitch is tripped or time expires move to PRESSURIZING
      Serial.println(timer);
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
