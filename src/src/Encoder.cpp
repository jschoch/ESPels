#include <ESP32Encoder.h>
#include "Encoder.h"
#include "Stepper.h"
#include "motion.h"

volatile int32_t spindlePos = 0;

int spindle_encoder_resolution=2400 ;

ESP32Encoder encoder;


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
  encoder.attachHalfQuad(25, 26);
  encoder.clearCount();
  
}