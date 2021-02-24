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
}

void loop() {
  writeLeds(&leds, &canvas, WIDTH, HEIGHT);
  leds.show();
  

}

void writeLeds(OctoWS2811 *led, GFXcanvas16 *grid, int canvasWidth, int canvasHeight){
  int ledNum = 0;
  for (int i = 0; i < canvasWidth; i++){
    for (int j = 0; j < canvasHeight; j++){
      int rgb = grid->getPixel(i, j);
      int red = (rgb >> 16) & 0xFF;
      int green = (rgb >> 8) & 0xFF;
      int blue = rgb & 0xFF;
      led->setPixel(ledNum, red, green, blue);
      ledNum += 1;
    }
  }
}
