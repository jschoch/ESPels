#include <Arduino.h>

#define ARDUINOJSON_USE_LONG_LONG 1
#define ARDUINOJSON_USE_DOUBLE 1

#include "src/config.h"
#include "src/Encoder.h"
#include "Bounce2.h"
#include "esp_err.h"
#include "esp_log.h"
#include "yasm.h"
#include "src/web.h"
#include "src/display.h"
#include "src/motion.h"
#include "src/Controls.h"
#include "src/BounceMode.h"
#include "src/myperfmon.h"
#include "src/Machine.h"

// track the sync to encoder 0 position?

volatile boolean start_sync = false;

// the ratio of needs steps/rev for stepper to spindle_encoder_resolution for each thread pitch we pick, this is calculated in the programme 
volatile int32_t delivered_stepper_pulses=0;          //number of steps delivered to the lead screw stepper motor
//number of steps we should have delivered for a given lead screw pitch


void setup() {
 
  Serial.begin(115200);

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
  bounce_yasm.next(BounceIdleState);


  Serial.println("setup done");
}

void loop() {
  do_web();

  // read controls
  //read_buttons();

  sendUpdates();

  do_rpm();
  do_state();

}
