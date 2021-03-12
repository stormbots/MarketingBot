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

// 'hpLogo', 13x13px
const unsigned char hpLogo [] PROGMEM = {
  0x03, 0x80, 0x33, 0xe0, 0x77, 0xf0, 0x67, 0xf0, 0xe1, 0x08, 0xe9, 0x48, 0xca, 0x58, 0xc2, 0x98, 
  0xd2, 0x38, 0x7c, 0xf0, 0x7d, 0xf0, 0x39, 0xe0, 0x09, 0x80
};




int inc = 0;




OctoWS2811 leds(ledsPerStrip, displayMemory, drawingMemory, config);
GFXcanvas16 canvas(8, 8);
void setup() {
  Serial.begin(115200);
  canvas.setCursor(0,0);
  canvas.setTextColor(WHITE);
  canvas.setTextWrap(false);
  leds.begin();
  leds.show();
}

void loop() {
  canvas.setCursor(inc,0);
  canvas.print("Blue Thunder");
  writeLeds(&leds, &canvas, WIDTH, HEIGHT);
  leds.show();
  delay(100);
  canvas.fillScreen(BLUE);
  inc--;
  if (inc <= -90){
    inc = 8;
  }
  


}

void scrollLogo(OctoWS2811 *led, GFXcanvas16 *grid, int scrollWidth, int scrollHeight, uint8_t *bitmap, uint16_t color){
  int x = 0;
  int y = 0;
  
  grid->drawBitmap(x, y, bitmap, 13, 13, color);
  
}

void writeLeds(OctoWS2811 *led, GFXcanvas16 *grid, int canvasWidth, int canvasHeight){
  int ledNum = 0;
  for (int y = 0; y < canvasWidth; y++){
    for (int x = 0; x < canvasHeight; x++){
      int rgb = grid->getPixel(x, y);
      int red = (rgb>>11)& 0x001f;
      int green=(rgb>>5) & 0x003f;
      int blue= (rgb)    & 0x001f;
      led->setPixel(ledNum, red, green, blue);
      ledNum += 1;
    }
  }
}
