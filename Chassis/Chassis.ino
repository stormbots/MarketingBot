// rf95_client.pde
// -*- mode: C++ -*-
// Example sketch showing how to create a simple messageing client
// with the RH_RF95 class. RH_RF95 class does not provide for addressing or
// reliability, so you should only use RH_RF95 if you do not need the higher
// level messaging abilities.
// It is designed to work with the other example rf95_server
// Tested with Anarduino MiniWirelessLoRa, Rocket Scream Mini Ultra Pro with
// the RFM95W, Adafruit Feather M0 with RFM95

#include <SPI.h>
#include <RH_RF95.h>
#include <DataPackets.h>
#include <elapsedMillis.h>
#include "Pins.h"
#include "Bot.h"

//M0 uses SerialUSB for the output, so rebind it
#if defined(ARDUINO_SAMD_ZERO) && defined(SERIAL_PORT_USBVIRTUAL)
// Required for Serial on Zero based boards
#define Serial SERIAL_PORT_USBVIRTUAL
#endif


elapsedMillis heartbeatTimer;
void heartbeat(){
  digitalWrite(LED_BUILTIN, heartbeatTimer >200);
  //Reset it
  if(heartbeatTimer>500) heartbeatTimer=0;
}

// Singleton instance of the radio driver
RH_RF95 rf95(8, 3);  // Adafruit Feather M0 with RFM95

// Need this on Arduino Zero with SerialUSB port (eg RocketScream Mini Ultra Pro)
//#define Serial SerialUSB


void setup() {
  pinMode(LED_BUILTIN, OUTPUT);

  Serial.begin(9600);
  while(!Serial){
    //Wait for a moment to see if we're trying to connect a USB wire
    if(elapsedMillis() > 1000) return;
  }
  Serial.println("Rebooting chassis....");

  if (!rf95.init()){
    Serial.println("init failed");
  }
  // Unless overwritten, library defaults are 434.0MHz, 13dBm, Bw = 125 kHz, Cr = 4/5, Sf = 128chips/symbol, CRC on
  // Configure RFM95, operating at 915mhz
  rf95.setFrequency(915.0);
  rf95.setTxPower(23, false);

  bot::shift(ChassisGear::Low);
  // bot::tankDrive((ChassisSpeed){0,0});
}


union ChassisControlData{
  ChassisControl data;
  uint8_t buffer[4*8];
} chassisControlData;
uint8_t ChassisControlDataLen=4;
ChassisControlData radioBuffer;

union ChassisTelemetryData{
  ChassisTelemetry data;
  uint8_t buffer[5*8];
} chassisTelemetryData;
uint8_t chassisTelemetryDataLen=5;


boolean enable= false;
elapsedMillis enableWatchdog;

elapsedMillis looptime;

void loop() {
  // Now wait for a reply
  if (rf95.waitAvailableTimeout(10)) {
    // Should be a reply message for us now
    if (rf95.recv(radioBuffer.buffer, &ChassisControlDataLen)) {
      //Have data; Make sure it's what we want to actually listen for
      //Note: This only works because we specifically ensure all transmitted packets 
      // include this packet type up front.
      if(radioBuffer.data.metadata.type==PacketType::CHASSIS_CONTROL){
        //valid data; Handle it appropriately
        chassisControlData.data = radioBuffer.data;
        enable = chassisControlData.data.enable;
        enableWatchdog = 0; //feed the doggo
        digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));

      //debug printing
      // Serial.print(rf95.lastRssi(), DEC);
      // Serial.print(",");
      // Serial.print(chassisControlData.data.metadata.type);
      // Serial.print(",");
      // Serial.print(chassisControlData.data.metadata.heartbeat);
      // Serial.print(",");
      // Serial.print(chassisControlData.data.speed.left);
      // Serial.print(",");
      // Serial.print(chassisControlData.data.speed.right);
      // Serial.print(",");
      // Serial.print(chassisControlData.data.gear);
      // Serial.print(",");
      // Serial.print(chassisControlData.data.enable);
      // Serial.println();

      }
    } else {
      Serial.println("recv failed");
    }
  } else {
    Serial.println("No reply, is rf95_server running?");
  }

  // Disable the bot if we lose track of the controller
  if(enableWatchdog > 100){
    enable = false;
    bot::shift(ChassisGear::Low);
  }


  if(enable){
    bot::tankDrive(chassisControlData.data.speed);
    bot::shift(chassisControlData.data.gear);    
  }

  //Reply with telemetry
  do{
    chassisTelemetryData.data.speed = bot::currentSpeed();
    chassisTelemetryData.data.batteryVoltage = bot::batteryVoltage()*10; //make sure to use deciVolts
    chassisTelemetryData.data.gear = chassisControlData.data.gear;
    chassisTelemetryData.data.enable = enable;
    chassisTelemetryData.data.pressure = bot::currentPressure(); //TODO: 

    rf95.send(chassisTelemetryData.buffer, sizeof(chassisTelemetryData.buffer));
    rf95.waitPacketSent();
  }while(false);

  //try to run at a consistent loop time
  int t=20-looptime;
  delay(t>0?t:0);
}
