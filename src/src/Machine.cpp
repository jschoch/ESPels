#include <Arduino.h>
#include "config.h"
#include "Machine.h"
#include "state.h"
#include "Stepper.h"

uint8_t motor_type = MOTOR_TYPE;

//TODO: is this redundant?
double lead_screw_pitch = 2.0;

//TODO configure using config.h
double backlash = 0.0;

void init_machine(){

  
  if(motor_type ==1)
      native_steps = 200;
  if(motor_type ==2)
    native_steps = 400;    
    
  // motor steps per mm
  // this should all be in gs.c now
  //motor_steps = (microsteps * native_steps) ;
  //stepsPerMM = motor_steps / lead_screw_pitch;
  //mmPerStep = (double) 1/stepsPerMM;
}


