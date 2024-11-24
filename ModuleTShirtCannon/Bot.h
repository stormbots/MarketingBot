#ifndef BOT_H
#define BOT_H


enum State{open,closed};

struct MechanismStates{
  State barrelLock;
  State firingPlate ;
  State dumpValve ;
  int barrelSpeed ;

  /** Create a new initial state from the default Idle config */
  static MechanismStates idle(){
    MechanismStates state={
    .barrelLock=State::open,
    .firingPlate=State::open,
    .dumpValve=State::closed,
    .barrelSpeed=0
    };
  return state;
  }

  MechanismStates setBarrelLock(State state){
    barrelLock=state;
    return *this;
  }
  
  MechanismStates setFiringPlate(State state){
    firingPlate=state;
    return *this;
  }

  /** Finalize the bot configuration and write it to the system*/
  void apply(){
    //do things with the current state and halt allowed mutation
  }

};

#endif