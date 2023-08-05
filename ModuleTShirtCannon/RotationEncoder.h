#ifndef ROTATIONENCODER
#define ROTATIONENCODER

int interruptPinRotation;
elapsedMicros isrTimerRotation;
unsigned int pulseRotation=0;

static void ISRRotationRead(){
  if(digitalRead(interruptPinRotation)==HIGH){
    isrTimerRotation=0;    
  } else {
    pulseRotation=isrTimerRotation;
  }
}

void ConfigRotationPWM(int pin ){
  interruptPinRotation=pin; 
  pinMode(interruptPinRotation, INPUT);
  attachInterrupt(digitalPinToInterrupt(interruptPinRotation), ISRRotationRead, CHANGE);
}

int ReadRotationPWM(){
  return pulseRotation;
}

int ReadRotationDegrees(){
  int pulse=pulseRotation;
  return map(pulse,0,4096,0,360);
}

#endif
