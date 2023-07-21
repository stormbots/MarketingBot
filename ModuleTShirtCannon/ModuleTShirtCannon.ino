//Libraries 
//PID, used for precise motor control on the elevation
#include "MiniPID.h"
//Timer
#include <elapsedMillis.h>
//Radio library
#include <PulsePosition.h>
//Encoder library, used for elevation
#include <Encoder.h>
//Used to control all of the motors on the bot
#include <Servo.h>
//Subfiles
#include "PinNames.h"
#include "encoderHoming.h"

//-----------------------------------//
//All pin declarations are in the PinNames subfile
//-----------------------------------//

//Limits for the elevation motor in degrees measured from horizon
#define ELEVATION_MAX_DEGREES 40
#define ELEVATION_MIN_DEGREES -10

//Range in encoder ticks
#define ELEVATION_ENCODER_RANGE 1261

// Named constants for solenoids
//The Index is the pin that goes up and down to lock the cylinder in place
#define INDEX_LOCKED LOW
#define INDEX_UNLOCKED HIGH
//The firing pin is the solenoid that fires the main cannon
#define FIRING_PIN_1_OPEN LOW
#define FIRING_PIN_1_CLOSED HIGH
#define FIRING_PIN_2_OPEN LOW
#define FIRING_PIN_2_CLOSED HIGH
//The frining plate is the part that moves forwards and backwards to seal the firing chamber
#define FIRING_PLATE_OPEN LOW
#define FIRING_PLATE_CLOSED HIGH

//Constants for motors
//The range for motors is from 1000 to 2000 with 1000 being 100% backwards and 2000 being 100% forwards. Thus, 1500 is neutral or no movement.
#define NEUTRAL_OUTPUT 1500
//Increase Forward Output to Increase Motor Speeds, forward output is added to neutral output to produce movement
#define FORWARD_OUTPUT 100

//Radios
//This use the pulse position library to creat inputs and outputs for our radio.
//You can access documentation for this on the PJRC website
//The output is commented out in this files because this module does not pass the radio signal on to another teensy.
//PulsePositionOutput radioCannonOutput;
PulsePositionInput radioCannonInput;

//PID for elevationServo
MiniPID elevationPID = MiniPID(10,0.0,0.0);

//Encoder for the elevationServo
Encoder elevationEncoder(ELEVATION_ENCODER_PIN_1, ELEVATION_ENCODER_PIN_2);

//Current Position of the elevation in degrees
float currentAngle = 0.0;

//Declare Motors using servo library
Servo cylinderServo;
Servo elevationServo;

//Timers
//Timer for state machine
elapsedMillis stateMachineTimer;
//Timer for the LED on the teensy;
elapsedMillis cannonHeartbeat;

//Booleans
//Boolean for if the controller switch that controls firing is off(false) or on(true)
bool cannonTrigger = false;
//Boolean for checking if the controller and the elevation of the cannon are aligned
bool elevationAligned = false;

enum ManualReloadSwitch{
  DONE,
  UNLOCKED,
  UNUSED
} ;

