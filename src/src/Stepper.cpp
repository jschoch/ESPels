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


volatile int delay_ticks = 3;
volatile int previous_delay_ticks = 0;
volatile int min_delay_ticks = 5;
volatile bool z_dir = true; //CW
volatile bool z_prev_dir = true;
volatile bool z_moving = false;

int z_step_pin = 13;
int z_dir_pin = 12;

int use_limit = false;
volatile int32_t calculated_stepper_pulses=0;

bool getDir(){
  return z_dir;
}

// update dir, return true if we changed dir which requires a delay.
bool setDir(bool d){
  
  if (d != z_dir ){
    z_dir = d;
    digitalWrite(z_dir_pin, d); 
    return true;
  }
  return false;
}


void IRAM_ATTR onTimer(){
  // Increment the counter and set the time of ISR
  portENTER_CRITICAL_ISR(&timerMux);

  if(feeding){

    // calculate updated delta
    // TODO move delta out of motion.cpp
    
    // calculate the current position in stepper pulses by multiplying encoder position by the current factor
    calculated_stepper_pulses = (int32_t)(factor * encoder.getCount());

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
          toolPos--; 
        }else{
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
          toolPos++; 
        }else{
            toolPos--;  
        }
        z_moving = true;  
      }
      
    }
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
  // setup a timer to handle stepper pulses
  timerSemaphore = xSemaphoreCreateBinary();
  timer = timerBegin(0, 80, true);
  timerAttachInterrupt(timer, &onTimer, true);

  // wait in us
  timerAlarmWrite(timer, timertics, true);
  timerAlarmEnable(timer);

}