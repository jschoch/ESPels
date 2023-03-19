#pragma once
#include "gear.h"
#include <elslog.h>

// yuck
#ifdef UNIT_TEST
#include "mocks.h"
#else
#include "rmtStepper.h"
#define SUPPORT_ESP32_RMT
#include "FastAccelStepper.h"
#endif

// use fast accel stepper
//#define useFAS




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
        FastAccelStepper *fzstepper = NULL;
        FastAccelStepperEngine engine = FastAccelStepperEngine();
        static bool diduseFAS ;
        

        // if returns true it worked
        inline bool setELSFactor(double pitch, bool recalculate_den = false){
            int old_nom = nom;
            if(pitch == 0){
                sprintf(lm.buf,"Pitch 0, no good");
                lm.error();
                return false;
            }
            
            if(recalculate_den == true){
                GenStepper::State::den = c.lead_screw_pitch * c.spindle_encoder_resolution;
                printf("recalculated denominator: %i\n",den);
            }
            GenStepper::State::nom = c.motor_steps * pitch;
            if(nom == 0 || den == 0){
                nom = old_nom;
                sprintf(lm.buf,"Bad Config? Bad Ratio: Den: %d Nom: %d\n",den,nom);
                lm.error();
                return false;
            }
            if(!mygear.setRatio(nom,den)){
                nom = old_nom;
                sprintf(lm.buf,"Bad Ratio: Den: %d Nom: %d\n",den,nom);
                lm.error();
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
                sprintf(lm.buf,"\n\n\n\tinit_gear setRatio failed: count: %lld nom: %i den: %i",count,nom,den);
                lm.error();
                return false;
            }
            
        }

        inline void stepPos(){
#ifdef useFAS
            int8_t r = fzstepper->move(1,false);
#else
            zstepper.step();
#endif
            position++;
        }
        inline void stepNeg(){
#ifdef useFAS
            int8_t r = fzstepper->move(-1,false);
#else
            zstepper.step();
#endif
            position--;

        }

        inline void setAccel(int val){
#ifdef useFAS
            fzstepper->setAcceleration(val);
            fzstepper->applySpeedAcceleration();
#else
            // do nothing no acceleration
#endif
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
            printf("steps per mm: %i pitch in steps: %i max pitch was: %i",stepsPerMM,pitchInSteps,max);
            if(max > pitchInSteps){
                return true;
            }
            return false;
        }

    };
    inline State init(const char * name,Log::Msg lm,Config c){
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
        int stepper_speed = 80000;
        int accel = 50000;
#ifdef useFAS
        state.engine.init();
        state.fzstepper = state.engine.stepperConnectToPin(Z_STEP_PIN);
        if (state.fzstepper) {
            state.fzstepper->setDirectionPin(Z_DIR_PIN,true,10);
            state.fzstepper->setEnablePin(Z_EN_PIN);
            state.fzstepper->setAutoEnable(true);

            // If auto enable/disable need delays, just add (one or both):
            //stepper->setDelayToEnable(15);

            state.fzstepper->setSpeedInHz(stepper_speed);  // the parameter is us/step !!!
            state.fzstepper->setAcceleration(accel);
            state.fzstepper->applySpeedAcceleration();
            state.diduseFAS = true;
        }
#else
        pinMode(Z_DIR_PIN, OUTPUT);
        pinMode(Z_STEP_PIN, OUTPUT);

        //stepsPerMM = c.motor_steps / c.lead_screw_pitch;


        state.zstepper.config.channel = RMT_CHANNEL_0;
        state.zstepper.config.stepPin = (gpio_num_t) Z_STEP_PIN;
        state.zstepper.config.dirPin = (gpio_num_t) Z_DIR_PIN;

        state.zstepper.init();
        state.diduseFAS = false;
#endif
        return state;
        
    }

    
}
