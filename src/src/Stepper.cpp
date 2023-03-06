#include "Stepper.h"
#include <cmath>
#include "gear.h"
#include "rmtStepper.h"
#include "state.h"
#include "genStepper.h"

int microsteps = Z_MICROSTEPPING;

int native_steps = Z_NATIVE_STEPS_PER_REV;
int motor_steps = 0;


// number of ticks to wait between timer events
int timertics = 10;

// used to figure out how many steps we need to get to the right position
// delta is in stepper steps
volatile int delta = 0;

// this is the time between a low and the next high or dir and step
volatile int delay_ticks = 3;
volatile int previous_delay_ticks = 0;
volatile int min_delay_ticks = 5;

// jog delay based on accel table
volatile int jog_delay_ticks = 0;
volatile bool z_dir = true; //CW
volatile bool z_prev_dir = true;
volatile bool z_moving = false;


int use_limit = false;
volatile int64_t calculated_stepper_pulses=0;

volatile int32_t jog_steps = 0;
volatile double jog_scaler = 0.2;
volatile uint16_t vel = 1;
volatile bool pos_feeding = false;


/*

Refactoring notes:

calculate max speed and create timer with slower frequency to act on encoder ticks.
rip everything not related to step signal gen out of 80mhz timer or use RMT stepping.

Add some debugging execution time checks to make sure the calculations are fast enough.
Consider pinning tasks to cores to distribute load.  One core is pegged the other is mostly idle.

try to make code testable but also can run in timer.  research how to do this.

consider moving display code into state functions


*/



void init_stepper(){

  pinMode(Z_DIR_PIN, OUTPUT);
  pinMode(Z_STEP_PIN, OUTPUT);

  toolPos = 0;
  stepsPerMM = motor_steps / lead_screw_pitch;


  gs.stepper.config.channel = RMT_CHANNEL_0;
  gs.stepper.config.stepPin = (gpio_num_t) Z_STEP_PIN;
  gs.stepper.config.dirPin = (gpio_num_t) Z_DIR_PIN;

  gs.stepper.init();


}
