

elapsedMillis heartbeatTimer;
void heartbeat(){
  digitalWrite(LED_BUILTIN, heartbeatTimer >200);
  //Reset it
  if(heartbeatTimer>500) heartbeatTimer=0;
}