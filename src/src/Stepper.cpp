#include "Stepper.h"

/////////////////////////////////////////////////
// stepper timer stuff
//////////////////////////////////////////////////


hw_timer_t * timer = NULL;
volatile SemaphoreHandle_t timerSemaphore;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;
//volatile uint8_t current_accel = 20;
volatile bool z_pause = false;
//volatile bool synced = false;


// number of ticks to wait between timer events
int timertics = 10;

// used to figure out how many steps we need to get to the right position
// delta is in stepper steps
volatile int64_t delta = 0;

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
volatile int step_delta = 0;
volatile double toolRelPos = 0;
double toolRelPosMM = 0;
double oldToolRelPosMM = 0;

int z_step_pin = 13;
int z_dir_pin = 12;

int use_limit = false;
volatile int64_t calculated_stepper_pulses=0;

volatile bool jog_done = true;
volatile int32_t jog_steps = 0;
float jog_mm = 0;
volatile float jog_scaler = 0.2;
volatile uint16_t vel = 1;


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

bool getDir(){
  return z_dir;
}

// update dir, return true if we changed dir which requires a delay.
bool IRAM_ATTR setDir(bool d){
  
  if (d != z_dir ){
    z_dir = d;
    digitalWrite(z_dir_pin, d); 
    return true;
  }
  return false;
}

void init_jog(){
  vel = 1;
  for(int i = 0; i < DEFAULT_ACCEL_TABLE_SIZE; i++){
    Serial.print("table ");
    Serial.print(i);
    Serial.print(",");
    Serial.println((int)(defaultAccelTable[i][1] * jog_scaler));
    scaledAccelTable[i][0] = defaultAccelTable[i][0];
    scaledAccelTable[i][1] = (int) (defaultAccelTable[i][1] * jog_scaler);
  }
  maxVel = scaledAccelTable[DEFAULT_ACCEL_TABLE_SIZE-1][0]; // last value in table.
  z_moving = false;
  z_pause = false;
  jog_done = false;
  jog_delay_ticks = 0;
  jogs = 0;
  
}

void updatePosition(){
  relativePosition = toolPos * stepsPerMM;
  toolRelPosMM = toolRelPos / stepsPerMM;
  if(toolRelPosMM != oldToolRelPosMM){
    //toolRelPosMM = toolRelPos * stepsPerMM;
    Serial.print(toolRelPosMM);
    Serial.print("#");
    oldToolRelPosMM = toolRelPosMM;
    updateStatusDoc();
  }
  absolutePosition = encoder.getCount() * stepsPerMM;
}

void IRAM_ATTR calcAccel(){
  step_delta = abs( jog_steps - jogs);

  if(vel == 0 && !jog_done){
    vel = 1;
  }

  if (step_delta > 0) {
    // case 1 : moving towards target (maybe under accel or decel)
    if (step_delta < vel) {
      // time to declerate
      vel--;
    } else if (vel < maxVel) {
      // accelerating
      vel++;
    } else {
      // at full speed - stay there
    }
  }
  uint8_t idx = 0;
  while(scaledAccelTable[idx][0] < vel) {
    idx++;
  }
  jog_delay_ticks = scaledAccelTable[idx][1];
}


void IRAM_ATTR onTimer(){
  // Increment the counter and set the time of ISR
  portENTER_CRITICAL_ISR(&timerMux);

  if(feeding){

    // TODO figure out how to sync the spindle rotations
    
    // calculate the current position in stepper pulses by multiplying encoder position by the current factor
    calculated_stepper_pulses = (int64_t)(factor * encoder.getCount());

    // calculate the delta in motor steps between the current spindle position and the current motor position aka toolPos

    if(feeding_dir){
      delta = toolPos - calculated_stepper_pulses;   
    }else{
      delta = calculated_stepper_pulses - toolPos;
    }
    
    if(delay_ticks == 0 && z_pause == true){
      z_pause = false;
      z_moving = false;
    }

    ////     end calculate delta

    // delay a bit after stepping low.
    if(z_pause == true && delay_ticks > 0){
      delay_ticks--;      
      portEXIT_CRITICAL_ISR(&timerMux);
      xSemaphoreGiveFromISR(timerSemaphore, NULL);
      return;
    }


    // turn the pulse off if we were moving.
    if(z_moving == true){
      digitalWrite(z_step_pin, LOW);    
      z_pause = true;
      
      // figure out how long to delay
      //delay_ticks = tbl[speed];
    }

    

    // if the queue is not full and we are not currently making a signal
    if(delta > 0 && z_moving == false){
      // delta > 0 means we need to set dir to true
      if(!setDir(true)){
        digitalWrite(z_step_pin, HIGH);
        if(feeding_dir){
          toolRelPos--;
          toolPos--; 
        }else{
            toolRelPos++;
            toolPos++;  
        }
        
        z_moving = true;
      }
      
    }  

    if(delta < 0 && z_moving == false){
      // delta < 0 means we need to set dir to false
      if(!setDir(false)){
        digitalWrite(z_step_pin, HIGH);
      
        if(feeding_dir){
          toolRelPos++;
          toolPos++; 
        }else{
            toolRelPos--;
            toolPos--;  
        }
        z_moving = true;  
      }
      
    }
  }


  ///////////////////////////////// Free Jogging Mode

  // TODO: this needs some kind of lockout
  if(jogging && !jog_done){
      
      // if paused for accel

      if(jog_delay_ticks > 0 && z_moving == true && z_pause == true){
        jog_delay_ticks--;        
      }else if(jog_delay_ticks <= 0 && z_pause == true && z_moving == true){
        z_pause = false;
        z_moving = false;
      }

      // if steps > 0 && dir_set && moving

      else if(!setDir(feeding_dir) && z_moving == true){
        digitalWrite(z_step_pin, LOW);
        calcAccel();
        z_pause = true;
      }   
      
      // if steps > 0 && dir_set && !moving   

      else if(jogs <= jog_steps && !setDir(feeding_dir) && z_moving == false){
        digitalWrite(z_step_pin, HIGH);
        if(feeding_dir){
          toolRelPos--;
          toolPos--;
        }else{
          toolRelPos++;
          toolPos++;
        }
        jogs++;
        z_moving = true;
      }  

      if(jogs >= jog_steps && !z_moving){
        jogging = false;
        jog_done = true;
      }
      

      

      // if steps == 0
      
  }

  if(jogging && jog_done ){
    init_jog();
  }
  

  
  portEXIT_CRITICAL_ISR(&timerMux);
  // Give a semaphore that we can check in the loop
  xSemaphoreGiveFromISR(timerSemaphore, NULL);
  // It is safe to use digitalRead/Write here if you want to toggle an output
}

void init_stepper(){

  pinMode(z_dir_pin, OUTPUT);
  pinMode(z_step_pin, OUTPUT);

  toolPos = 0;
  stepsPerMM = motor_steps / lead_screw_pitch;
  // setup a timer to handle stepper pulses
  timerSemaphore = xSemaphoreCreateBinary();
  timer = timerBegin(0, 80, true);
  timerAttachInterrupt(timer, &onTimer, true);

  // wait in us
  timerAlarmWrite(timer, timertics, true);
  timerAlarmEnable(timer);

}