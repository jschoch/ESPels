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
volatile int32_t delta = 0;


volatile int delay_ticks = 3;
volatile int previous_delay_ticks = 0;
volatile int min_delay_ticks = 5;
volatile bool z_dir = true; //CW
volatile bool z_prev_dir = true;
volatile bool z_moving = false;

int z_step_pin = 13;
int z_dir_pin = 12;

int use_limit = false;

bool getDir(){
  return z_dir;
}
void setDir(bool d){
  
  if (d != z_dir ){
    z_dir = d;
    digitalWrite(z_dir_pin, d); 
  }
}

void stepLeft(){

}

void IRAM_ATTR onTimer(){
  // Increment the counter and set the time of ISR
  portENTER_CRITICAL_ISR(&timerMux);

  if(feeding){
    if(delay_ticks == 0 && z_pause == true){
      z_pause = false;
      z_moving = false;
    }

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
      setDir(true);
      digitalWrite(z_step_pin, HIGH);
      if(feeding_dir){
         toolPos--; 
      }else{
          toolPos++;  
      }
      
      z_moving = true;
    }  

    if(delta < 0 && z_moving == false){
      // delta < 0 means we need to set dir to false
      setDir(false);
      digitalWrite(z_step_pin, HIGH);
      
      if(feeding_dir){
         toolPos++; 
      }else{
          toolPos--;  
      }
      z_moving = true;
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


  // setup a timer to handle stepper pulses
  timerSemaphore = xSemaphoreCreateBinary();
  timer = timerBegin(0, 80, true);
  timerAttachInterrupt(timer, &onTimer, true);

  // wait in us
  timerAlarmWrite(timer, timertics, true);
  timerAlarmEnable(timer);

}