#include <Arduino.h>
#include "config.h"
#include "Machine.h"

float lead_screw_pitch = 2.0;
int microsteps = 16;
int native_steps = 200;
int motor_steps = 0;
volatile double factor = 0.0;
float pitch = 0.06;
int tpi = 0;
float depth = 0.0;

float rapids = 1.0;
float backlash = 0.0;
uint8_t motor_type = MOTOR_TYPE;
long mm_per_tick = 0;
float encoder_factor = 0;

void init_machine(){

  
  if(motor_type ==1)
      native_steps = 200;
  if(motor_type ==2)
    native_steps = 400;    
    
  // motor steps per mm
  motor_steps = (microsteps * native_steps) ;
  mm_per_tick = 1 / spindle_encoder_resolution /lead_screw_pitch;
  encoder_factor = (float)spindle_encoder_resolution / (motor_steps/lead_screw_pitch);
}


/*
lead_screw_pitch
spindle_encoder_resolution
microsteps
motor_type
rapids
backlash



*/
