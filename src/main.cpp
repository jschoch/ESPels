#include <Arduino.h>
#include "src/config.h"


// track the sync to encoder 0 position?

volatile boolean start_sync = false;

// the ratio of needs steps/rev for stepper to spindle_encoder_resolution for each thread pitch we pick, this is calculated in the programme 
volatile int32_t delivered_stepper_pulses=0;          //number of steps delivered to the lead screw stepper motor
//number of steps we should have delivered for a given lead screw pitch

// tracks the current tool position based on delivered steps
volatile int64_t toolPos = 0;


void setup() {
 
  Serial.begin(115200);

  init_display();

  init_encoder();  

  //moved to init_machine
  //init_stepper();

  init_motion();

  init_machine();

  // setup buttons
  
  init_controls();
  
  thread_parameters();

  setFactor();

  init_web();


  esp_log_level_set("perfmon", ESP_LOG_DEBUG);
  esp_err_t e = perfmon_start();
  if(e != ESP_OK){
    Serial.println("perfmon failed to start ");
    Serial.println(e);
  }

  Serial.println("setup done");


}

void loop() {
  do_display();
  
  do_web();

  // read controls
  read_buttons();

  do_rpm();

}
