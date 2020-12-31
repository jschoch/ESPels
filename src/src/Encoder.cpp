#include <ESP32Encoder.h>
#include "Encoder.h"
#include "Stepper.h"
#include "motion.h"

volatile int64_t spindlePos = 0;

int spindle_encoder_resolution=2400 ;
int64_t last_count = 0;
int rpm = 0;

ESP32Encoder encoder;

Neotimer rpm_timer = Neotimer(100);

int virtEncoderCount = 0;
bool virtEncoderEnable = false;
bool virtEncoderDir = true;

void init_encoder(){
  /*   -------------------------------------   OLD   --------------------------------------
  //we must initialize rorary encoder 
  pinMode(EA,INPUT_PULLUP);
  pinMode(EB,INPUT_PULLUP);
  

  encoder0Pos = 0;
  // encoder interrupts
  attachInterrupt(digitalPinToInterrupt(25),doEncoderA,CHANGE);
  attachInterrupt(digitalPinToInterrupt(26),doEncoderB,CHANGE);
  */
  ESP32Encoder::useInternalWeakPullResistors=UP;
  encoder.attachFullQuad(25, 26);
  encoder.clearCount();
  
}

void do_rpm(){
  if(rpm_timer.repeat()){
    // TODO: put this in the webUI
    static int64_t c = encoder.getCount();
    long count_diff = abs(last_count - c);
    float revolutions = (float) count_diff / spindle_encoder_resolution;
    rpm = revolutions * 10 * 60;
    last_count = c;    
  }
}
