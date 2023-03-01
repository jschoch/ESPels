#include <Arduino.h>
#include "config.h"
#include "Machine.h"
#include "state.h"
#include "Stepper.h"

uint8_t motor_type = MOTOR_TYPE;

double lead_screw_pitch = 2.0;
double pitch = 0.06;
double oldPitch = pitch;

int tpi = 0;
double depth = 0.0;

//TODO configure using config.h
double rapids = 1.0;
double backlash = 0.0;

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


