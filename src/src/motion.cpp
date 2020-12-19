#include <Arduino.h>
#include "motion.h"
#include "config.h"


hw_timer_t * timer2 = NULL;
volatile SemaphoreHandle_t timer2Semaphore;
portMUX_TYPE timer2Mux = portMUX_INITIALIZER_UNLOCKED;
int timer2tics = 2;

void IRAM_ATTR do_pos_feeding(volatile bool pos_feeding){
  if(pos_feeding){
    // evaluate done?

    // calculate jumps and delta

    // evaluate stops, no motion if motion would exceed stops

    // handle feeding_dir

    // handle dir change

    // issue step?
  }

}
void IRAM_ATTR onTimer2(){
  portENTER_CRITICAL_ISR(&timer2Mux);

  xstepper.step();

  do_pos_feeding(pos_feeding);
  portEXIT_CRITICAL_ISR(&timer2Mux);
  // Give a semaphore that we can check in the loop
  xSemaphoreGiveFromISR(timer2Semaphore, NULL);
  // It is safe to use digitalRead/Write here if you want to toggle an output

  
}


void init_motion(){
  setFactor();


  // Setup timer to check encoder and decide if steps are needed
  timer2Semaphore = xSemaphoreCreateBinary();

  // divisor 400 should run this loop at 200 khz and should be plenty for the encoder signals.
  // Assuming an encoder ppr of 1024 and a speed of 5000 RPM the encoder frequency shoudl be 
  // generating pulses at 85khz.  timerticks = 2 runs at 100khz.

  timer2 = timerBegin(1, 400, true);
  timerAttachInterrupt(timer2, &onTimer2, true);

  // wait in us
  timerAlarmWrite(timer2, timer2tics, true);
  timerAlarmEnable(timer2);
}