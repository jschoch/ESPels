#pragma once
#include <Arduino.h>

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
        bool syncMove = true;
        bool isAbs = false;

        // returns a bool to be used by stepper.setDir
        inline bool setStops(int32_t current_position){
           if(isAbs){
             moveSyncTarget = moveDistanceSteps;
           }else {
             moveSyncTarget =  current_position + moveDistanceSteps;
           }
           if( moveDistanceSteps < 0){
             moveDirection = false;
             stopNeg = moveSyncTarget;
             stopPos = current_position;
             return false;
           } else{
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
