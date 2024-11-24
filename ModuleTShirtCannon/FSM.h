#ifndef FSM_H
#define FSM_H
#include <DataPackets.h>
#include "Pins.h"
#include <Encoder.h>
#include <elapsedMillis.h>


enum Index{open,closed};
enum FiringPlate{open,closed};
enum DumpValve{open,closed};

struct MechanismStates{
  Index index,
  FiringPlate firingPlate,
  DumpValve dumpValve,
  int barrelSpeed,
}

class BotState{
  /**Create default idle state*/
  MechanismStates states={
    .index=Index.closed,
    .firingplate=FiringPlate.closed,
    .dumpValve=DumpValve.closed,
    .index=Index.closed,
    
  }

  BotState(){

  };

  idleState{
    
  }


 SetIndex(IndexState)
};