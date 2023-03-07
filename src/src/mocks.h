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
