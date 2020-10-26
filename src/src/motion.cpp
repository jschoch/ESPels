#include <Arduino.h>
#include "motion.h"
#include "config.h"




void IRAM_ATTR calcDelta(){

  
  /*
  // calculate the current spindle position in motor steps.
  spindlePos = encoder.getCount();
  int64_t currentToolPos = toolPos;

  calculated_stepper_pulses = (int32_t)(factor * spindlePos);

  // calculate the delta in motor steps between the current spindle position and the current motor position aka toolPos

  if(feeding_dir){
    delta = currentToolPos - calculated_stepper_pulses;   
  }else{
    delta = calculated_stepper_pulses - currentToolPos;
  }
  */
  
  /*
  //  For debugging
  if (delta > 1){
    Serial.print(z_moving);
    Serial.print(",");
    Serial.print(z_pause);
    Serial.print(",");
    Serial.print(delay_ticks);
    
    Serial.print(",SP:");
    Serial.print((int32_t)spindlePos);
    Serial.print(",CP: ");
    Serial.print(calculated_stepper_pulses);
    Serial.print(",TP: ");
    Serial.print((int32_t)toolPos);
    Serial.print(",CTP: ");
    Serial.print((int32_t)currentToolPos);
    Serial.print(",D:");
    Serial.print(getDir());
    Serial.print(",FD: ");
    Serial.print(feeding_dir);
    Serial.print(",");
    Serial.println((int32_t)delta);
    }
  */

  // TODO: Calculate acceleration here, 
  // how much ramp is needed and when is it too much to stay in sync?

  /*
  if delta is increasing we should reduce the step delay

  if delta is decreasing we should increase the step delay

  if delta is somewhere close to 0 the step delay should stay the same.
  */
  
}


void init_motion(){
  
  // motor_steps = (microsteps * native_steps) /lead_screw_pitch;
  // factor is motor steps per spindle tick
  setFactor();
}