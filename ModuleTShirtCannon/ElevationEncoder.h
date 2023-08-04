#ifndef ELEVATIONENCODER
#define ELEVATIONENCODER

int interruptPinElevation;
elapsedMicros isrTimerElevation;
unsigned int pulseElevation=0;

static void ISRElevationRead(){
  if(digitalRead(interruptPinElevation)==HIGH){
    isrTimerElevation=0;    
  } else{
    pulseElevation=isrTimerElevation;
  }
}

void ConfigElevationPWM(int pin ){
  interruptPinElevation=pin; 
  pinMode(interruptPinElevation, INPUT);
  attachInterrupt(digitalPinToInterrupt(interruptPinElevation), ISRElevationRead, CHANGE);
}

int ReadElevationDegrees(){
  return map(pulseElevation,1,1036,1,360)-120;
}


#endif
