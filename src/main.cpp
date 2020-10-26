#include <Arduino.h>
#include "src/config.h"


// track the sync to encoder 0 position?

volatile boolean start_sync = false;

// the ratio of needs steps/rev for stepper to spindle_encoder_resolution for each thread pitch we pick, this is calculated in the programme 
volatile int32_t delivered_stepper_pulses=0;          //number of steps delivered to the lead screw stepper motor
//number of steps we should have delivered for a given lead screw pitch

// tracks the current tool position based on delivered steps
volatile int32_t toolPos = 0;


void setup() {
 
  Serial.begin(115200);

  init_display();

  init_encoder();  

  init_stepper();

  init_machine();

  // setup buttons
  
  init_controls();
  
  thread_parameters();

  setFactor();

  init_web();

  Serial.println("setup done");

}

void loop() {
  // put your main code here, to run repeatedly:
   // TODO: why is this run so frequently?
  

  // calculate how far to move
  calcDelta();  

  
  do_display();
  
  do_web();
  //????
  // update factor 
  //if(factor_timer.repeat()){
    //setFactor();
  //}

  // read controls
  read_buttons();

  do_rpm();

}