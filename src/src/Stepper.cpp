
#include "Arduino.h"
#include "Stepper.h"
#include "state.h"
#include <iostream>

#include <algorithm>
#include <cmath>
#include <cstdint>
#include "driver/timer.h"
#include <esp_log.h>


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
int32_t s_0 = 0;
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

bool IRAM_ATTR stepperTimerISR(void* par){
     BaseType_t high_task_awoken = pdFALSE;
    // stuff
    // figure out accel timer, how fast to run it? frequency 
    if(stepsDelta > 0){
        gs.step();
    }
    return high_task_awoken == pdTRUE;
}

bool IRAM_ATTR accelTimerISR(void * par){
    BaseType_t high_task_awoken = pdFALSE;
    /*
    if(stepsDelta > 0){

        //setStepFrequency(updateSpeed(&gs));
    }
    */

    return high_task_awoken == pdTRUE;
}

void startStepperTimer(){
    timer_start(TIMER_GROUP_0,TIMER_0);
}

void stopStepperTimer(){
    timer_pause(TIMER_GROUP_0, TIMER_0);
}

void IRAM_ATTR setStepFrequency(int32_t f)
{
   //timer_spinlock_take(TIMER_GROUP_0);
   uint64_t alarm_value = (uint64_t)f;
   timer_pause(TIMER_GROUP_0,TIMER_0);
   timer_set_alarm_value(TIMER_GROUP_0, TIMER_0,alarm_value);
   timer_set_alarm(TIMER_GROUP_0,TIMER_0,TIMER_ALARM_EN);
   //timer_set_alarm
   timer_start(TIMER_GROUP_0,TIMER_0);
   //timer_spinlock_give(TIMER_GROUP_0);
}

void startAccelTimer(){
    //timer_spinlock_take(TIMER_GROUP_1);
    timer_set_alarm_value(TIMER_GROUP_0, TIMER_0,1);
    timer_set_alarm(TIMER_GROUP_1,TIMER_1,TIMER_ALARM_EN);
    timer_start(TIMER_GROUP_1,TIMER_1);
    //timer_spinlock_give(TIMER_GROUP_1);
}

bool initStepperTimer(){

    /*
    uint64_t timer_val = timerRead(timer);
    uint16_t div = timerGetDivider(timer);
    return timer_val * div / (getApbFrequency() / 1000000); 
    */
    timer_config_t timer_conf = {
        .alarm_en = TIMER_ALARM_EN,         // we need alarm
        .counter_en = TIMER_PAUSE,          // dont start now lol
        .intr_type = TIMER_INTR_LEVEL,      // interrupt
        .counter_dir = TIMER_COUNT_UP,      // count up duh
        .auto_reload = TIMER_AUTORELOAD_EN, // reload pls
        //.divider = 80000000ULL / TIMER_F,   // ns resolution
        //.divider = 1000 * (getApbFrequency() / 1000000) 
        .divider = 80
    };


    timer_config_t accel_timer_conf = {
        .alarm_en = TIMER_ALARM_EN,         // we need alarm
        .counter_en = TIMER_START,          // dont start now lol
        .intr_type = TIMER_INTR_LEVEL,      // interrupt
        .counter_dir = TIMER_COUNT_UP,      // count up duh
        .auto_reload = TIMER_AUTORELOAD_EN, // reload pls
        .divider = 360
    };



    ESP_ERROR_CHECK(timer_init(TIMER_GROUP_0, TIMER_0, &timer_conf));                   // init the timer
    ESP_ERROR_CHECK(timer_set_counter_value(TIMER_GROUP_0, TIMER_0, 0));                // set it to 0
    ESP_ERROR_CHECK(timer_isr_callback_add(TIMER_GROUP_0, TIMER_0, stepperTimerISR, NULL, ESP_INTR_FLAG_IRAM)); // add callback fn to run when alarm is triggrd

    //  Acceleration timer
    /*

    ESP_ERROR_CHECK(timer_init(TIMER_GROUP_1, TIMER_1, &accel_timer_conf));                   // init the timer
    ESP_ERROR_CHECK(timer_set_counter_value(TIMER_GROUP_1, TIMER_1, 0));                // set it to 0
    ESP_ERROR_CHECK(timer_set_alarm_value(TIMER_GROUP_1,TIMER_0,10));
    //ESP_ERROR_CHECK(timer_isr_callback_add(TIMER_GROUP_1, TIMER_1, accelTimerISR, NULL, ESP_INTR_FLAG_IRAM)); // add callback fn to run when alarm is triggrd
    ESP_ERROR_CHECK(timer_isr_callback_add(TIMER_GROUP_1, TIMER_1, accelTimerISR, NULL, 0)); // add callback fn to run when alarm is triggrd
    */
    
 

return true;
}
