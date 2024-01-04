#pragma once
#include "gear.h"
#include <elslog.h>

// yuck
#ifdef UNIT_TEST
#include "mocks.h"
#else
#include "rmtStepper.h"

// use fast accel stepper

#endif // unit_test





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
        Log::Msg *lm;
        static rmtStepper::State zstepper;
        

        // if returns true it worked
        inline bool setELSFactor(double pitch, bool recalculate_den = false){
            int old_nom = nom;
            if(pitch == 0){
                sprintf(lm->buf,"Pitch 0, no good");
                lm->error();
                return false;
            }
            
            if(recalculate_den == true){
                GenStepper::State::den = c.lead_screw_pitch * c.spindle_encoder_resolution;
                printf("recalculated denominator: %i\n",den);
            }
            GenStepper::State::nom = c.motor_steps * pitch;
            if(nom == 0 || den == 0){
                nom = old_nom;
                sprintf(lm->buf,"Bad Config? Bad Ratio: Den: %d Nom: %d\n",den,nom);
                lm->error();
                return false;
            }
            if(!mygear.setRatio(nom,den)){
                nom = old_nom;
                sprintf(lm->buf,"Bad Ratio: Den: %d Nom: %d\n",den,nom);
                lm->error();
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
                sprintf(lm->buf,"\n\n\n\tinit_gear setRatio failed: count: %lld nom: %i den: %i",count,nom,den);
                lm->error();
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

        inline void setAccel(int val){
        }
        inline void step(){
            if( zstepper.dir){
                stepPos();
            }else{
                stepNeg();
            }
        }
        inline int maxPitch(){
            return den;
        }
        inline bool validPitch(double pitch){
            int stepsPerMM = c.motor_steps / c.lead_screw_pitch;
            int pitchInSteps = stepsPerMM * pitch;
            int max =  den;
            printf("steps per mm: %i pitch in steps: %i max pitch was: %i\n",stepsPerMM,pitchInSteps,max);
            if(max > pitchInSteps){
                return true;
            }
            return false;
        }

    };
    inline State init(const char * name,Log::Msg *lm,Config c){
        double init_pitch = 0.1;
        State state;
        state.position = 0;
        state.c = c;
        state.c.name = name;
        state.lm = lm;
        state.nom = c.spindle_encoder_resolution * init_pitch;
        state.den = (int)(c.lead_screw_pitch / c.motor_steps);
         
        state.setELSFactor(init_pitch,true);
        state.init_gear(0);
        // TODO:  need sane defaults for these vars
        //int stepper_speed = 80000;
        //int accel = 50000;
#ifndef UNIT_TEST
        pinMode(Z_DIR_PIN, OUTPUT);
        pinMode(Z_STEP_PIN, OUTPUT);

        //stepsPerMM = c.motor_steps / c.lead_screw_pitch;


        state.zstepper.config.channel = RMT_CHANNEL_0;
        state.zstepper.config.stepPin = (gpio_num_t) Z_STEP_PIN;
        state.zstepper.config.dirPin = (gpio_num_t) Z_DIR_PIN;

        state.zstepper.init();
#endif
        return state;
        
    }

    
}
