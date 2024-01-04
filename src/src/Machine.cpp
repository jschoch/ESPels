#include <Arduino.h>
#include "config.h"
#include "Machine.h"
#include "state.h"
#include "Stepper.h"

uint8_t motor_type = MOTOR_TYPE;


void init_machine(){
  // TODO: refactor this out, this seems silly 
  if(motor_type ==1)
      native_steps = 200;
  if(motor_type ==2)
    native_steps = 400;    
}


