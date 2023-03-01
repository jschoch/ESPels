#include <Arduino.h>
#include "config.h"
#include <yasm.h>

void HobReadyState(){
    if(run_mode != RunMode::HOB_RUN){
        Serial.println("Enter HobReadyState");
    }else{
        el.error("Use HobStopState, invalid state transition");
    }
}

void HobRunState(){
        Serial.println("Enter HobRunState");
        
        // Turn off stops, we just slave to the spindle
        useStops = false;
        // flag processor and encoder to start
        jogging = true;
        init_hob_feed();
        updateMode(RunMode::HOB_RUN);
}

void HobStopState(){
        useStops = true;
        jogging = false;
        feeding = false;
        pos_feeding = false;
        updateMode(RunMode::HOB_STOP);
        HobReadyState();
        updateMode(RunMode::HOB_READY);
}
