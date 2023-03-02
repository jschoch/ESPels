#ifndef _Stepper_h
#define _Stepper_h

#include <Arduino.h>
#include "config.h"
#include "rmtStepper.h"

extern volatile bool z_pause;


// number of ticks to wait between timer events
extern int timertics;

// used to figure out how many steps we need to get to the right position
// delta is in stepper steps
extern volatile int delta;

// this is the time between a low and the next high or dir and step
extern volatile int delay_ticks;
extern volatile int previous_delay_ticks;
extern volatile int min_delay_ticks;

// jog delay based on accel table
extern volatile int jog_delay_ticks;
extern volatile bool z_dir; //CW
extern volatile bool z_prev_dir;
extern volatile bool z_moving;
extern volatile int32_t jogs;
extern volatile double toolRelPos;
extern double oldToolRelPosMM;
extern volatile double targetToolRelPos;


extern int use_limit;
extern volatile int64_t calculated_stepper_pulses;

extern volatile int32_t jog_steps;
extern volatile double jog_scaler;
extern volatile uint16_t vel;
extern volatile double stopPos;
extern volatile double stopNeg;
extern volatile bool useStops;
extern volatile bool pos_feeding;

extern int microsteps;

extern int native_steps;
extern int motor_steps;

extern rmtStepper::State zstepper;


void init_stepper(void);
bool getDir(void);
bool setDir(bool);
void updatePosition();
void init_feed();

#endif
