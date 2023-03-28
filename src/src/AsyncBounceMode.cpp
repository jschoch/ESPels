#include <Arduino.h>
#include "myyasm.h"
#include "genStepper.h"
#include "moveConfig.h"

YASM async_bounce_yasm;
volatile bool async_bouncing = false;

Neotimer astate_timer(200);

void yasm_run_async_bounce(){
    if(astate_timer.repeat()){
        bounce_yasm.run();
    }
    
}

