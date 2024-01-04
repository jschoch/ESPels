
#include "Arduino.h"
#include "Stepper.h"
#include "state.h"
#include <iostream>

#include <algorithm>
#include <cmath>
#include <cstdint>
#include "driver/timer.h"
#include <esp_log.h>
#include "esp_timer.h"
#include <functional>
#include "motion.h"





// this is the main flag for signaling movement to begin
volatile bool pos_feeding = false;



#define TIMER_F 1000000ULL
#define TICK_PER_S TIMER_F

// trying anythign at this poitn
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;


// Much of this is derrived from the 
volatile int32_t oldSpeed = 1;
volatile int32_t theSpeed = 0;

int32_t move_start_position_0 = 0;
int32_t move_distance = 0;


uint32_t vstart = 0;
uint32_t vend = 0;
uint32_t vtarget = 0;
int64_t vs_sqr = 0;
int64_t ve_sqr = 0;
int64_t vt_sqr = 0;
uint32_t two_a = 0;
int32_t accEnd = 0;
int32_t  decStart = 0;
volatile int32_t stepsDelta = 0;
volatile int64_t alarm_value = 0;
volatile int32_t last_frequency = 0;
volatile int32_t frequency = 1;
AccelState accelState = ACCEL_OFF;

	
hw_timer_t * stepTimer = NULL;
hw_timer_t * accelTimer = NULL;
volatile bool stepTimerIsRunning = false;
int64_t last_pulse_count = 0;


void IRAM_ATTR stepTimerISR(){
   if(stepTimerIsRunning){
        gs.step();
    } 
    else{

#ifdef MOTION_MODE_TIMER
        if(last_pulse_count != encoder.getCount()){
            processMotion();
            last_pulse_count = encoder.getCount();
        }
#endif

    }
}



//void IRAM_ATTR accelTimerCallback(void *par){
void IRAM_ATTR accelTimerCallback(){
    if(stepTimerIsRunning){
        frequency = updateSpeed(&gs);
        if(frequency != last_frequency){
            setStepFrequency(frequency);
            last_frequency = frequency;
        }
    }
}

void startStepperTimer(int32_t initial_speed){
    if(!timerStarted(stepTimer)){
        Serial.println("Step timer not started, starting....");
        timerStart(stepTimer);
    }
    if(!timerAlarmEnabled(stepTimer)){
        Serial.println("\nStepper timer was not enabled, this seems like an error");
        timerAlarmEnable(stepTimer);
    }
    stepTimerIsRunning = true;
    setStepFrequency(initial_speed);

}

void stopStepperTimer(){
    timerAlarmDisable(stepTimer);
    stepTimerIsRunning = false;
}

void IRAM_ATTR setStepFrequency(int32_t f)
{
    if(f != 0){
        alarm_value = (uint64_t)1000000 / f;
        timerAlarmWrite(stepTimer, alarm_value,true);
    }else{
        stepTimerIsRunning = false;
    }
}


bool initStepperTimer(){

    Serial.println("\nStepper timer starting");
    // STepper timer
    stepTimerIsRunning = false; 
    stepTimer = timerBegin(0,80,true);
    timerAttachInterrupt(stepTimer, &stepTimerISR, false);
    
    timerAlarmWrite(stepTimer, 4,true);
    timerStart(stepTimer);
    timerAlarmEnable(stepTimer);

    //stepTimerIsRunning = false; 
    accelTimer = timerBegin(1,480,true);
    timerAttachInterrupt(accelTimer, &accelTimerCallback, false);
    
    timerAlarmWrite(accelTimer, 1000,true);
    timerStart(accelTimer);
    timerAlarmEnable(accelTimer);
    Serial.println("\nStepper timer init complete");


return true;
}
