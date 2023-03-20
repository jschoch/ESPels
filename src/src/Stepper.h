#ifndef _Stepper_h
#define _Stepper_h

#include <Arduino.h>
#include "config.h"

// TOOD: move to a machine state struct
extern volatile bool pos_feeding;

extern int microsteps;

extern int native_steps;
extern int motor_steps;




#endif
