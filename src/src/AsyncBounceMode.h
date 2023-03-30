#pragma once
//#include "yasm.h"
#include <Arduino.h>
#include <vFSM.h>
#include "yasm.h"
#include "Stepper.h"



#ifndef UNIT_TEST
#include <Ticker.h>
namespace vTck {
    struct O{
        static bool running = false;
        void startTimer(){
            printf("starting timer\n");
            running = true;
            asb_ticker.attach(mc.dwell,stopTimer);
        }
        void stopTimer(){
            printf("stopping timer\n");
            running = false;
            // set state for next move
        }
        bool timerRunning(){
            return running;
        }
    };
}
#else
namespace vTck {
    struct O{
        static bool running;
        static void startTimer(){
            printf("starting timer\n");
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
        pos_feeding = false;
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

        pos_feeding = true;
        return;
    }
    printf("async bounce move from N run\n");
    if(pos_feeding == false){
        printf("asyncbouncemovefrom done move, going back again");
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
        return;
    }
    printf("async bounce move to N run pos_feeding: %d\n",pos_feeding);
    if(pos_feeding == false){
        async_bounce_yasm.next(AsyncBounceDwellState,true);
    }
};

inline void AsyncBounceIdleState(){
    if( maybeStop())
        return;
    if(async_bounce_yasm.isFirstRun()){
        printf("asyncbounce idle first \n");
        return;
    }
    printf("async bounce idle....\n");
    if(async_bouncing == true){
        pos_feeding = true;
        async_bounce_yasm.next(AsyncBounceMoveToState,true);
    }
};






