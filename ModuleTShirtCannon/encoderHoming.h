
//Degrees measured from horizon
//#define ELEVATION_MIN_DEGREES 40
//#define ELEVATION_MAX_DEGREES -10
//#define ELEVATION_ENCODER_RANGE 1261
//#define ANALOG_ENCODER_RESOLUTION 1024 //todo FIND IN DATASHEET
#define ANALOG_UP_MICROS 266 //uS
#define ANALOG_DOWN_MICROS 453 //uS


volatile unsigned long int fall = 0;
volatile unsigned long int rise = 0;
volatile int duty = 0;
//ignore first sample
volatile short int samples; 
volatile boolean lock=false;



void edgeTimer(){
  if( lock == false ){
    lock = true;
    if(digitalRead(8)==HIGH){
      rise=micros();
    }
    else if(digitalRead(8)==LOW){
      fall=micros();
      duty = abs(rise-fall);
      samples +=1;
    }
    lock = false;
  }
}


void home(){
  attachInterrupt(digitalPinToInterrupt(ABSOLUTE_ENCODER_PIN), edgeTimer, CHANGE);

  //count some number of samples
  while(samples < 20)/*do nothing*/;

  detachInterrupt(digitalPinToInterrupt(ABSOLUTE_ENCODER_PIN));
  Serial.println(duty);
}


int getEncoderPosition(){
  return map(duty, ANALOG_DOWN_MICROS, ANALOG_UP_MICROS,0,1024);
}
