#include <Arduino.h>
#include "config.h"
#include "Machine.h"

float lead_screw_pitch = LEAD_SCREW_PITCH;
int microsteps = MICROSTEPS;
int native_steps = 200;
int motor_steps = 0;
volatile float factor = 0.0;
float pitch = 0.0;
int tpi = 0;
float depth = 0.0;
float pitch_factor = 0.0;

void init_machine(){

  
  if(MOTOR_TYPE ==1)
      native_steps = 200;
  if(MOTOR_TYPE ==2)
    native_steps = 400;    
    
  // motor steps per mm
  motor_steps = (microsteps * native_steps) /lead_screw_pitch;
}
