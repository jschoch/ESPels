
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


int microsteps = Z_MICROSTEPPING;

int native_steps = Z_NATIVE_STEPS_PER_REV;
int motor_steps = Z_MICROSTEPPING * Z_NATIVE_STEPS_PER_REV;



// this is the main flag for signaling movement to begin
volatile bool pos_feeding = false;



#define TIMER_F 1000000ULL
#define TICK_PER_S TIMER_F

// trying anythign at this poitn
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;



//volatile int32_t stepsDelta = 0;
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
volatile bool stepTimerIsRunning = false;


void IRAM_ATTR stepTimerISR(){
   if(stepTimerIsRunning){
        gs.step();
    } 
}



void IRAM_ATTR accelTimerCallback(void *par){
    if(stepTimerIsRunning){
        frequency = updateSpeed(&gs);
        //if(frequency != last_frequency){
            setStepFrequency(frequency);
            last_frequency = frequency;
        //}
    }
}

void startStepperTimer(int32_t initial_speed){
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
        //timerAlarmDisable(stepTimer);
    }
}

void startAccelTimer(){
    /*
    */
}

bool initStepperTimer(){

    // STepper timer
    stepTimerIsRunning = false; 
    stepTimer = timerBegin(0, 80, true);
    timerAttachInterrupt(stepTimer, &stepTimerISR, false);
    timerAlarmEnable(stepTimer);


    
    // trying the other api

    const esp_timer_create_args_t periodic_timer_args = {
            .callback = &accelTimerCallback,
            /* name is optional, but may help identify the timer when debugging */
            .name = "periodic"
    };

    esp_timer_handle_t periodic_timer;
    ESP_ERROR_CHECK(esp_timer_create(&periodic_timer_args, &periodic_timer));
    /* The timer has been created but is not running yet */
     ESP_ERROR_CHECK(esp_timer_start_periodic(periodic_timer, 5000));


return true;
}
