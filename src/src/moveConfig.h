#pragma once
#include <Arduino.h>

// breaks tests
//#include <esp_log.h>

static const char* TAGmc = "Mc";


namespace MoveConfig{

    struct State{
        // was jog_mm
        int default_accel = 200000;
        static int32_t moveDistanceSteps ;
        static bool waitForSync ;
        // was z_feeding_direction
        static bool moveDirection ;

        static int stopPos ;
        static int stopNeg ;
        // spindle spinning CW or CCW
        static bool spindle_handedness ;
        static double pitch ;
        static double rapidPitch ;
        static double oldPitch ;
        //static bool syncMoveStart ;
        //static bool isAbs  ;
        static bool useStops ;
        static int moveTargetSteps;
        static int accel;

        // returns a bool to be used by stepper.setDir
        inline bool setStops(int32_t current_position){
           useStops = true;
           /*
           if(!isAbs){
             //moveSyncTarget = moveDistanceSteps;
             //ESP_LOGE(TAGmc,"Absolute move:\n");
           }else {
            //ESP_LOGE(TAGmc,"Relative move: \n");
             //moveSyncTarget =  current_position + moveDistanceSteps;
           }
           */
           if( moveDistanceSteps < 0){
            
             moveDirection = false;
             stopNeg = moveDistanceSteps + current_position;
             moveTargetSteps = stopNeg;
             stopPos = current_position;
             //ESP_LOGE(TAGmc,"Stops:\n\t Distance: %ld stopNeg: %ld stopPos: %ld, moveDirection: %d\n",moveDistanceSteps, stopNeg, stopPos, moveDirection);
             printf("Stops:\n\t Distance: %i stopNeg: %i stopPos: %i, moveDirection: %d\n",moveDistanceSteps, stopNeg, stopPos, moveDirection);
             return false;
           } else{
             moveDirection = true;
             stopPos = moveDistanceSteps + current_position;
             moveTargetSteps = stopPos;
             stopNeg = current_position;
             //ESP_LOGE(TAGmc,"Stops:\n\t Distance: %ld stopNeg: %ld stopPos: %ld, moveDirection: %d\n", moveDistanceSteps, stopNeg, stopPos, moveDirection);
             printf("Stops:\n\t Distance: %i stopNeg: %i stopPos: %i, moveDirection: %d\n",moveDistanceSteps, stopNeg, stopPos, moveDirection);

             return true;
           }
        }
        

    };
    inline State init(){
        State state;
        state.waitForSync = true;
        state.moveDirection = true;
        state.spindle_handedness = true;
        state.pitch = 0.1;
        state.rapidPitch = 0.1;
        state.oldPitch = state.pitch;
        //state.syncMoveStart = true;
        //state.isAbs = false;
        state.accel = state.default_accel;
        state.useStops = true;
        return state;
    }
}
