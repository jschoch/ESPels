#pragma once
//#include "yasm.h"
#include <Arduino.h>
#include <vFSM.h>
#include "yasm.h"
#include "Stepper.h"
#include "state.h"
#include "moveConfig.h"



#ifndef UNIT_TEST
#include <Ticker.h>
extern Ticker vTcker;
namespace vTck {
    struct O{
        static bool running;
        static void startTimer(){
            printf("starting timer for: %i seconds\n",mc.dwell);
            running = true;
            // attach is float seconds
            // attach_ms is uint32
            vTcker.attach_ms(mc.dwell,stopTimer);
        }
        static void stopTimer(){
            printf("stopping timer\n");
            running = false;
            // set state for next move
        }
        static bool timerRunning(){
            return running;
        }
    };
}
#else
namespace vTck {
    struct O{
        static bool running;
        static void startTimer(){
            printf("starting timer for %i seconds\n",mc.dwell);
            vTck::O::stopTimer();
            return;
        }
        static void stopTimer(){
            vTck::O::running = false;
            printf("stopping timer\n");
            return;
        }
        static bool timerRunning(){
            return vTck::O::running;
        }
    };
}

#endif

extern YASM async_bounce_yasm;

extern volatile bool async_bouncing;

void AsyncBounceMoveToState();
void AsyncBounceIdleState();

inline bool maybeStop(){
    if(!async_bouncing){
        printf("stopping async move now\n");
        stepTimerIsRunning = false;
        async_bounce_yasm.next(AsyncBounceIdleState,true);
        return true;
    }
    return false;
}

inline void AsyncBounceMoveFromState(){
    if( maybeStop())
        return;
    if(async_bounce_yasm.isFirstRun()){
        printf("async bounce move from first run\n");
        // setup mc to go back
        printf("moving %i steps at %i speed",mc.moveDistanceSteps,mc.moveSpeed);


        // set diorection
        bool z_dir = gs.zstepper.setDir(!mc.moveDirection);
        printf("z_dir was: %d\n",z_dir);
        int32_t initial_speed = prepareMovement(gs.position, mc.moveDistanceSteps, mc.moveSpeed, 100,100,mc.accel);
        Serial.printf("Async FROM step test start: distance in steps: %i, initial speed: %i\n",mc.moveDistanceSteps,initial_speed);
        Serial.printf("accel: %i speed: %i\n ",mc.accel,mc.moveSpeed);
        //setStepFrequency(initial_speed);
        startStepperTimer(initial_speed);
        return;
    }
    printf("async bounce move from N run\n");
    if(stepTimerIsRunning == false){
        printf("asyncbouncemovefrom done move, going back again");
        printf("shouldn't I dwell here?");
        async_bounce_yasm.next(AsyncBounceMoveToState,true);
    }
};

inline void AsyncBounceDwellState(){
    if( maybeStop())
        return;
    if(async_bounce_yasm.isFirstRun()){
        printf("async bounce dwell first run\n");
        vTck::O::startTimer();
        return;
    }
    printf("async bounce dwell N run %d\n",vTck::O::timerRunning());
    if(!vTck::O::timerRunning()){
        printf("timer done, moving back\n");
        async_bounce_yasm.next(AsyncBounceMoveFromState,true);
    }
};

inline void AsyncBounceMoveToState(){
    if( maybeStop())
        return;
    if(async_bounce_yasm.isFirstRun()){
        printf("async bounce move to first run\n");

        // setup

        printf("moving %i steps at %i speed",mc.moveDistanceSteps,mc.moveSpeed);
        // set direction?
        // set diorection
        bool z_dir = gs.zstepper.setDir(mc.moveDirection);
        int32_t initial_speed = prepareMovement(gs.position, mc.moveDistanceSteps, mc.moveSpeed, 100,100,mc.accel);
        Serial.printf("Async step test start: distance in steps: %i, initial speed: %i\n",mc.moveDistanceSteps,initial_speed);
        Serial.printf("accel: %i speed: %i\n ",mc.accel,mc.moveSpeed);
        //setStepFrequency(initial_speed);
        startStepperTimer(initial_speed);

        return;
    }
    //printf("async bounce move to N run pos_feeding: %d\n",pos_feeding);
    if(stepTimerIsRunning == false){
        async_bounce_yasm.next(AsyncBounceDwellState,true);
    }
};

inline void AsyncBounceIdleState(){
    if(async_bounce_yasm.isFirstRun()){
        printf("asyncbounce idle first \n");
        return;
    }
    if(async_bouncing == true){
        async_bounce_yasm.next(AsyncBounceMoveToState,true);
    }
};






