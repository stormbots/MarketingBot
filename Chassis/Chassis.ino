#include <MarketingBotDataPackets.h>
#include <elapsedMillis.h>
#include <Encoder.h>
#include <SPI.h>
#include <RH_RF95.h>
#include <Scheduler.h>
#include "Pins.h"
#include "Bot.h"


#if defined(ARDUINO_SAMD_ZERO) && defined(SERIAL_PORT_USBVIRTUAL)
// Required for Serial on Zero based boards
#define Serial SERIAL_PORT_USBVIRTUAL
#endif

#define RF95_FREQ 915.0
#define RFM95_CS    8
#define RFM95_INT   3
#define RFM95_RST   4
RH_RF95 rf95(RFM95_CS, RFM95_INT);

union ChassisControlData{
  ChassisControl data;
  uint8_t buffer[CHASSIS_CONTROL_SIZE_BYTES];
} chassisControlData;

union RadioBuffer{
  ChassisControl ccd;
  uint8_t buffer[128]; //Set to max size bitsize of the radio to avoid potential overflow
} radioBuffer;

union ChassisTelemetryData{
  ChassisTelemetry data;
  uint8_t buffer[CHASSIS_TELEMETRY_SIZE_BYTES];
} chassisTelemetryData;

elapsedMillis watchdog;

void setup() {
  // put your setup code here, to run once:
  
  Serial.begin(19200);
  while(!Serial && millis()<2000){
    delay(10);
  }
  Serial.println("============");
  Serial.println("==New Boot==");
  Serial.println("============");

  //Do various hardware tweaks
  bot::init();

  pinMode(LED_BUILTIN,OUTPUT);

  pinMode(RFM95_RST, OUTPUT);
  digitalWrite(RFM95_RST, LOW);
  delay(100);
  digitalWrite(RFM95_RST, HIGH);
  delay(100);

  if (!rf95.init()){
    Serial.println("init failed");
  } else {
    Serial.println("Radio online...");
  }

  rf95.setModemConfig(RH_RF95::Bw500Cr45Sf128);  // 500hz data rate; Default is 128
  rf95.setFrequency(915.0);
  rf95.setTxPower(23, false);

  //Core operating tasks
  Scheduler.startLoop(send_telemetry);
  Scheduler.startLoop(recieve_input);

  //Informative tasks for debugging
  Scheduler.startLoop(print_status);
  // Scheduler.startLoop(printControl);
  // Scheduler.startLoop(printTelemetry);
}

bool isEnabled=false;
String enableText="EN-> Never enabled"; //big long string to preset the buffer
void loop() {
  bool enableAllowed=true; //Assume we're good to go

  //Check unsafe conditions
  if(isEnabled && watchdog>400){
    enableText="EN -> Timed out";
    enableAllowed=false;
  }
  else if(isEnabled && bot::batteryVoltage()<9){
    enableText="EN -> Low Battery";
    enableAllowed=false;
  }
  else if(isEnabled && chassisControlData.data.enable==false){
    enableText="EN -> Remote disable";
    enableAllowed=false;
  }
  else if(!isEnabled && enableAllowed && chassisControlData.data.enable){
    //All good! Clear our text
    bot::enable();
    enableText="";
  }
  //We can keep enable if both are allowed
  isEnabled = chassisControlData.data.enable && enableAllowed;

  //Write the robot state to telemetry
  chassisTelemetryData.data.metadata.type = PacketType::CHASSIS_TELEMETRY;

  chassisTelemetryData.data.batteryVoltage = bot::batteryVoltage()*10; // TODO: Battery sensing not currently implimented
  chassisTelemetryData.data.gear = ChassisGear::Low; // TODO: Shifter solenoid not operational in hardware
  chassisTelemetryData.data.enable = isEnabled;
  chassisTelemetryData.data.pressure = bot::currentPressure(); //TODO: Hardware detection not installed
  chassisTelemetryData.data.speed = bot::currentSpeed();

  //TODO: Left side encoder not actually hooked up inside of the gearbox, and does not spin
  //This means for now, we cannot use certain nice to have features.


  //If we're disabled, stop actuators and exit loop
  if(isEnabled){
    //Pass provided Control data to the hardware
    // int speed = constrain(millis()/10,1500,2000);
    // bot::tankDriveRaw(speed,speed);
    // bot::tankDrive(ChassisSpeeds speeds)

    bot::tankDrive(chassisControlData.data.speed);
    bot::shift(chassisControlData.data.gear);
  }else{
    bot::tankDrive((ChassisSpeeds){0,0});
    bot::shift(ChassisGear::Low);
    bot::disable();
  }

  digitalWrite(LED_BUILTIN,isEnabled?255:0);

  //wait til next loop
  delay(5);
}



