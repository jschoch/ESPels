#include "Stepper.h"
#include <cmath>
#include "gear.h"
#include "rmtStepper.h"
/////////////////////////////////////////////////
// stepper timer stuff
//////////////////////////////////////////////////


volatile bool z_pause = false;


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
volatile bool jogging = false;
volatile int32_t jogs = 0;
volatile double toolRelPos = 0;
double oldToolRelPosMM = 0;
volatile double targetToolRelPos = 0;


int use_limit = false;
volatile int64_t calculated_stepper_pulses=0;

volatile bool jog_done = true;
volatile int32_t jog_steps = 0;
double jog_mm = 0;
volatile double jog_scaler = 0.2;
volatile uint16_t vel = 1;
volatile double stopPos = 0;
volatile double stopNeg = 0;
volatile bool useStops = true;
volatile bool stopPosEx = false;
volatile bool stopNegEx = false;
volatile int exDelta = 0;
volatile bool pos_feeding = false;

rmtStepper::State zstepper;

/*

Refactoring notes:

calculate max speed and create timer with slower frequency to act on encoder ticks.
rip everything not related to step signal gen out of 80mhz timer or use RMT stepping.

Add some debugging execution time checks to make sure the calculations are fast enough.
Consider pinning tasks to cores to distribute load.  One core is pegged the other is mostly idle.

try to make code testable but also can run in timer.  research how to do this.

consider moving display code into state functions


*/


/////////////  naive acceleration

// This table defines the acceleration curve as a list of (steps, delay) pairs.
// 1st value is the cumulative step count since starting from rest, 2nd value is delay in microseconds.
// 1st value in each subsequent row must be > 1st value in previous row
// The delay in the last row determines the maximum angular velocity.

// borrowed from Guy Carpente clearwater SwitecX25
// the 2nd value is the delay in timer ticks.  Step pulses are currently 10micros 
static uint16_t defaultAccelTable[][2] = {
  {   20, 3000},
  {   35, 2005},
  {  80, 1000},
  {  130,  800},
  {  250,  600},
  {   450, 405},
  {  600, 350},
  {  700,  330},
  {  800,  310},
  {  900,   290},
  {  1000,   280},
  {  1100,   270},
  {  1200,   260},
  {  1400,   250},
  {  1500,   240},
  {  1600,   230},
  {  1700,   220},
  {  1800,   210},
  {  1900,   200},
  
};
#define DEFAULT_ACCEL_TABLE_SIZE (sizeof(defaultAccelTable)/sizeof(*defaultAccelTable))
static uint16_t scaledAccelTable[DEFAULT_ACCEL_TABLE_SIZE][2] = {0,0};
int maxVel = defaultAccelTable[DEFAULT_ACCEL_TABLE_SIZE-1][0]; // last value in table.





void init_stepper(){

  pinMode(Z_DIR_PIN, OUTPUT);
  pinMode(Z_STEP_PIN, OUTPUT);

  toolPos = 0;
  stepsPerMM = motor_steps / lead_screw_pitch;


  zstepper.config.channel = RMT_CHANNEL_0;
  zstepper.config.stepPin = (gpio_num_t) Z_STEP_PIN;
  zstepper.config.dirPin = (gpio_num_t) Z_DIR_PIN;

  zstepper.init();


}
