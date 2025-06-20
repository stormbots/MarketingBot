//Libraries 
//PID, used for precise motor control on the elevation
#include "MiniPID.h"
//Timer
#include <elapsedMillis.h>
#include <Scheduler.h>
//Radio library
// #include <PulsePosition.h>
//Encoder library
#include <Encoder.h>
//Used to control all of the motors on the bot
#include <Servo.h>
#include <RH_RF95.h>
//Subfiles
#include "PinNames.h"
#include "./RotationEncoder.h"
#include "./ElevationEncoder.h"
#include <MarketingBotDataPackets.h>


//-----------------------------------//
//All pin declarations are in the PinNames subfile
//-----------------------------------//

//Limits for the elevation motor in degrees measured from horizon
#define ELEVATION_MAX_DEGREES 40
#define ELEVATION_MIN_DEGREES 5


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

#define TICKS_PER_BARREL ((4096*30)/24)

//Constants for motors
//The range for motors is from 1000 to 2000 with 1000 being 100% backwards and 2000 being 100% forwards. Thus, 1500 is neutral or no movement.
#define NEUTRAL_OUTPUT 1500
//Increase Forward Output to Increase Motor Speeds, forward output is added to neutral output to produce movement
#define FORWARD_OUTPUT 110

//Radios
#define RF95_FREQ 915.0
#define RFM95_CS    8
#define RFM95_INT   3
#define RFM95_RST   4
RH_RF95 rf95(RFM95_CS, RFM95_INT);

//PID for elevationServo
MiniPID elevationPID = MiniPID(10,0.0,0.0);
Encoder barrelEncoder(ENCODER_2_A_PIN, ENCODER_2_B_PIN);

union CannonControlData{
  CannonControl data;
  uint8_t buffer[CANNON_CONTROL_SIZE_BYTES];
} cannonControlData;

union RadioBuffer{
  CannonControl ccd;
  uint8_t buffer[128]; //Set to max size bitsize of the radio to avoid potential overflow
} radioBuffer;

union CannonTelemetryData{
  CannonTelemetry data;
  uint8_t buffer[CANNON_TELEMETRY_SIZE_BYTES];
} cannonTelemetryData;

elapsedMillis watchdog;

/*************************
Existing random variables that need better placement
*************************/

//Current Position of the elevation in degrees
float currentElevation = 0.0;
float currentRotation = 0.0;

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

bool isEnabled=false;

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
  
  //Configure elevationPID limits
  elevationPID.setOutputLimits(-175,175);
  elevationPID.setMaxIOutput(20);

  //Set Motors to not move on startup and attach servos to pins
  elevationServo.attach(ELEVATION_MOTOR_PIN);
  elevationServo.writeMicroseconds(NEUTRAL_OUTPUT);
  cylinderServo.attach(CYLINDER_MOTOR_PIN);
  cylinderServo.writeMicroseconds(NEUTRAL_OUTPUT);

  //Use the encoderHome subfile to read the absolute encoder
  ConfigElevationPWM(ABSOLUTE_ENCODER_PIN);
  // ConfigRotationPWM(BARREL_ABSOLUTE_ENCODER_PIN);
  
  //Wait to ensure everything gets time to respond
  delay(30); 
}


void loop(){
  //Heartbeat LED
  if (cannonHeartbeat > 1000){
    cannonHeartbeat=0;
    Serial.print("cannon ");
    Serial.print(cannonControlData.data.metadata.heartbeat);
    Serial.println("");

    digitalWrite(BUILT_IN_LED, !digitalRead(BUILT_IN_LED) || cannonControlData.data.enable );
  }

  //Read the encoder from the subfile
  currentElevation = ReadElevationDegrees();
  currentRotation= ReadRotationPWM();
  // Serial.println(currentElevation);
  // Serial.println(currentRotation);
  
  //Read Radio Channels
  double targetElevation =  0;//radioCannonInput.read(3);
  bool cannonTrigger = false; //radioCannonInput.read(7) >= 1600;
  //bool sirenTrigger = radioCannonInput.read(6)>= 1600;
  ManualReloadSwitch manualReloadSwitch = DONE;

  //Turn siren on and off 
  //if(sirenTrigger){
  //digitalWrite(SIREN_PIN,SIREN_ON);
  //}
  //else{
  //digitalWrite(SIREN_PIN,SIREN_OFF);
  //}
  
    
  //Map/Lerp the Radio Signal to Angles
  if(targetElevation<10)targetElevation=10; //TODO adjust for temp hard stops
  targetElevation = map(targetElevation,1000,2000,ELEVATION_MIN_DEGREES,ELEVATION_MAX_DEGREES);
  
  
  //Check if the positions  on the controller and the hardware are similar
  if (targetElevation >= currentElevation - 10 && targetElevation <= currentElevation + 10){
    elevationAligned = true;
  }
  if (false && elevationAligned == false){
    //TODO: Reinstate this check, or otherwise resolve functionality. Temporarily disabled for testing. 

    //Serial.println("Move the elevation to align with hardware");
    Serial.println("Incorrect Alignment");
    delay(50);
    return;
  }
  
  //Write to elevationServo using PID and the current and target angles
  double elevationMotorOutput=NEUTRAL_OUTPUT+elevationPID.getOutput(currentElevation,targetElevation);
  elevationServo.writeMicroseconds(elevationMotorOutput);
  
  //Run State Machine
  // cannonTrigger = millis()%3000 > 1500;
  cannonTrigger = cannonControlData.data.fire && cannonControlData.data.enable;

  if(cannonControlData.data.load){
    manualReloadSwitch = ManualReloadSwitch::UNLOCKED;
  }else{
    manualReloadSwitch = ManualReloadSwitch::DONE;
  }
  run_state_machine(cannonTrigger,manualReloadSwitch);
  
  delay(10);
}



