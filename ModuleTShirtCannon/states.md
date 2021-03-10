

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

