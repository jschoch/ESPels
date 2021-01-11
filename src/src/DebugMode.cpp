#include <Arduino.h>
#include "config.h"
#include <yasm.h>



void debugState(){
    if(btn_yasm.isFirstRun()){
        updateMode(DEBUG_READY,RunMode::DEBUG_READY);
        web = true;
    }
    
}
