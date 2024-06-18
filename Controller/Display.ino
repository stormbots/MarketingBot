#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include <DataPackets.h>

/** Floating point version of map(), since it uses integer division */
float lerp(float x, float in_min, float in_max, float out_min, float out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

// namespace screen{
  Adafruit_SH1107 display = Adafruit_SH1107(64, 128, &Wire);

  void display_setup(){
    delay(250); //screen power on wait
    display.begin(0x3C, true); // Address 0x3C default

    display.clearDisplay();
    display.display();

    // display.setRotation(1);

    display.setTextSize(1);
    display.setTextColor(SH110X_WHITE);
  }

  void display_clear(){
    display.clearDisplay();
  }

  void display_show(){
    display.display();
  }

  void drawControllerVoltage(float voltage){
    //draw the battery icon
    display.drawRect(0,0,20,10,SH110X_WHITE);
    display.drawRect(20,3,2,4,SH110X_WHITE);

    //put the text on it
    display.setCursor(2,2);
    short percent = 0;
    if(voltage > 6){//lead acid
      percent = map(voltage,10,12.5,0,100);
    }
    else{ //feather lipo
      percent = map(voltage,3.2,4.2,0,100);
    }
    display.print((int)voltage);
  }

  void drawChassisVoltage(ChassisTelemetry data){
    float voltage = data.batteryVoltage/10.0;
    int x=12;
    int y=43;
    
    //draw a battery graphic
    display.drawRect(x,y,20,12,SH110X_WHITE);
    display.drawRect(x+1,y,4,-2,SH110X_WHITE);
    display.drawRect(x+14,y,4,-2,SH110X_WHITE);
    
    float percent = 0;
    if(voltage > 6){//lead acid
      percent = lerp(voltage,10,12.5,0,100);
    }
    else{ //feather lipo
      percent = lerp(voltage,3.2,4.2,0,100);
    }

    //justify the text based on voltage
    if(percent>=100){
      x+=0;
    }
    else if(percent>=10 || percent<0 ){
      x+=2+4;
    }
    else{
      x+=2+8+2;
    }
    display.setCursor(x,y+3);
    display.print((int)percent);
  }

  void drawChassisPressure(int pressure){

  }

  void drawChassisSpeeds(ChassisTelemetry data){
    int x=1;
    int y=25;
    display.drawLine(3, y, 60, y, SH110X_WHITE);

    y=45;
    // display.setCursor(0,50);
    // display.print(data.speed.left);
    // display.print(" ");
    // display.print(data.speed.right);

    //draw the wheel speed graphs
    display.drawRect(x,  y,6, 10,SH110X_WHITE);
    display.drawRect(x,  y,6,-10,SH110X_WHITE);
    display.fillRect(x+2,y,2,(int)map(data.speed.left,-100,100,-10,10),SH110X_WHITE);
    x=63-5;
    display.drawRect(x,y,6, 10,SH110X_WHITE);
    display.drawRect(x,y,6,-10,SH110X_WHITE);
    display.fillRect(x+2,y,2,(int)map(data.speed.right,-100,100,-10,10),SH110X_WHITE);
  }

  void drawSignalStrength(int rssi){
    display.setCursor(50,2);
    // display.print(rssi);

    //draw a cute little antenna
    display.drawLine(49,10,49,3,SH110X_WHITE);
    display.drawCircle(49,3,2,SH110X_WHITE);

    //Debugging fudgery
    // rssi = map(millis()%5000,0,5000,-120,-50); //for debugging the graphic
    //add some text for precision
    display.setTextSize(1);
    display.setCursor(35,12);
    display.println(rssi);



    if(rssi==0){ //not connected
      if(millis()%400>300) return; //flash the NC
      //no signal
      display.drawCircle(57,5,4,SH110X_WHITE);
      display.drawLine(52,10,63,0,SH110X_WHITE);
      return;
    }

    if(rssi<-80){ // ! for poor connection
      display.fillCircle(60,9,1,SH110X_WHITE);
      display.fillRoundRect(
        59,1,
        3,5,
        2,SH110X_WHITE);
    }

    //visualize the signal
    // display.drawLine(63,10,63,0,SH110X_WHITE);
    rssi = constrain(rssi,-120,-50);
    int x=map(rssi,-120,-50,50,63);
    int y=map(rssi,-120,-50,10,0);
    display.fillTriangle(50,10,x,10,x,y,SH110X_WHITE);
  }

// }