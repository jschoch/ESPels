#ifdef UNIT_TEST
    #include "ArduinoFake.h"
#else
    #include "Arduino.h"
#endif

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

void setup() {
 
  Serial.begin(115200);

  init_encoder();  

  init_motion();

  init_machine();

  init_controls();
  
  //setFactor();
  gs.setELSFactor(pitch);

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

  sendUpdates();

  do_rpm();
  do_state();

}
