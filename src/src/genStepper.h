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
        int dir ;
        const char* name;
        float lead_screw_pitch ;
        int spindle_encoder_resolution ;
        int native_steps ;
        int microsteps ;
        int motor_steps ;
    };

    struct State{
        static int position;
        float factor = 0;
        float offset = 0; //add to stepPosition

        static int32_t den; // should rarely change leadscrew or encoder resolution
        static int32_t nom;

        Config c;
        static Gear::State mygear;
        Log::Msg lm;
        static rmtStepper::State zstepper;
        

        // if returns true it worked
        inline bool setELSFactor(double pitch, bool recalculate_den = false){
            if(pitch == 0){
                sprintf(lm.buf,"Pitch 0, no good");
                //lm.error();
                return false;
            }
            
            if(recalculate_den){
                den = c.lead_screw_pitch * c.spindle_encoder_resolution;
            }
            nom = c.motor_steps * pitch;
            if(!mygear.setRatio(nom,den)){
                sprintf(lm.buf,"Bad Ratio: Den: %d Nom: %d\n",nom,den);
                //lm.error();
                return false;
            }
            printf("Set ELS Factor pitch: %f nom: %d den: %d\n",pitch,nom,den);
            return true;
        }
        inline void setHobFactor(){

        }
        inline bool init_gear(int64_t count){
            //printf("init_gear count was: %lld \n",count);
            if(mygear.setRatio(nom,den)){
                mygear.calc_jumps(count);
                mygear.last = mygear.prev;
                return true;
            }else{
                printf("\n\n\n\tinit_gear setRatio failed: count: %lld nom: %i den: %i",count,nom,den);
                return false;
            }
            
        }

        inline void stepPos(){
            zstepper.step();
            position++;
        }
        inline void stepNeg(){
            zstepper.step();
            position--;

        }
        inline void step(){
            if( zstepper.dir){
                stepPos();
            }else{
                stepNeg();
            }
        }

    };
    inline State init(const char * name,Log::Msg lm,Config c){
        double init_pitch = 0.1;
        State state;
        state.position = 0;
        state.c = c;
        state.c.name = name;
        state.lm = lm;
        state.nom = state.c.spindle_encoder_resolution * init_pitch;
        state.den = (int)(state.c.lead_screw_pitch / state.c.motor_steps);
         
        state.setELSFactor(init_pitch,true);
        state.init_gear(0);
        return state;
        
    }

    inline bool checkRatio(){
        return false;
    }
}
