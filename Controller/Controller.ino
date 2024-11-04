
#include <MarketingBotDataPackets.h>
#include <elapsedMillis.h>
#include <Encoder.h>
#include <SPI.h>
#include <RH_RF95.h>
#include <Scheduler.h>
#include <Adafruit_SleepyDog.h>

#include "Buttons.h"
// #include "Pins.h"
// #include "Bot.h"
#include "Chassis.h"


#if defined(ARDUINO_SAMD_ZERO) && defined(SERIAL_PORT_USBVIRTUAL)
// Required for Serial on Zero based boards
#define Serial SERIAL_PORT_USBVIRTUAL
#endif

#define RF95_FREQ 915.0
#define RFM95_CS 8
#define RFM95_INT 3
#define RFM95_RST 4
RH_RF95 rf95(RFM95_CS, RFM95_INT);

union ChassisControlData {
  ChassisControl data;
  uint8_t buffer[CHASSIS_CONTROL_SIZE_BYTES];
} chassisControlData;

union ChassisTelemetryData {
  ChassisTelemetry data;
  uint8_t buffer[CHASSIS_TELEMETRY_SIZE_BYTES];
} chassisTelemetryData;

union RadioBuffer{
  ChassisControl chassisControl;
  ChassisTelemetry chassisTelemetry;
  uint8_t buffer[128]; //Set to max size bitsize of the radio to avoid potential overflow
} radioBuffer;

struct SleepTime {
  int chassisControl;
  int chassisTelemetry;
};
SleepTime sleepTime = { 100, 100 };
SleepTime sleepTimeIdle = { .chassisControl = 200, .chassisTelemetry = 20 };
SleepTime sleepTimeEnable = { .chassisControl = 30, .chassisTelemetry = 100 };

void setup() {
  // put your setup code here, to run once:

  while (!Serial && millis() < 2000) {
    delay(10);
  }
  Serial.println("============");
  Serial.println("==New Boot==");
  Serial.println("============");

  //Do radio bringup
  pinMode(RFM95_RST, OUTPUT);
  digitalWrite(RFM95_RST, LOW);
  delay(100);
  digitalWrite(RFM95_RST, HIGH);
  delay(100);

  if (!rf95.init()) {
    Serial.println("init failed");
  } else {
    Serial.println("Radio online...");
  }

  rf95.setModemConfig(RH_RF95::Bw500Cr45Sf128);  // 500hz data rate; Default is 128
  rf95.setFrequency(915.0);
  rf95.setTxPower(23, false);

  //Let the button initialization do it's thing
  Buttons::init();

  // Core operating tasks
  Scheduler.startLoop(send_control);
  Scheduler.startLoop(recieve_telemetry);
  Scheduler.startLoop(updateControls);

  //Informational tasks
  // Scheduler.startLoop(Buttons::printDebug);
  // Scheduler.startLoop(print_status);
  // Scheduler.startLoop(printChassisControl);
  // Scheduler.startLoop(printChassisTelemetry);

  Scheduler.startLoop(debugControlPackets);

  // Scheduler.startLoop(a);
  // Scheduler.startLoop(b);

}

//For temp code and the like
void loop() {
  delay(1000);
}

/** Monitor hardware + do the things. */
void updateControls(){
  if(Buttons::home2() >2000 && chassisControlData.data.enable==false){
    chassisControlData.data.enable=true;
  }
  else if(Buttons::home2() >2000 && chassisControlData.data.enable==true){
    //wait to release button
  }
  else if(Buttons::home2() && chassisControlData.data.enable==true){
    chassisControlData.data.enable=false;
  }
  //handle forgotten controller
  if(Buttons::idleTimer() > 30*1000){
    //Handle a controller being forgotten
    chassisControlData.data.enable = false;
  }

  //Auto power off the controller
  // TODO: This sleeps, but doesn't seem to wake properly on buttons. Leave off for now.
  // if(Buttons::idleTimer() > 2*60*1000 || Buttons::home2() >5000 ){
  //   while(Buttons::home2()); //wait til home button is released
  //   //do a low power mode
  //   unsigned long int sleepms=0;
  //   Serial.println("\n zzz....");
  //   rf95.sleep();
  //   while(Buttons::home2()==false){
  //     sleepms += Watchdog.sleep(1000);
  //     delay(2);
  //   }
  //   while(Buttons::home2()); //wait til home button is released
  //   Serial.printf("...yawn %s\n",sleepms);
  // }



  //CANNOT USE: Some idiot mentor wired this pin to the wheel encoder, so there's a pin conflict
  // analogWrite(LED_BUILTIN, enable ? 255:0);


  Buttons::JoystickReadings stick = Buttons::LeftStick();
  stick.x*=-1; //Convert x to the right to CCW notation for arcade drive
  stick.y*=-1; //Covert y from HID standard of negative-is-away to positive robot forward
  chassisControlData.data.speed = Chassis::arcadeDrive( stick.y, stick.x*.25 );

  delay(10);
}




