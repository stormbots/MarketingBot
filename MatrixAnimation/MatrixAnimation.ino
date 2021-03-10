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


OctoWS2811 leds(ledsPerStrip, displayMemory, drawingMemory, config);
GFXcanvas16 canvas(8, 8);

class Rain{
  public:
    Rain();
    Rain(int xCord, int yCord, int s){
      x = xCord;
      y = yCord;
      speedOfRain = s;
   };
    int getXCord(void){
      return x;
    }
    int getYCord(void){
      return y;
    }
    int getSpeedOfRain(void){
      return speedOfRain;
    }

    void setXCord(int xCord){
      x = xCord;
    }
    void setYCord(int yCord){
      y = yCord;
    }
    void setSpeedOfRain(int s){
      speedOfRain = s;
    }

  private:
    int x;
    int y;
    //Bigger number is slower
    int speedOfRain;
};


void setup() {
  Serial.begin(115200);
  leds.begin();
  leds.show();
  Rain *listOfRain[WIDTH];
  for(int i = 0; i < WIDTH; i++){
    listOfRain[i] = new Rain(i, 0, random(0, 10));
  }
}

void loop() {

  

}
uint16_t rgbToColor(int r, int g, int b){
  uint16_t color;
  color = r;
  color = (color<<6 | g);
  color = (color<<5 | b);
  return color;
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