void setup(){
  //Starts Serial For Debug
  Serial.begin(9600);// Keep at 9600 baud

  //Setting Pin Modes
  //In INPUT by default, needs to be in OUTPUT for solenoids and the inbuilt LED
  pinMode(BUILT_IN_LED, OUTPUT);
  pinMode(CYLINDER_LOCK_PIN, OUTPUT);
  pinMode(FIRING_PIN_1,OUTPUT);
  pinMode(FIRING_PIN_2, OUTPUT);
  pinMode(FIRING_PLATE_PIN,OUTPUT);
  
  //Begin reading the radio using the radio pins
  radioCannonInput.begin(RADIO_IN_PIN);
  //radioCannonOutput.begin(RADIO_OUT_PIN);

  //Configure elevationPID limits
  elevationPID.setOutputLimits(-175,175);
  elevationPID.setMaxIOutput(20);

  //Set Motors to not move on startup and attach servos to pins
  elevationServo.attach(ELEVATION_MOTOR_PIN);
  elevationServo.writeMicroseconds(NEUTRAL_OUTPUT);
  cylinderServo.attach(CYLINDER_MOTOR_PIN);
  cylinderServo.writeMicroseconds(NEUTRAL_OUTPUT);

  //Use the encoderHome subfile to read the absolute encoder
  encoderHome();
  //Read the encoder from the subfile
  currentAngle = getEncoderPosition();
  //set the encoder to the previously read position
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

  //Read our position from Encoder
  currentAngle = elevationEncoder.read(); 

  //Check if radio is connected
  if (radioCannonInput.read(8)<200){
    Serial.println("No Signal");
    delay(500);
    return;
  }
  
  //Read Radio Channels
  double targetAngle = radioCannonInput.read(3);
  bool cannonTrigger = radioCannonInput.read(7) >= 1600;
  //bool sirenTrigger = radioCannonInput.read(6)>= 1600;
  ManualReloadSwitch manualReloadSwitch = DONE;

  //Turn siren on and off 
  //if(sirenTrigger){
  //digitalWrite(SIREN_PIN,SIREN_ON);
  //}
  //else{
  //digitalWrite(SIREN_PIN,SIREN_OFF);
  //}
  
  
  //only move to unlocked if switch is in the middle position
  if (radioCannonInput.read(8) >= 1600){
    manualReloadSwitch = UNLOCKED;
  }
  else if (radioCannonInput.read(8)<=1300){
    manualReloadSwitch = UNUSED;
  }
  
  //Map/Lerp the Radio Signal to Angles
  if(targetAngle<10)targetAngle=10; //TODO adjust for temp hard stops
  targetAngle = map(targetAngle,1000,2000,ELEVATION_MIN_DEGREES,ELEVATION_MAX_DEGREES);
  
  
  //Check if the positions  on the controller and the hardware are similar
  if (targetAngle >= currentAngle - 10 && targetAngle <= currentAngle + 10){
    elevationAligned = true;
  }
  if (elevationAligned == false){
    //Serial.println("Move the elevation to align with hardware");
    Serial.println("Incorrect Alignment");
    delay(500);
    Serial.println(currentAngle);
    return;
  }
  
  //Map/Lerp value from Encoder to Angles
  currentAngle = map(currentAngle,0,ELEVATION_ENCODER_RANGE,ELEVATION_MAX_DEGREES,ELEVATION_MIN_DEGREES);
  
  //Write to elevationServo using PID and the current and target angles
  double angleMotorOutput=NEUTRAL_OUTPUT+elevationPID.getOutput(currentAngle,targetAngle);
  elevationServo.writeMicroseconds(angleMotorOutput);
  
  //Run State Machine
  run_state_machine(cannonTrigger,manualReloadSwitch);
  
  delay(10);
}

//States in the state machine
enum State{
  STARTUP,
  PRESSURIZING,
  IDLE,
  PLATE_RELEASE_UNLOCKED,
  PLATE_RELEASE_LOCKED_RELATCHING,//TODO IMPLEMENT, MOTOR ON
  PLATE_RELEASE_LOCKED,
  FIRING,
  RECOVERY,
  RELOAD_UNLOCKED,
  RELOAD_LOCKED,
  RESET
};

enum State state = STARTUP;
enum State last_state=RESET;