elapsedMillis cctimer;
void send_control() {
  //Observe the need of RX mode to reserve the radio;
  //In such cases, just hang out until it passes control back
  while (rf95.mode() == RHGenericDriver::RHModeRx) delay(1);
  
  chassisControlData.data.metadata.type = PacketType::CHASSIS_CONTROL;
  chassisControlData.data.metadata.heartbeat+=1;

  bool sent = false;
  sent = rf95.send(chassisControlData.buffer, CHASSIS_CONTROL_SIZE_BYTES);
  bool done = rf95.waitPacketSent(200);
  // Serial.println();
  // Serial.print("#");
  // Serial.print(sent ? ">>" : "--");
  // Serial.print(done ? "++" : "--");
  cctimer = 0;

  delay(sleepTime.chassisControl);
}


elapsedMillis cttimer;
void recieve_telemetry() {
  //Set by radio when recieving a buffer
  //Note, this can be initailized to anything *but* zero; Which represents a closed socket. 
  uint8_t radioBufferLength=CHASSIS_TELEMETRY_SIZE_BYTES;
  
  if (rf95.waitAvailableTimeout(sleepTime.chassisTelemetry)) {
    if (rf95.recv(radioBuffer.buffer, &radioBufferLength)) {
      if (
        radioBufferLength == CHASSIS_TELEMETRY_SIZE_BYTES && 
        radioBuffer.chassisTelemetry.metadata.type == PacketType::CHASSIS_TELEMETRY
      ) {
        //valid data; Handle it appropriately
        chassisTelemetryData.data = radioBuffer.chassisTelemetry;
        cttimer = 0;
      }
    }
  } else {
    // Serial.print(".");
  }
  rf95.setModeIdle();

  delay(sleepTime.chassisTelemetry);
}

void print_status() {
  Serial.println();
  Serial.print("<3 ");
  Serial.print(rf95.lastRssi());

  delay(2000);
}


void printChassisControl(){
  Serial.println();

  Serial.print("> Chassis ");

  Serial.printf(
    "%s",chassisControlData.data.enable?"EN":".."
  );

  Serial.printf(
    "[L%4i R%4i]",
    chassisControlData.data.speed.left,
    chassisControlData.data.speed.right
  );

  Serial.printf(
    "[%s]",
    chassisControlData.data.gear==ChassisGear::High?"HG":"LG"
  );
  Serial.printf(
    "[<3 %i]",
    chassisControlData.data.metadata.heartbeat
  );

  delay(200);
}

void printChassisTelemetry(){
  Serial.println();

  Serial.print(" <Chassis ");

  Serial.printf(
    "%s",chassisTelemetryData.data.enable?"EN":".."
  );

  Serial.printf(
    "[L%4i R%4i]",
    chassisTelemetryData.data.speed.left,
    chassisTelemetryData.data.speed.right
  );

  Serial.printf(
    "[%s]",
    chassisTelemetryData.data.gear==ChassisGear::High?"HG":"LG"
  );

  // Serial.printf(
  //   "[%i psi]",
  //   chassisTelemetryData.data.pressure
  // );
  // Serial.printf(
  //   "[%1f v]",
  //   chassisTelemetryData.data.batteryVoltage/10.0
  // );

  delay(200);
}

/** Print some very detailed information about the Control packets
* May be necessary to troubleshoot communication related issues
*/
void debugControlPackets(){
  Serial.print("D ");

  Serial.printf("<%2i>",chassisControlData.data.metadata.heartbeat);

  Serial.printf(" %2s ",chassisControlData.data.enable?"EN":"--");

  //Print a bitfield of the data as it would be sent
  //Useful to troubleshoot length and offset issues
  Serial.print(" ");
  for(int i = 0; i < sizeof(chassisControlData); i++){
    for(int j = 0; j < 8; j++){
      Serial.print((chassisControlData.buffer[i]>>(7-j)) &1);
    }
    Serial.print(".");
  }


  Serial.println();
  delay(200);
}

