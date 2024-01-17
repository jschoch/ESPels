#pragma once
#ifndef _Stepperr_h
#define _Stepperr_h

#include <Arduino.h>
#include "genStepper.h"

#ifndef UNIT_TEST

#endif

extern volatile bool pos_feeding;

// TODO: seems like this could be done better
extern int32_t move_start_position_0;
extern int32_t move_distance;



extern uint32_t vstart, vend, vtarget;
extern int64_t vs_sqr, ve_sqr, vt_sqr;
extern uint32_t two_a;
extern int32_t accEnd, decStart;
extern  volatile int32_t stepsDelta;
extern volatile int64_t alarm_value;
extern volatile bool stepTimerIsRunning;

void startStepperTimer(int32_t initial_speed);
void stopStepperTimer();

#ifndef UNIT_TEST
void IRAM_ATTR setStepFrequency(int32_t f);
#else
void setStepFrequency(int32_t f);
void startStepperTimer(int x){};
#endif
bool initStepperTimer();
void startAccelTimer();

typedef enum AccelState{
    ACCEL_OFF= 0,
    ACCELERATING = 1,
    COASTING = 2,
    DECELERATING = 3,
} AccelState;
extern AccelState accelState;

inline int32_t prepareMovement(int32_t currentPos, int32_t targetPos, uint32_t targetSpeed, uint32_t pullInSpeed, uint32_t pullOutSpeed, uint32_t accel)
{
    vtarget    = targetSpeed;   
    vstart    = pullInSpeed;  // v_start
    vend    = pullOutSpeed; // v_end
    two_a = 2 * accel;

    // reset starting zero position
    move_start_position_0 = currentPos;
    move_distance = abs(targetPos);

    vs_sqr = (int32_t)vstart * vstart;
    ve_sqr = (int32_t)vend * vend;
    vt_sqr = (int32_t)vtarget * vtarget;


    int32_t sm = ((ve_sqr - vs_sqr) / two_a + move_distance) / 2; // position where acc and dec curves meet

    printf("vend: %u\n", vend);
    printf("vstart: %u\n", vstart);
    printf("move_distance: %d\n", move_distance);
    printf("sm: %i\n", sm);

    if (sm >= 0 && sm <= move_distance) // we can directly reach the target with the given values vor v0, vend and a
    {
        int32_t sa = (vt_sqr - vs_sqr) / two_a; // required distance to reach target speed
        if (sa < sm)                            // target speed can be reached
        {
            accEnd   = sa;
            decStart = sm + (sm - sa);
            printf("reachable accEnd: %i decStart:%i\n", accEnd, decStart);
        }
        else
        {
            accEnd = decStart = sm;
            printf("limit accEnd: %i decStart:%i\n", accEnd, decStart);
        }
    }
    else
    {
        // TODO: hack, call some error callback instead
        printf("ERROR\n");
    }




    return vstart;
}


#ifdef UNIT_TEST
inline int32_t updateSpeed(GenStepper::State *gs)
#else
inline IRAM_ATTR int32_t updateSpeed(GenStepper::State *gs)
#endif
{

    stepsDelta = abs(move_start_position_0 - gs->position);


    // Initially stepsDelta is zero, and we early return in each conditional
    // as stepsDelta grows due the the stepper timer running along at 
    // the calculated frequency

    // acceleration phase -------------------------------------
    if (stepsDelta < accEnd)
    {
        if(accelState != ACCELERATING){
            accelState = ACCELERATING;
        }
        return sqrt((double)two_a * stepsDelta + vs_sqr);
    }

    // constant speed phase ------------------------------------
    if (stepsDelta < decStart)
    {
        if(accelState != COASTING){
            accelState = COASTING;
        }
        return vtarget;
    }

    //deceleration phase --------------------------------------
    if (stepsDelta < move_distance)
    {
        if(accelState != DECELERATING){
            accelState = DECELERATING;
        }
        return sqrt((double)two_a * (move_distance - stepsDelta - 1) + ve_sqr);
    }

    //we are done, clean up
    accelState = ACCEL_OFF;
    stepTimerIsRunning = false;
    stepsDelta = 0;
    return 0;
}


#endif
