#include <Arduino.h>
#include "config.h"
#include "yasm.h"
#include "web.h"
#include "controls.h"
#include "stepper.h"
#include "BounceMode.h"


void debugState(){
    if(bounce_yasm.isFirstRun()){
        updateMode(RunMode::DEBUG_READY);
        web = true;
    }
    
}
