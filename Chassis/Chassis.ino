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
// RH_RF95 rf95;
//RH_RF95 rf95(5, 2); // Rocket Scream Mini Ultra Pro with the RFM95W
RH_RF95 rf95(8, 3);  // Adafruit Feather M0 with RFM95

// Need this on Arduino Zero with SerialUSB port (eg RocketScream Mini Ultra Pro)
//#define Serial SerialUSB


void setup() {
  pinMode(LED_BUILTIN, OUTPUT);

  Serial.begin(9600);
  while(!Serial){
    if(elapsedMillis() > 1000) return;
  }
  Serial.println("Rebooting chassis....");

  if (!rf95.init())
    Serial.println("init failed");
  // Defaults after init are 434.0MHz, 13dBm, Bw = 125 kHz, Cr = 4/5, Sf = 128chips/symbol, CRC on

  // You can change the modulation parameters with eg
  // rf95.setModemConfig(RH_RF95::Bw500Cr45Sf128);

  // The default transmitter power is 13dBm, using PA_BOOST.
  // If you are using RFM95/96/97/98 modules which uses the PA_BOOST transmitter pin, then
  // you can set transmitter powers from 2 to 20 dBm:
  rf95.setFrequency(915.0);
  rf95.setTxPower(23, false);
}



union ChassisControlData{
  ChassisControl data;
  uint8_t buffer[4*8];
} chassisControlData;
uint8_t ChassisControlDataLen=4;


union ChassisTelemetryData{
  ChassisTelemetry data;
  uint8_t buffer[5*8];
} chassisTelemetryData;
uint8_t chassisTelemetryDataLen=5;

//temporary namespace for testing; Move actual encoder stuff somewhere else
namespace encoders{
  int encLeft=0;
  int encRight=0;
  int pressure=0;
  float pressuredelta=0;
}

boolean enable= false;
elapsedMillis enableWatchdog;

elapsedMillis dt;

void loop() {

  // uint8_t data[] = "Hello World!";
  // rf95.send(data, sizeof(data));
  // rf95.waitPacketSent();


  // Now wait for a reply
  if (rf95.waitAvailableTimeout(3000)) {
    // Should be a reply message for us now
    if (rf95.recv(chassisControlData.buffer, &ChassisControlDataLen)) {
      
      // Serial.print("got reply: ");
      // Serial.println((char*)chassisControlData.buffer);
      //      Serial.print("RSSI: ");
      Serial.print(rf95.lastRssi(), DEC);
      Serial.print(",");
      Serial.print(chassisControlData.data.metadata.type);
      Serial.print(",");
      Serial.print(chassisControlData.data.metadata.heartbeat);
      Serial.print(",");
      Serial.print(chassisControlData.data.speed.left);
      Serial.print(",");
      Serial.print(chassisControlData.data.speed.right);
      Serial.print(",");
      Serial.print(chassisControlData.data.gear);
      Serial.print(",");
      Serial.print(chassisControlData.data.enable);
      Serial.println();


      enable = chassisControlData.data.enable;
      enableWatchdog = 0; //feed the doggo
      digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
    } else {
      Serial.println("recv failed");
    }
  } else {
    Serial.println("No reply, is rf95_server running?");
    digitalWrite(LED_BUILTIN, HIGH);
    delay(100);
    digitalWrite(LED_BUILTIN, LOW);
    delay(400);

  }

  // Disable the bot if we lose track of the controller
  if(enableWatchdog > 250){ enable = false; }

  //mock some hardware to make telemetry interesting
  int maxdv = 10;
  encoders::encRight = constrain(chassisControlData.data.speed.right, encoders::encRight-maxdv, encoders::encRight+maxdv );
  encoders::encLeft = constrain(chassisControlData.data.speed.left, encoders::encLeft-maxdv, encoders::encLeft+maxdv );
  if(encoders::pressure>=90){encoders::pressuredelta=-0.5;}
  if(encoders::pressure<60){encoders::pressuredelta=5;}
  encoders::pressure += encoders::pressuredelta*dt/1000;
  dt=0;

  //Shove data into telemetry
  chassisTelemetryData.data.speed.left = encoders::encLeft;
  chassisTelemetryData.data.speed.right = encoders::encRight;
  chassisTelemetryData.data.batteryVoltage = analogRead(A7) //configured for lipo
    *2 //double reading due to voltage divider
    *3.3 //multiply by reference voltage
    *10 // convert from volt to decivolt
    /1024 // Divide by ADC steps to get voltage
    ; 
  chassisTelemetryData.data.gear = chassisControlData.data.gear;
  chassisTelemetryData.data.enable;
  chassisTelemetryData.data.pressure = encoders::pressure;

  rf95.send(chassisTelemetryData.buffer, sizeof(chassisTelemetryData.buffer));
  rf95.waitPacketSent();

  delay(20);
}
