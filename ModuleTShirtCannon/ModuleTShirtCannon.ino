//Libraries 
#include "MiniPID.h"
#include <elapsedMillis.h>
#include <PulsePosition.h>
#include <Bounce.h>
#include <Encoder.h>
#include <Servo.h>
#include "PinNames.h"
#include "encoderHoming.h"


//Degrees measured from horizon
#define ELEVATION_MAX_DEGREES 40
#define ELEVATION_MIN_DEGREES -10
#define ELEVATION_ENCODER_RANGE 1261

  // Named constants for solenoids
#define INDEX_LOCKED LOW
#define INDEX_UNLOCKED HIGH

#define FIRING_PIN_1_OPEN LOW
#define FIRING_PIN_1_CLOSED HIGH
#define FIRING_PIN_2_OPEN LOW
#define FIRING_PIN_2_CLOSED HIGH

#define FIRING_PLATE_OPEN LOW
#define FIRING_PLATE_CLOSED HIGH

//Constants for motors
#define NEUTRAL_OUTPUT 1500
#define FORWARD_OUTPUT 75//Increase Forward Output to Increase Motor Speeds

//Radios
//PulsePositionOutput radioCannonOutput;
PulsePositionInput radioCannonInput;

//PID for elevationServo
MiniPID elevationPID = MiniPID(10,0.0,0.0);//TODO needs tuning

//Encoder for the elevationServo
Encoder elevationEncoder(ELEVATION_ENCODER_PIN_1, ELEVATION_ENCODER_PIN_2);

//Current Position of the elevation 
float currentAngle = 0.0;

//Declare Motors
Servo cylinderServo;
Servo elevationServo;

//Timers
elapsedMillis verticalDriveTimer;
elapsedMillis timer;
elapsedMillis cannonHeartbeat;

//Booleans
bool cannonTrigger = false;
bool pressureRelease = false;
bool elevationAligned = false;

void setup(){
  //Starts Serial For Debug
  Serial.begin(9600);// Keep at 9600 baum

  //Setting Pin Modes
  pinMode(BUILT_IN_LED, OUTPUT);
  pinMode(CYLINDER_LOCK_PIN, OUTPUT);
  pinMode(FIRING_PIN_1,OUTPUT);
  pinMode(FIRING_PIN_2, OUTPUT);
  pinMode(FIRING_PLATE_PIN,OUTPUT);
  
  //Begin Reading Radio
  radioCannonInput.begin(RADIO_IN_PIN);
  //radioCannonOutput.begin(RADIO_OUT_PIN);

  //Configure elevationPID limits
  elevationPID.setOutputLimits(-175,175);
  elevationPID.setMaxIOutput(20);

  //Set Motors to 0 On Startup and attach servos to pins
  elevationServo.attach(ELEVATION_MOTOR_PIN);
  elevationServo.writeMicroseconds(NEUTRAL_OUTPUT);
  cylinderServo.attach(CYLINDER_MOTOR_PIN);
  cylinderServo.writeMicroseconds(NEUTRAL_OUTPUT);

 
  //If we have not gotten our home position do not progrees 
  encoderHome();
  currentAngle = getEncoderPosition();
  elevationEncoder.write(currentAngle);

  //Wait to ensure everything gets time to respond
  delay(30); 
}


void loop(){
  //Heartbeat LED
  if (cannonHeartbeat > 1000){
    cannonHeartbeat=0;
    digitalWrite(BUILT_IN_LED, !digitalRead(BUILT_IN_LED));
  }
  
  //Check if radio is connected
  if (radioCannonInput.read(8)<200){
    Serial.println("No Signal");
    delay(100);
    return;
  }
  
  //Read Radio Channels
  double targetAngle = radioCannonInput.read(3);
  bool cannonTrigger = radioCannonInput.read(7) >= 1500;
  bool pressureReleaseLocked = false;
  bool pressureReleaseUnlocked =false;
  //only move to unlocked if switch is in the middle position
  if (radioCannonInput.read(8) >= 1600){
    pressureReleaseUnlocked = true;
  }
  else if (radioCannonInput.read(8)>=1300){
    pressureReleaseLocked = true;
  }
  
  //Map/Lerp the Radio Signal to Angles
  targetAngle = map(targetAngle,1000,2000,ELEVATION_MIN_DEGREES,ELEVATION_MAX_DEGREES);

  //Check if the positions  on the controller and the hardware are similar
  if (targetAngle >= currentAngle - 10 && targetAngle <= currentAngle + 10){
    elevationAligned = true;
  }
  if (elevationAligned == false){
    //Serial.println("Move the elevation to align with hardware");
    Serial.println("Incorrect Alignment");
    Serial.println(currentAngle);
    return;
  }
  
  //Read our position from Encoder
  currentAngle = elevationEncoder.read(); 
  
  //Map/Lerp value from Encoder to Angles
  currentAngle = map(currentAngle,0,ELEVATION_ENCODER_RANGE,ELEVATION_MAX_DEGREES,ELEVATION_MIN_DEGREES);
  
  //Write to elevationServo using PID and the current and target angles
  double angleMotorOutput=NEUTRAL_OUTPUT+elevationPID.getOutput(currentAngle,targetAngle);
  elevationServo.writeMicroseconds(angleMotorOutput);
  
  //Run State Machine
  run_state_machine(cannonTrigger, pressureReleaseUnlocked,pressureReleaseLocked);
  
  delay(10);
}

