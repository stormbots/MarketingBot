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
    Rain(int xCord, int yCord, int s, int l){
      x = xCord;
      y = yCord;
      speedOfRain = s;
      lengthOfTrail = l;
      currentOffset = 0;
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
    int getCurrentOffset(void){
      return currentOffset;
    }
    int getLengthOfTrail(void){
      return lengthOfTrail;
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
    void setCurrentOffset(int c){
      currentOffset = c;
    }
    void setLengthOfTrail(int t){
      lengthOfTrail = t;
    }

  private:
    int x;
    int y;
    //Bigger number is slower
    int speedOfRain;
    int currentOffset;
    int lengthOfTrail;
};


Rain *listOfRain[WIDTH];
void setup() {
  Serial.begin(115200);
  leds.begin();
  leds.show();
  randomSeed(analogRead(A1));
  for(int i = 0; i < WIDTH; i++){
    listOfRain[i] = new Rain(i, 0, random(50,100), random(3,5));
  }
}

void loop() {
  canvas.fillScreen(BLACK);
  for(int i = 0; i < HEIGHT; i++){
    if (listOfRain[i]->getSpeedOfRain() == listOfRain[i]->getCurrentOffset()){
      listOfRain[i]->setCurrentOffset(0);
      if (listOfRain[i]->getYCord() < (HEIGHT + listOfRain[i]->getLengthOfTrail())){
        listOfRain[i]->setYCord(listOfRain[i]->getYCord() + 1);
      }
      else {
        listOfRain[i]->setYCord(0);
      }
      
    } 
    else {
      listOfRain[i]->setCurrentOffset(listOfRain[i]->getCurrentOffset() + 1);
    }
    int x = listOfRain[i]->getXCord();
    int y = listOfRain[i]->getYCord();
    for(int j = 0; j < listOfRain[i]->getLengthOfTrail(); j++){
      int changingColor = map(j, 0, listOfRain[i]->getLengthOfTrail() - 1, 63, 0);
      //changingColor = constrain(changingColor, 63, 0);
      Serial.println(changingColor);
      canvas.drawPixel(x, (y - j), rgbTo16BitColor(0, changingColor, 0));
      
    }
  }
  writeLeds(&leds, &canvas, 8, 8);
  Serial.println("Main Loop");
  leds.show();
  

}
uint16_t rgbTo16BitColor(int r, int g, int b){
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
