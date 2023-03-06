#pragma once
#include <Arduino.h>
#include <esp_log.h>

static const char* TAGmc = "Mc";

namespace MoveConfig{

    struct State{
        // was jog_mm
        int32_t moveDistanceSteps = 0;
        bool waitForSync = true;
        // was z_feeding_direction
        bool moveDirection = true;
        // was targetToolRelPosMM
        int32_t moveSyncTarget = 0;
        int stopPos = 0;
        int stopNeg = 0;
        // spindle spinning CW or CCW
        bool spindle_handedness = true;
        float pitch = 0;
        float rapidPitch = 0;
        float oldPitch = 0;
        bool syncMoveStart = true;
        bool isAbs = false;
        bool useStops = true;

        // returns a bool to be used by stepper.setDir
        inline bool setStops(int32_t current_position){
           if(isAbs){
             moveSyncTarget = moveDistanceSteps;
             ESP_LOGE(TAGmc,"Absolute move:\n");
           }else {
            ESP_LOGE(TAGmc,"Relative move: \n");
             moveSyncTarget =  current_position + moveDistanceSteps;
           }
           if( moveDistanceSteps < 0){
            ESP_LOGE(TAGmc,"Negative move, setting moveDirection and stops:");
             moveDirection = false;
             stopNeg = moveSyncTarget;
             stopPos = current_position;
             return false;
           } else{
             moveDirection = true;
             stopPos = moveSyncTarget;
             stopNeg = current_position;
             return true;
           }
        }
        

    };
    inline State init(){
        State state;
        return state;
    }
}
