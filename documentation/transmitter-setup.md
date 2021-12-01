## Transmitter Setup

This documentation details the configuration of a FlySky FS-i6X flight controller. 

This documentation is intended to be used in the case of loss or damage to the original
controller, so that an identical replacement can be configured again to match the existing code. 
As such, it may be difficult to read if you do not have a controller in front of you. 

If this controller is not available for purchase, a replacement controller model will need at least

- at least 8 output channels
- PPM output from the reciever
- Configurable channel values on loss of reciever (to safely disable robot)

It will then need to be configured appropriately, such that all signals are handled appropriately.

```
======Display Emulation Box======
1 [            |============]
2 [            |======      ]
3
4
5
6
```
### Configuration tips
Navigation of the controller setup is very inconsistent, and buttons may not work as expected. 
It's advised to be careful when entering values and double checking your work. It's easy to 
nudge something unintentionally, or to change values but not have them save and need to be re-entered.

Of note, there's two non-obvious interactions with the setup:
- `OK` is often used as select, with `Up`/`Down` selecting between values
- Holding `OK` usually will reset the selection to a default value
- Pressing `Cancel` will typically exit the screen _without_ saving. 
- Holding `Cancel` will usually save the values and then exit the screen.


# Channels and Button assignments

### Physical Switch Description 
The controllers signals are mapped as follows: 
```
          __________
__________[        ]_________
SWA  SWB  VRA   VRB  SWC  SWD 
```
- SWA, SWB, SWD are simple on/off switches
- SWC is a three position switch
- VRA, VRB are adjustable knobs
- Left Joystick Y is throttle, and has no spring. 
- Other joystick axes spring to center. 

### Channel assignments
1: Right Joystick X
2: Right Joystick Y
3: Left Joystick Y (throttle)
4: Left Joystick X
5: VRA
6: VRB
7: Mixed at 50% of SWA + 25% of SWB
8: Mixed at 50% SWC + 25% SWB

# System Menu
### Output Mode
RX Battery  (maybe needs to be set to avoid beeping?)
### Output Mode
Select PPM
### Sticks Mode
Select Mode 2
### Aux Switches
Set `Ch : 10`
Set all others to `On`
This enables all buttons to work correctly. By default most buttons are disabled. 

### RX Settings
#### Failsafe
Channel 1 : 0%
Channel 2: 0%
Channel 3: 0%
Channel 4: 0%
Channel 5: 0%
Channel 6: OFF
Channel 7: OFF
Channel 8: OFF
Channel 9: OFF
Channel 10: OFF
# Functions Setup
### Mixes
We configure 2 mixes, which can be selected by the `Mix #` line.
They should look as follows. This controller adds a portion of the `master`
channel's output to the `slave` channel. 

```
======Mixes====
Mix #1        
Mix is         On              
Master        Ch9
Slave         ch7
Pos. mix      25%
Neg. mix      25%
Offset         0%
```

```
======Mixes====
Mix #2           
Mix is         On
Master       Ch10
Slave         ch8
Pos. mix      25%
Neg. mix      25%
Offset         0%
```

#### Aux Channels
```
======Aux Channels====
    Channel 5
    Source VrA

    Channel 6
    Source VrB
    
    Channel 7
    Source SwA
    
    Channel 8
    Source SwC
    
    Channel 9
    Source SwB
    
    Channel 10
    Source SwD
```

#### End Points
Navigation on this screen is odd. You must flip the physical switch assigned to a 
channel in order to switch columns. This only applies to channels 7-10 though, as 
those are the only ones with switches

```
======End Points====
    Ch1  100%  100%
    Ch2  100%  100%
    Ch3  100%  100%
    Ch4  100%  100%
    Ch5  100%  100%
    Ch6  100%  100%
    Ch7   50%   50%
    Ch8   50%   50%
    Ch9   50%   50%
    Ch10  50%   50%
```



