# MarketingBot

  This is the Github Repository for the Stormbots Marketing Bot, also known as "Mark". This robot was designed, programmed, and built during the Covid-19 pandemic. The robot was designed to serve as a custom platform for marketing events, before the creation of Mark the Stormbots used whatever robot was available from the last competition season. Mark was designed to be easier to operate and maintain than the competition bots. 
  
  Mark was built with the idea of modularity at its core. The chassis can accept multiple different "modules" on the flat top. As of the writing of this Read Me on 7/19/2022 there was one module, a pneumatic T-shirt cannon. To facilitate this modularity Mark does not use the standard FRC Roborio as a control board. Instead it uses two Teensy 3.2 microcontrollers. Additionally, Mark uses radio PPM communication and a RC controller rather than the standard FRC computer and controller setup. This controller setup was chosen for its ease of transport and use compared to a bulky computer. Because of this radically different control schema programming for Mark is remarkably different compared to programming for any other bot.
  
  Because of this difference in control scheme Mark utilizes c++ rather than Java. The code is divided into arduino files for each module and one for the chassis. These files are completely independent aside from the radio communication allowing for the easy swap of modules. Each module and the chassis share some common features. All programs will need to utilize both the pulse position and ellapsedMillis libraries. The pulse position library allows for radio communication from the robot to the controller. EllapsedMillis is used for the heartbeat of the teensy controller, a led which turns on and off at one second intervals. this led shows that the teensy is operating nominally and is recieving power. All code other than that pertaining to these two libraries will be module specific.
  
                                                                      **_Radio_**
  
  The radio communication is one of the most important and most complex parts of the marketing bot code. The code must communicate from the controller to the chassis, and the chassis to whatever module. This is done through the pulsePosition module which how it works is beyond my ability to easily explain through text without writing a small essay. However, I can provide an explanation of how to use it when writing a new module. First you need to declare a radio input pin and a radio output pin on the teensy. 
  
  **Example**
  #define RADIO_IN_PIN 22
  #define RADIO_OUT_PIN 23
  **Example**
  
  Next, you must create a variable for the input and output. One thing to note is that you don't need a output on the module as it doesnt communicate TO anywhere, it only recieves signals from the chassis.
  
  **Example**
  PulsePositionOutput radioOutput;
  PulsePositionInput radioInput;
  **Example**
  
  Then, in the setup portion of the file you must begin transmitting on the input and output channels. 
  
  **Example**
  radioOutput.begin(RADIO_OUT_PIN);
  radioInput.begin(RADIO_IN_PIN);
  **Example**
  
  After this setup you can now read the radio channels and declare them as variables. Each channel is used for a specific thing. Channels cannot be used for two things  at once. The chassis as it exists currently uses channels 1 for throttle, 2 for turning, and 7 for shifting. This leaves the remaining channels: 3,4,5,6, and 8 for use in your module. 
  
  **Example**
  float turningValue= radioInput.read(1); 
  float shiftValue = radioInput.read(7);
  **Example**

   These .read()'s return a value between 1000 and 2000 with all the way down being 1000 and all the way up being 2000. Something important to note is that you shouldn't do any <1500 or >1500's as the signal is not perfectly steady and will fluctuate. Instead do >1250 or <1750. 
  
