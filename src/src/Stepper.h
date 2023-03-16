#ifndef _Stepper_h
#define _Stepper_h

#include <Arduino.h>
#include "config.h"



// number of ticks to wait between timer events
extern int timertics;

// used to figure out how many steps we need to get to the right position
// delta is in stepper steps
extern volatile int delta;

// this is the time between a low and the next high or dir and step
extern volatile int delay_ticks;
extern volatile int previous_delay_ticks;
extern volatile int min_delay_ticks;



extern int use_limit;
extern volatile int64_t calculated_stepper_pulses;


// TOOD: move to a machine state struct
extern volatile bool pos_feeding;

extern int microsteps;

extern int native_steps;
extern int motor_steps;



void init_stepper(void);
bool getDir(void);
bool setDir(bool);
void updatePosition();
void init_feed();

#endif
