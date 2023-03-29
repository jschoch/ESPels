#pragma once
//#include "yasm.h"
#include <Arduino.h>
#include "yasm.h"

extern YASM async_bounce_yasm;

extern volatile bool async_bouncing;

void yasm_run_async_bounce();
void AsyncBounceMoveState(){
    if(async_bounce_yasm.isFirstRun()){
        printf("firstrun move state");
        return;
    }
    else {
        Serial.println("Move Done, starting rapid");
    };
    }
void AsyncBounceIdleState(){
    if(async_bounce_yasm.isFirstRun()){
        printf("asyncbounce idle first \n");
        return;
    }
    printf("async bounce idle....\n");
};
void AsyncBounceRapidState();
