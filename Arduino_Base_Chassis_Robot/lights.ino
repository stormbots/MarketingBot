void light_setup(){


}


//knob vrb(the right one)selects the setting
// INSTRUCTIONS: Get acquainted with how far a single step is on the knob before operating
void light_loop(double setting){
  if (setting <1125){
    //nothing allowed here this is for when the lights are off
  }
  //This should have higher brightness for indoors and when necessary
  else if (setting < 1250){
    //put function call for individual patterns in here
  }
  else if (setting < 1375){
    //put function call for individual patterns in here
  }
  else if (setting < 1500){
    //put function call for individual patterns in here
  }
  //These should have higher brightness for outdoors and when necessary
  else if (setting < 1625){
    //put function call for individual patterns in here
  }
  else if (setting < 1750){
    //put function call for individual patterns in here
  }
  else if (setting < 1875){
    //put function call for individual patterns in here
  }
  else if (setting < 2000){
    //put function call for individual patterns in here
    //maybe run all in order here
  }
  //you can add more settings at the cost of it being more likely for the operator to mess up
}
