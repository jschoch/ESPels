#include <Arduino.h>
#include "config.h"
#include "Machine.h"

float lead_screw_pitch = 2.0;
int microsteps = 8;
int native_steps = 200;
int motor_steps = 0;
float pitch = 0.06;
float oldPitch = pitch;
int tpi = 0;
float depth = 0.0;

float rapids = 1.0;
float backlash = 0.0;
uint8_t motor_type = MOTOR_TYPE;


void init_machine(){

  
  if(motor_type ==1)
      native_steps = 200;
  if(motor_type ==2)
    native_steps = 400;    
    
  // motor steps per mm
  motor_steps = (microsteps * native_steps) ;
  stepsPerMM = motor_steps / lead_screw_pitch;
  mmPerStep = (double) 1/stepsPerMM;
  init_stepper();
}


/*
lead_screw_pitch
spindle_encoder_resolution
microsteps
motor_type
rapids
backlash



*/
