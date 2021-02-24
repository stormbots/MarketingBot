#include <OctoWS2811.h>
#include <Adafruit_GFX.h>

#define WIDTH 8
#define HEIGHT 8
#define NUM_OF_LEDS 64

// Color definitions
#define BLACK 0x0000
#define BLUE 0x001F
#define RED 0xF800
#define GREEN 0x07E0
#define CYAN 0x07FF
#define MAGENTA 0xF81F
#define YELLOW 0xFFE0
#define WHITE 0xFFFF

const int ledsPerStrip = 64;

DMAMEM int displayMemory[ledsPerStrip*6];
int drawingMemory[ledsPerStrip*6];

const int config = WS2811_GRB | WS2811_800kHz;

// Apple Logo, 8x8px, 
//Does not look good because of the low resolution
const unsigned char appleLogo [] PROGMEM = {
  0x00, 0x08, 0x7e, 0x7c, 0xfc, 0x7e, 0x7e, 0x24
};


OctoWS2811 leds(ledsPerStrip, displayMemory, drawingMemory, config);
GFXcanvas16 canvas(8, 8);
void setup() {
  Serial.begin(115200);
  canvas.drawBitmap(0, 0, appleLogo, 8, 8, RED);
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
      int red = (rgb>>11)& 0x001f;
      int green=(rgb>>5) & 0x003f;
      int blue= (rgb)    & 0x001f;
      led->setPixel(ledNum, red, green, blue);
      ledNum += 1;
    }
  }
}
