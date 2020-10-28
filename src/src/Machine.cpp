#include <Arduino.h>
#include "config.h"
#include "Machine.h"

float lead_screw_pitch = 2.0;
int microsteps = MICROSTEPS;
int native_steps = 200;
int motor_steps = 0;
volatile double factor = 0.0;
float pitch = 0.0;
int tpi = 0;
float depth = 0.0;
float pitch_factor = 0.0;
float rapids = 1.0;
float backlash = 0.0;
uint8_t motor_type = MOTOR_TYPE;

void init_machine(){

  
  if(motor_type ==1)
      native_steps = 200;
  if(motor_type ==2)
    native_steps = 400;    
    
  // motor steps per mm
  motor_steps = (microsteps * native_steps) /lead_screw_pitch;
}


/*
lead_screw_pitch
spincle_encoder_resolution
microsteps
motor_type
rapids
backlash



*/
