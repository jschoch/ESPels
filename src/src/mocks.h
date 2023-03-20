#pragma once
#include <iostream>

namespace rmtStepper{
    struct State {
        bool dir = {true};
        bool dir_has_changed = {false};
        void step(){
            //std::cout <<"step " << dir << "\n";
        };
        inline bool setDir(bool newDir){
            dir = newDir;
            return dir;
        }
        inline void setDir(bool newDir,bool now){
            dir = newDir;
        }
    } ;
}



#define Z_STEP_PIN 1
#define Z_DIR_PIN 2
#define Z_EN_PIN 3
class FastAccelStepper {
    public: 

    void setDirectionPin(uint8_t dirPin, bool dirHighCountsUp,
                                       uint16_t dir_change_delay_us ){}
    int8_t move(int32_t move, bool blocking){
        return 0;
        };
    void setEnablePin(int8_t i){};
    void setEnablePin(uint8_t enablePin,
                                    bool low_active_enables_stepper){};

    void applySpeedAcceleration(){};
    void setAcceleration(int accel){};
    void setSpeedInHz(int speed){};
    void setAutoEnable(bool t){};
    void setDirectionPin(int i,bool t,int i2){};
};

class FastAccelStepperEngine{

    public: 
    void init(){};
    FastAccelStepper *stepperConnectToPin(uint8_t step_pin){return NULL;};
};
