
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


timer_group_t timer_group;
//timer_isr_t timer_idx;
timer_idx_t timer_idx;

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
    // stuff
    // figure out accel timer, how fast to run it? frequency 
    gs.zstepper.step();
    return true;
}

bool IRAM_ATTR accelTimerISR(void * par){


    setStepFrequency(updateSpeed(&gs));

    return true;
}

void startStepperTimer(){
    timer_start(timer_group,timer_idx);
}

void stopStepperTimer(){
    timer_pause(timer_group, timer_idx);
}

void IRAM_ATTR setStepFrequency(int32_t f)
{
   uint64_t alarm_value = (uint64_t)f;
   timer_pause(timer_group,timer_idx);
   timer_set_alarm_value(timer_group, timer_idx,alarm_value);
   timer_set_alarm(timer_group,timer_idx,TIMER_ALARM_EN);
   timer_start(timer_group,timer_idx);
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
        .counter_en = TIMER_PAUSE,          // dont start now lol
        .intr_type = TIMER_INTR_LEVEL,      // interrupt
        .counter_dir = TIMER_COUNT_UP,      // count up duh
        .auto_reload = TIMER_AUTORELOAD_EN, // reload pls
        .divider = 80
    };




    if (timer_group != TIMER_GROUP_MAX && timer_idx != TIMER_MAX)
    {
        // timer was configured explixitly in config structure, we dont need to do it here
        goto timer_avail;
    }

    // try to find free timer
    timer_config_t temp;
    for (int i = 0; i < 1; i++)
    {
        for (int j = 0; j < 1; j++)
        {
            printf("timer found: idx: %i j: %i",i,j);
            timer_get_config((timer_group_t)i, (timer_idx_t)j, &temp);
            if (temp.alarm_en == TIMER_ALARM_DIS)
            {
                // if the alarm is disabled, chances are no other dendostepper instance is using it
                timer_group = (timer_group_t)i;
                timer_idx = (timer_idx_t)j;
                goto timer_avail;
            }
        }
    }

    // if we got here it means that there isnt any free timer available
    printf("can't get a timer");
    return false;

timer_avail:
    ESP_ERROR_CHECK(timer_init(timer_group, timer_idx, &timer_conf));                   // init the timer
    ESP_ERROR_CHECK(timer_set_counter_value(timer_group, timer_idx, 0));                // set it to 0
    ESP_ERROR_CHECK(timer_isr_callback_add(timer_group, timer_idx, stepperTimerISR, NULL, ESP_INTR_FLAG_IRAM)); // add callback fn to run when alarm is triggrd


    ESP_ERROR_CHECK(timer_init(timer_group, TIMER_1, &accel_timer_conf));                   // init the timer
    ESP_ERROR_CHECK(timer_set_counter_value(timer_group, TIMER_1, 0));                // set it to 0
    ESP_ERROR_CHECK(timer_isr_callback_add(timer_group, TIMER_1, accelTimerISR, NULL, ESP_INTR_FLAG_IRAM)); // add callback fn to run when alarm is triggrd

return true;
}