void send_telemetry(){
  // Serial.println();
  // Serial.print("#");
  bool sent=false;

  chassisTelemetryData.data.metadata.heartbeat += 1;
  sent=rf95.send(chassisTelemetryData.buffer, CHASSIS_TELEMETRY_SIZE_BYTES);
  // Serial.print(sent ? ">>" : "--" );
  bool done = rf95.waitPacketSent(200);
  // Serial.print(done ? "++" : "--" );

  // prevent timing hiccups with switching radio tx/rx modes
  // when robot is operating until better solution located
  delay(isEnabled? 2000 : 200);
}



void recieve_input(){
  uint8_t radiobufferlen=CHASSIS_CONTROL_SIZE_BYTES;

  if (rf95.available()) {
    // Serial.println();
    // Serial.print("!");
    if (rf95.recv(radioBuffer.buffer, &radiobufferlen)) {
      
      if(
        radiobufferlen==CHASSIS_CONTROL_SIZE_BYTES &&
        radioBuffer.ccd.metadata.type==PacketType::CHASSIS_CONTROL
      ){
        //valid data; Handle it appropriately
        chassisControlData.data = radioBuffer.ccd;
        // Serial.printf("\n[OK %2i%s] ",
        //   chassisControlData.data.metadata.heartbeat,
        //   chassisControlData.data.enable?"+":"-"
        //   );


        //pet the watchdog to keep the system alive
        watchdog=0;
      }
      else if(
        radiobufferlen==CANNON_CONTROL_SIZE_BYTES &&
        radioBuffer.ccd.metadata.type==PacketType::CANNON_CONTROL
      ){
        //Catch to avoid printing stuff
      }
      else{
        //Some other packet type
        //Print out info about it
        Serial.printf("\n?(%i) ",radiobufferlen);
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
    // Serial.print(".");
  }

  //We want this to go as fast as possible; Effectively our default task
  delay(1);
}

void print_status(){
  Serial.println();
  Serial.print(watchdog<300?":D":":(");
  delay(200);
}


void printControl(){
  Serial.println();

  Serial.print("> Chassis ");

  Serial.printf(
    "%2s ",chassisControlData.data.enable?"EN":"--"
  );

  Serial.printf(
    "[L%4i R%4i] ",
    chassisControlData.data.speed.left,
    chassisControlData.data.speed.right
  );

  Serial.printf(
    "[%2s] ",
    chassisControlData.data.gear==ChassisGear::High?"HG":"LG"
  );

  Serial.printf(
    "(%2i) ",
    chassisControlData.data.metadata.heartbeat
  );

  delay(200);
}


void printTelemetry(){
  Serial.println();

  Serial.print(" <Chassis ");

  Serial.printf(
    "%2s",
    chassisTelemetryData.data.enable?"EN":"--"
  );

  Serial.printf(
    "[L%4i R%4i] ",
    chassisTelemetryData.data.speed.left,
    chassisTelemetryData.data.speed.right
  );

  Serial.printf(
    "[%s]",
    chassisTelemetryData.data.gear==ChassisGear::High?"HG":"LG"
  );

  Serial.printf(
    "(%2i)",
    chassisTelemetryData.data.metadata.heartbeat
  );


  Serial.printf(
    "[%2i psi] ",
    chassisTelemetryData.data.pressure
  );
  Serial.printf(
    "[%2.1fv]",
    chassisTelemetryData.data.batteryVoltage/10.0
  );

  //Non-telemetry but useful 
  Serial.print(" "); 
  Serial.printf(
    "enc[L%4i R%4i] ",
    bot::encLeft.read(),
    bot::encRight.read()
  );

  //Non-telemetry but useful 
  Serial.print(" "); 
  Serial.printf(
    "max[L%4i R%4i] ",
    bot::maxleft,bot::maxright
  );

  Serial.printf(
    "in[L%4i R%4i] ",
    bot::encLeft.read()/bot::configLow.encoderRatio,
    bot::encRight.read()/bot::configLow.encoderRatio
  );

  Serial.print(enableText);
  Serial.print(" ");

  delay(200);
}