//States in the state machine
enum State{
  STARTUP,
  PRESSURIZING,
  IDLE,
  DUMPPRESSURE_UNLOCKED,
  DUMPPRESSURE_LOCKED,
  FIRING,
  RECOVERY,
  RELOAD_UNLOCKED,
  RELOAD_LOCKED,
  RESET
};

enum State state = STARTUP;
enum State last_state=RESET;

void run_state_machine(bool cannonTrigger, bool pressureReleaseUnlocked, bool pressureReleaseLocked){
  switch(state){
    case STARTUP:
//      Serial.println("STARTUP");
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
    break;
    case PRESSURIZING:
//      Serial.println("PRESSURIZING");

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
//      Serial.println("IDLE");
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
      //if the pressure release is swithed move to dump pressure
      else if(pressureReleaseUnlocked){
        state=DUMPPRESSURE_UNLOCKED;
      }
      else if (pressureReleaseLocked){
        state=DUMPPRESSURE_LOCKED;
      }
    break;
    case DUMPPRESSURE_UNLOCKED:
//      Serial.println("DUMPPRESSURE_UNLOCKED");

      //index unlocked
      digitalWrite(CYLINDER_LOCK_PIN, INDEX_UNLOCKED);
      //Revolver motor off
      cylinderServo.writeMicroseconds(NEUTRAL_OUTPUT);
      //firing plate opened
      digitalWrite(FIRING_PLATE_PIN, FIRING_PLATE_OPEN);
      //dump valve closed 
      digitalWrite(FIRING_PIN_1, FIRING_PIN_1_CLOSED);
      digitalWrite(FIRING_PIN_2, FIRING_PIN_2_OPEN);
      
      //if trigger is not pressed and pressure release is turned back off return to idle
      if (cannonTrigger == false && pressureReleaseUnlocked == false){
        state = IDLE;
      }
      else if (pressureReleaseLocked == true){
        state = DUMPPRESSURE_LOCKED;
      }
    break;
    case DUMPPRESSURE_LOCKED:
//      Serial.println("DUMPPRESSURE_LOCKED");

      //index locked
      digitalWrite(CYLINDER_LOCK_PIN, INDEX_LOCKED);
      //Revolver motor off
      cylinderServo.writeMicroseconds(NEUTRAL_OUTPUT);
      //firing plate opened
      digitalWrite(FIRING_PLATE_PIN, FIRING_PLATE_OPEN);
      //dump valve closed 
      digitalWrite(FIRING_PIN_1, FIRING_PIN_1_CLOSED);
      digitalWrite(FIRING_PIN_2, FIRING_PIN_2_OPEN);
      
      //if trigger is not pressed and pressure release is turned back off return to idle
      if (cannonTrigger == false && pressureReleaseLocked == false){
        state = IDLE;
      }
      else if (pressureReleaseUnlocked == true){
        state = DUMPPRESSURE_UNLOCKED;
      }
    break;
    case FIRING:
//      Serial.println("FIRING");
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
//      Serial.println("RECOVERY");
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
      if (timer > 5000){
        state=RELOAD_UNLOCKED;
      }
    break;
    case RELOAD_UNLOCKED:
//      Serial.println("RELOAD_UNLOCKED");
      //index unlocked
      digitalWrite(CYLINDER_LOCK_PIN,INDEX_UNLOCKED);
      //Serial.println(digitalRead(FIRING_PLATE_PIN));
      //revolver motor on
      cylinderServo.writeMicroseconds(NEUTRAL_OUTPUT+FORWARD_OUTPUT);
      //firing plate open
      digitalWrite(FIRING_PLATE_PIN,FIRING_PLATE_OPEN);

      
      //dump valve closed
      digitalWrite(FIRING_PIN_1,FIRING_PIN_1_CLOSED);
      digitalWrite(FIRING_PIN_2,FIRING_PIN_2_OPEN);
      // if timer expires advance to RELOAD_LOCKED
      if (timer > 600){
        state=RELOAD_LOCKED;
      }
    break;
    case RELOAD_LOCKED:
     //Serial.println("RELOAD_LOCKED");
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
      //Serial.println(timer);
      if(timer>800){
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
      case DUMPPRESSURE_UNLOCKED : Serial.println("Dump Pressure");break;;
      case DUMPPRESSURE_LOCKED : Serial.println("Dump Pressure");break;;
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
