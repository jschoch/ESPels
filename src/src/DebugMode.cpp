#include <Arduino.h>
#include "config.h"
#include <yasm.h>



void debugState(){
    if(btn_yasm.isFirstRun()){
        display_mode = DEBUG_READY;
        web = true;
    }
    if(lbd.deb->rose()){
        
        Serial.println("jogging 10mm");
        
        //freeJog(10.0);
        if(!jogging){
            resetToolPos();
            // distance to jog in revolutions
            float turns = 10.0;
            // set pitch to 1.0
            pitch_menu = 11;
            // update factor
            feed_parameters();
            Serial.print("factor: ");
            Serial.println(String(factor,9));
            jog_steps = (spindle_encoder_resolution * turns) * factor;
            Serial.print("Joggin ");
            Serial.println(jog_steps );
            
            
            jogging = true;
            
        }
    } 
    if(sbd.deb->rose()){
        
        btn_yasm.next(startupState);
    }
    
}