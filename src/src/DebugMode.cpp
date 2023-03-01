#include <Arduino.h>
#include "config.h"
#include <yasm.h>



void debugState(){
    if(btn_yasm.isFirstRun()){
        updateMode(RunMode::DEBUG_READY);
        web = true;
    }
    
}
