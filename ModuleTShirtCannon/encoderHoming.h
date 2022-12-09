
//Degrees measured from horizon
//#define ELEVATION_MIN_DEGREES 40
//#define ELEVATION_MAX_DEGREES -10
//#define ELEVATION_ENCODER_RANGE 1261
//#define ANALOG_ENCODER_RESOLUTION 1024 //todo FIND IN DATASHEET
#define ANALOG_UP_MICROS 266 
#define ANALOG_DOWN_MICROS 453

/* reported value off the machine at all the way up
 * duty cycle: 4294966843
 * encoder pos: 232
 * 
 */

volatile unsigned long int fall = 0;
volatile unsigned long int rise = 0;
volatile unsigned long int duty = 0;
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
      if(rise==0)return;
      fall=micros();
      duty = abs(rise-fall);
      samples +=1;
    }
    lock = false;
  }
}


int getEncoderPosition(){
  return 40;
  return map(abs(duty), ANALOG_DOWN_MICROS, ANALOG_UP_MICROS,-10,40);
}

void encoderHome(){
  attachInterrupt(digitalPinToInterrupt(ABSOLUTE_ENCODER_PIN), edgeTimer, CHANGE);

  //count some number of samples
  while(samples < 20)/*do nothing*/;

  detachInterrupt(digitalPinToInterrupt(ABSOLUTE_ENCODER_PIN));
//    delay(10000);
    Serial.print("duty cycle: ");
    Serial.println(duty);
    Serial.print("encoder pos: ");
    Serial.println(getEncoderPosition());
//    delay(10000);
}
