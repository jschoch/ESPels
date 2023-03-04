#pragma once
#include "gear.h"
#include "log.h"

namespace GenStepper {
    struct Config{
        int dir = 0;
        const char* name;
        float lead_screw_pitch = 2;
        int spindle_encoder_resolution = 2400;
        int motor_steps = 800;
    };

    struct State{
        int currrent_z  = 0;
        int32_t stepPosition = 0;
        float factor = 0;
        float offset = 0; //add to stepPosition

        int32_t den; // should rarely change leadscrew or encoder resolution
        int32_t nom;
        Config c;
        Gear::State mygear;

        void setELSFactor(float pitch, bool recalculate_den = false){
            float oldPitch = mygear.pitch;
            mygear.pitch = pitch;
            
            if(recalculate_den){
                den = c.lead_screw_pitch * c.spindle_encoder_resolution;
            }
            nom = c.motor_steps * pitch;
            if(!mygear.setRatio(nom,den)){
                sprintf(el.buf,"Bad Ratio: Den: %d Nom: %d\n",nom,den);
                #ifndef UNIT_TEST
                el.error();
                #endif
                #ifdef UNIT_TEST
                printf("bork\n\n!!\n");
                #endif
                mygear.pitch = oldPitch;
                return;
            }
        }
        void setHobFactor(){

        }
        void init_gear(int64_t count){
            mygear.is_setting_dir = false;
            mygear.calc_jumps(count,true);
            mygear.jumps.last = mygear.jumps.prev;
            if(!mygear.setRatio(nom,den)){
                printf("disaster\n");
                throw "holy shit";
            }
        }

    };
    State init(const char * name){
        State state;
        state.c.name = name;
        state.nom = state.c.spindle_encoder_resolution * 0.1;
        state.den = (int)(state.c.lead_screw_pitch * state.c.motor_steps);
        state.init_gear(0); 
        return state;
        
    }

    bool checkRatio(){
        return false;
    }
}
