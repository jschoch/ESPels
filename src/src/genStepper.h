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
        int32_t stepPosition = 1;
        float factor = 0;
        float offset = 0; //add to stepPosition

        int32_t den; // should rarely change leadscrew or encoder resolution
        int32_t nom;
        Config c;
        Gear::State mygear;
        Log::Msg lm;

        bool setELSFactor(float pitch, bool recalculate_den = false){
            if(pitch == 0){
                sprintf(lm.buf,"Pitch 0, no good");
                //lm.error();
                return false;
            }
            float oldPitch = mygear.pitch;
            mygear.pitch = pitch;
            
            if(recalculate_den){
                den = c.lead_screw_pitch * c.spindle_encoder_resolution;
            }
            nom = c.motor_steps * pitch;
            if(!mygear.setRatio(nom,den)){
                sprintf(lm.buf,"Bad Ratio: Den: %d Nom: %d\n",nom,den);
                //lm.error();
                mygear.pitch = oldPitch;
                return false;
            }
            return true;
        }
        void setHobFactor(){

        }
        bool init_gear(int64_t count){
            mygear.is_setting_dir = false;
            printf("init_gear count was: %d \n",count);
            if(mygear.setRatio(nom,den)){
                mygear.calc_jumps(count,true);
                mygear.jumps.last = mygear.jumps.prev;
                if(!mygear.setRatio(nom,den)){
                    printf("disaster\n");
                    throw "holy shit";
                }
                return true;
            }else{
                printf("init_gear setRatio failed");
                return false;
            }
            
        }

    };
    State init(const char * name,Log::Msg lm){
        State state;
        state.c.name = name;
        state.lm = lm;
        state.nom = state.c.spindle_encoder_resolution * 0.1;
        state.den = (int)(state.c.lead_screw_pitch * state.c.motor_steps);
        state.init_gear(0); 
        return state;
        
    }

    bool checkRatio(){
        return false;
    }
}
