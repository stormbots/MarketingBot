#include <OctoWS2811.h>
#include <Adafruit_GFX.h>

#define WIDTH 8
#define HEIGHT 8
#define NUM_OF_LEDS 64

const int ledsPerStrip = 64;

DMAMEM int displayMemory[ledsPerStrip*6];
int drawingMemory[ledsPerStrip*6];

const int config = WS2811_GRB | WS2811_800kHz;

OctoWS2811 leds(ledsPerStrip, displayMemory, drawingMemory, config);
GFXcanvas16 canvas(8, 8);
void setup() {
  Serial.begin(115200);
  canvas.fillScreen(0xF800);
  leds.begin();
  leds.show();
  delay(9000);
}

void loop() {
  writeLeds(leds, canvas, 8, 8);
  leds.show();
  Serial.println("helllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllo");
  

}

void writeLeds(OctoWS2811 leds, GFXcanvas16 canvas, int canvasWidth, int canvasHeight){
  int ledNum = 0;
  for (int i = 0; i < canvasWidth; i++){
    Serial.println(i);
    for (int j = 0; j < canvasHeight; j++){
      Serial.println(j);
      int rgb = canvas.getPixel(i, j);
      Serial.println("Pixel Value get");
      int red = (rgb >> 16) & 0xFF;
      int green = (rgb >> 8) & 0xFF;
      int blue = rgb & 0xFF;
      Serial.println("conversion");
      leds.setPixel(ledNum, red, green, blue);
      Serial.println("set pixel");
      ledNum++;
      Serial.println("increase num");
    }
  }
}
