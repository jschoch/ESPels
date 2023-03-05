#pragma once
#include <iostream>

namespace Mocks{
    struct rmtState {
        bool dir = {true};
        bool dir_has_changed = {false};
        void step(){
            std::cout <<"step\n";
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
