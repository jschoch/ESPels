#pragma once
#include "gear.h"
#include "log.h"

// yuck
#ifdef UNIT_TEST
#include "mocks.h"
#else
#include "rmtStepper.h"
#endif

namespace GenStepper {
    struct Config{
        int dir = 0;
        const char* name;
        float lead_screw_pitch = lead_screw_pitch;
        int spindle_encoder_resolution = spindle_encoder_resolution;
        int native_steps = Z_NATIVE_STEPS_PER_REV;
        int microsteps = Z_MICROSTEPPING;
        int motor_steps = native_steps * microsteps;
    };

    struct State{
        int position = 0;
        int32_t stepPosition = 1;
        float factor = 0;
        float offset = 0; //add to stepPosition

        int32_t den; // should rarely change leadscrew or encoder resolution
        int32_t nom;

        Config c;
        Gear::State mygear;
        Log::Msg lm;
        rmtStepper::State stepper;
        

        inline bool setELSFactor(float pitch, bool recalculate_den = false){
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
            Serial.printf("Set ELS Factor pitch: %f nom: %d den: %d\n",pitch,nom,den);
            return true;
        }
        inline void setHobFactor(){

        }
        inline bool init_gear(int64_t count){
            mygear.is_setting_dir = false;
            printf("init_gear count was: %lld \n",count);
            if(mygear.setRatio(nom,den)){
                mygear.calc_jumps(count);
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

        inline void stepPos(){
            stepper.step();
            position++;


        }
        inline void stepNeg(){
            stepper.step();
            position--;

        }
        inline void step(){
            if( stepper.dir){
                stepPos();
            }else{
                stepNeg();
            }
        }
        inline int currentPosition(){
            return position + offset;
        }

    };
    inline State init(const char * name,Log::Msg lm){
        State state;
        state.c.name = name;
        state.lm = lm;
        state.nom = state.c.spindle_encoder_resolution * 0.1;
        state.den = (int)(state.c.lead_screw_pitch * state.c.motor_steps);
        state.init_gear(0); 
        return state;
        
    }

    inline bool checkRatio(){
        return false;
    }
}
