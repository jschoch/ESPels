#include <Arduino.h>
#include "config.h"
#include <yasm.h>

void HobReadyState(){
        updateMode(RunMode::HOB_READY);
        Serial.println("Enter HobReadyState");
}

void HobRunState(){
        Serial.println("Enter HobRunState");
        updateMode(RunMode::HOB_RUN);
        // Turn off stops, we just slave to the spindle
        useStops = false;
        // flag processor and encoder to start
        jogging = true;
        updateStateDoc();
        init_pos_feed();
}

void HobStopState(){
        useStops = true;
        jogging = false;
        feeding = false;
        pos_feeding = false;
        HobReadyState();
}
