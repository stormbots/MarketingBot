
 
#include <SPI.h>
#include <RH_RF95.h>
#include <DataPackets.h>
#include <elapsedMillis.h>


// Singleton instance of the radio driver
RH_RF95 rf95(8, 3); // Adafruit Feather M0 with RFM95 
 
// Need this on Arduino Zero with SerialUSB port (eg RocketScream Mini Ultra Pro)
#define Serial SERIAL_PORT_USBVIRTUAL
 
elapsedMillis watchdog;

void setup() 
{
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
  Serial.begin(9600);
  Serial.println("Controller booting");
  display_setup();

  // while (!Serial) ; // Wait for serial port to be available
  if (!rf95.init())
    Serial.println("init failed");  
  // Defaults after init are 434.0MHz, 13dBm, Bw = 125 kHz, Cr = 4/5, Sf = 128chips/symbol, CRC on
 
  // You can change the modulation parameters with eg
  // rf95.setModemConfig(RH_RF95::Bw500Cr45Sf128);
 
  // The default transmitter power is 13dBm, using PA_BOOST.
  rf95.setFrequency(915.0);
  rf95.setTxPower(23, false);
  digitalWrite(LED_BUILTIN, HIGH);
}

union ChassisControlData{
  ChassisControl data;
  uint8_t buffer[4*8];
} chassisControlData;

union{
  ChassisTelemetry data;
  uint8_t buffer[5*8];
} chassisTelemetry;
uint8_t chassisTelemetryLen = 5;

void loop(){
  //Send controller outputs
  heartbeat();

  //Emulate a controller
  chassisControlData.data.metadata.type = PacketType::CHASSIS_CONTROL;
  chassisControlData.data.metadata.heartbeat+=1;
  chassisControlData.data.enable=1;
  chassisControlData.data.gear= millis()%3000 >2000 ? ChassisGear::High : ChassisGear::Low;
  chassisControlData.data.speed.left = 50+millis()/50%50;
  chassisControlData.data.speed.right = 50-millis()/50%50;

  //Send our input packets
  rf95.send(chassisControlData.buffer, sizeof(chassisControlData));
  rf95.waitPacketSent();

  // listen for telemetry
  if (rf95.available()){
    // Should be a message for us now   
    //TODO: Actually check the incoming packet type
    if (rf95.recv(chassisTelemetry.buffer, &chassisTelemetryLen)) {

     Serial.print(rf95.lastRssi(), DEC);
     Serial.print(",");
     Serial.print(chassisTelemetry.data.metadata.type);
     Serial.print(",");
     Serial.print(chassisTelemetry.data.metadata.heartbeat);
     Serial.print(",");
     Serial.print(chassisTelemetry.data.batteryVoltage);
     Serial.print(",");
     Serial.print(chassisTelemetry.data.speed.left);
     Serial.print(",");
     Serial.print(chassisTelemetry.data.speed.right);
     Serial.print(",");
     Serial.print(chassisTelemetry.data.gear);
     Serial.print(",");
     Serial.print(chassisTelemetry.data.enable);
     Serial.print(",");
     Serial.print(chassisTelemetry.data.pressure);
     Serial.println();

    watchdog = 0;
    }
    else {
      Serial.println("recv failed");
    }
  }

  float controllerBatteryVoltage = analogRead(A7) //configured for lipo
    *2 //double reading due to voltage divider
    *3.3 //multiply by reference voltage
    /1024; // Divide by ADC steps


  //Do the display routines
  display_clear();
  drawControllerVoltage(controllerBatteryVoltage);
  drawSignalStrength(watchdog<500 ? rf95.lastRssi() : 0);

  drawChassisSpeeds(chassisTelemetry.data);
  drawChassisVoltage(chassisTelemetry.data);
  
  display_show();


  delay(100);
}