void run_state_machine(bool cannonTrigger, ManualReloadSwitch manualReloadSwitch){
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
      
      state=IDLE;
    break;
    case PRESSURIZING:
//      Serial.println("PRESSURIZING");

      //index locked
      digitalWrite(CYLINDER_LOCK_PIN,INDEX_LOCKED);
      //revolver motor off  
      cylinderServo.writeMicroseconds(NEUTRAL_OUTPUT);
      //firing plate closed
      digitalWrite(FIRING_PLATE_PIN,FIRING_PLATE_CLOSED);
      
      //Not Firing
      digitalWrite(FIRING_PIN_1,FIRING_PIN_1_CLOSED);
      digitalWrite(FIRING_PIN_2,FIRING_PIN_2_OPEN);

      //If not 
      if (cannonTrigger == false && stateMachineTimer>1000){
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
      else if(manualReloadSwitch == UNLOCKED){
        state=PLATE_RELEASE_UNLOCKED;
      }
    break;
    case PLATE_RELEASE_UNLOCKED:
      //index unlocked
      digitalWrite(CYLINDER_LOCK_PIN, INDEX_UNLOCKED);
      //Revolver motor off
      cylinderServo.writeMicroseconds(NEUTRAL_OUTPUT);
      //firing plate opened
      digitalWrite(FIRING_PLATE_PIN, FIRING_PLATE_OPEN);
      //dump valve closed 
      digitalWrite(FIRING_PIN_1, FIRING_PIN_1_CLOSED);
      digitalWrite(FIRING_PIN_2, FIRING_PIN_2_OPEN);
      
      if (manualReloadSwitch == DONE){
        state = PLATE_RELEASE_LOCKED_RELATCHING;
      }
    break;
    case PLATE_RELEASE_LOCKED_RELATCHING:
      //index locked 
      digitalWrite(CYLINDER_LOCK_PIN, INDEX_UNLOCKED);
      //Revolver motor on
      cylinderServo.writeMicroseconds(NEUTRAL_OUTPUT+FORWARD_OUTPUT);
      //firing plate opened
      digitalWrite(FIRING_PLATE_PIN, FIRING_PLATE_OPEN);
      //dump valve closed 
      digitalWrite(FIRING_PIN_1, FIRING_PIN_1_CLOSED);
      digitalWrite(FIRING_PIN_2, FIRING_PIN_2_OPEN);
      if (stateMachineTimer >400){
        state=PLATE_RELEASE_LOCKED;
      }
    break;
    case PLATE_RELEASE_LOCKED:
      // Serial.println("PLATE_RELEASE_LOCKED");

      //index locked
      digitalWrite(CYLINDER_LOCK_PIN, INDEX_LOCKED);
      //Revolver motor off
      cylinderServo.writeMicroseconds(NEUTRAL_OUTPUT+FORWARD_OUTPUT);
      //firing plate opened
      digitalWrite(FIRING_PLATE_PIN, FIRING_PLATE_OPEN);
      //dump valve closed 
      digitalWrite(FIRING_PIN_1, FIRING_PIN_1_CLOSED);
      digitalWrite(FIRING_PIN_2, FIRING_PIN_2_OPEN);
      
      //if trigger is not pressed and pressure release is moved to middle off return to idle
      if (stateMachineTimer>1000){
        state = IDLE;
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
      // if the stateMachineTimer expires advance to RECOVERY
      if (stateMachineTimer > 1000){
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
      if (stateMachineTimer > 1000){
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
      if (stateMachineTimer > 400){
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
      //Serial.println(stateMachineTimer);
      if(stateMachineTimer>1000){
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
      case PLATE_RELEASE_UNLOCKED : Serial.println("Dump Pressure Unlocked");break;;
      case PLATE_RELEASE_LOCKED_RELATCHING : Serial.println("Dump Pressure Locked Relatching");break;;
      case PLATE_RELEASE_LOCKED : Serial.println("Dump Pressure Locked");break;;
      case FIRING : Serial.println("Firing");break;;
      case RECOVERY : Serial.println("Recovery");break;;
      case RELOAD_UNLOCKED : Serial.println("Reload_unlocked");break;;
      case RELOAD_LOCKED : Serial.println("Reload_locked");break;;
      case RESET : Serial.println("reset");break;;
    }
    stateMachineTimer=0;
    last_state=state;
  }
}
