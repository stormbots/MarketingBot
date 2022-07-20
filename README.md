# MarketingBot

  This is the Github Repository for the Stormbots Marketing Bot, also known as "Mark". This robot was designed, programmed, and built during the Covid-19 pandemic. The robot was designed to serve as a custom platform for marketing events, before the creation of Mark the Stormbots used whatever robot was available from the last competition season. Mark was designed to be easier to operate and maintain than the competition bots. 
  
  Mark was built with the idea of modularity at its core. The chassis can accept multiple different "modules" on the flat top. As of the writing of this Read Me on 7/19/2022 there was one module, a pneumatic T-shirt cannon. To facilitate this modularity Mark does not use the standard FRC Roborio as a control board. Instead it uses two Teensy 3.2 microcontrollers. Additionally, Mark uses radio PPM communication and a RC controller rather than the standard FRC computer and controller setup. This controller setup was chosen for its ease of transport and use compared to a bulky computer. Because of this radically different control schema programming for Mark is remarkably different compared to programming for any other bot.
  
  Because of this difference in control scheme Mark utilizes c++ rather than Java.
