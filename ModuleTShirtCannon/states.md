
  //todo: Disabled HANDLING SOMEHOW

Semi-formal definitions used by build eams:
- the rear plate that moves forward and back is the "firing plate", 
- the arm that indexes is called the "indexing arm", 
- the 8 firing tubes together are called the "drum", 
- the whole tilting assembly of the drum, firing plate, and barrel is called the "cannon", 
- and everything that doesn't move and supports the cannon is called the "base"


// hardware:

// up/down window motor, heavy reduction
// has absolute position sensor mag encoder
// limit on up and down
  
// indexing revolver, 8 barrels
// pins for hard stop
// hopefully limit switch

// pressure regulated by humans, and only decreases


/**
  assume indexed and firing

  engage cylinderClamp
  wait for pressure (timer probably)
  open pressureChamber pressure (if it's sufficiently pressurized)
  wait for pressureChamber to empty (timer again)
  seal pressureChamber (rebuild pressure, background)
  disengage cylinderClamp (some wait time)
  release indexLock (wait some time)
  start revolver
  (wait some time to clear lock)
  engage indexLock
  wait for indexSwitch || max runtime exceeded


*/

# Startup 
//complicated
- index locked
- pressure clamp open
- pressureChamber closed
- Motor
- Exit to Pressurizing when IndexSwitch

# Pressurizing
- Index locked
- PressureClamp is open
- PressureChamber is closed
- Motor is off
- Exit to idle when timer ends

# Idle:
- Pressure clamp closed
- index is locked
- Revolver motor is off
- Pressurechamber is closed

# Firing: 
- PressureChamber is open

# Recovery
- PressureChamber is closed
- PressureClamp is open
- Index is unlocked

# ReloadUnlock
- RevolverMotor is on
- Index is unlocked

# ReloadLocked
- RevolverMotor is on
- Index is locked
- go to Pressuring when Index switch or timeout

# Pressurizing
- Motor is off






Running our closed loop and angle positioning

absolute position (PWM pulse)
relative position (quadrature encoder)

relative absolute
0       25

50  <-  75
        100
        0
100     25


encoders = easy;


fallback/safety
if hit a switch, state our position