void send_telemetry(){
  // Serial.println();
  // Serial.print("#");
  bool sent=false;

  cannonTelemetryData.data.metadata.heartbeat += 1;
  sent=rf95.send(cannonTelemetryData.buffer, CANNON_TELEMETRY_SIZE_BYTES);
  // Serial.print(sent ? ">>" : "--" );
  bool done = rf95.waitPacketSent(200);
  // Serial.print(done ? "++" : "--" );

  // prevent timing hiccups with switching radio tx/rx modes
  // when robot is operating until better solution located
  delay(isEnabled? 2000 : 200);
}



void recieve_input(){
  uint8_t radiobufferlen=CANNON_CONTROL_SIZE_BYTES;

  if (rf95.available()) {
    // Serial.println();
    // Serial.print("!");
    if (rf95.recv(radioBuffer.buffer, &radiobufferlen)) {
      
      if(
        // radiobufferlen==CANNON_CONTROL_SIZE_BYTES &&
        radioBuffer.ccd.metadata.type==PacketType::CANNON_CONTROL
      ){
        //valid data; Handle it appropriately
        cannonControlData.data = radioBuffer.ccd;
        Serial.printf("[OK %2i%s] ",
          cannonControlData.data.metadata.heartbeat,
          cannonControlData.data.enable?"+":"-"
          );


        //pet the watchdog to keep the system alive
        watchdog=0;
      }
      else{
        //Some other packet type
        //Print out info about it
        Serial.printf("?(%i) ",radiobufferlen);
        for(int i = 0; i < radiobufferlen; i++){
          for(int j = 0; j < 8; j++){
            Serial.print((radioBuffer.buffer[i]>>(7-j)) &1);
          }
          Serial.print(".");
        }
        Serial.println();

      }
    }
  } else {
    Serial.print(".");
  }

  //We want this to go as fast as possible; Effectively our default task
  delay(1);
}



/*******************
MOVE THIS ELSEWHERE
*******************/


//States in the state machine
enum State{
  STARTUP,
  PRESSURIZING,
  IDLE,
  IDLE_READY,
  MANUAL_LOADING,
  MANUAL_RELOAD_UNLOCKED,//TODO IMPLEMENT, MOTOR ON
  MANUAL_RELOAD_LOCKED,
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

      //For safety: Always ensure users undo the fire
      // switch between state machine loops.
      // This should be the _only_ transition out of idle
      if (cannonTrigger == false){
        state = IDLE_READY;
      }
    break;
    case IDLE_READY:
      // This should be the only state with no control/device logic
      // if the trigger for the cannon is pressed move to FIRING
      if (cannonTrigger){
        state = State::FIRING;
      }
      //if the pressure release is swithed move to dump pressure
      else if(manualReloadSwitch == UNLOCKED){
        state=MANUAL_LOADING;
      }
    break;
    case MANUAL_LOADING:
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
        state = MANUAL_RELOAD_UNLOCKED;
      }
    break;
    case MANUAL_RELOAD_UNLOCKED:
      //index locked 
      digitalWrite(CYLINDER_LOCK_PIN, INDEX_UNLOCKED);
      //Revolver motor on
      cylinderServo.writeMicroseconds(NEUTRAL_OUTPUT);
      //firing plate opened
      digitalWrite(FIRING_PLATE_PIN, FIRING_PLATE_OPEN);
      //dump valve closed 
      digitalWrite(FIRING_PIN_1, FIRING_PIN_1_CLOSED);
      digitalWrite(FIRING_PIN_2, FIRING_PIN_2_OPEN);

      if (stateMachineTimer>200){
        state=MANUAL_RELOAD_LOCKED;
      }
    break;
    case MANUAL_RELOAD_LOCKED:
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
        barrelEncoder.readAndReset();
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

      
      //dump valve closed
      digitalWrite(FIRING_PIN_1,FIRING_PIN_1_CLOSED);
      digitalWrite(FIRING_PIN_2,FIRING_PIN_2_OPEN);
      // if timer expires advance to RELOAD_LOCKED
      if (barrelEncoder.read() > TICKS_PER_BARREL/6){
        state=RELOAD_LOCKED;
      }
      //TODO: Reconsider?
      if (stateMachineTimer > 2000){
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
      case IDLE_READY : Serial.println("Idle_ready");break;;
      case MANUAL_LOADING : Serial.println("Manual Loading");break;;
      case MANUAL_RELOAD_UNLOCKED : Serial.println("Manual Reload Unlocked");break;;
      case MANUAL_RELOAD_LOCKED : Serial.println("Manual Reload Locked");break;;
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
