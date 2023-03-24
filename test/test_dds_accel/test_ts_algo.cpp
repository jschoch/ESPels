#ifdef UNIT_TEST

#include <unity.h>
#include <iostream>

#include <algorithm>
#include <cmath>
#include <cstdint>


#include "src/moveConfig.h"
#include "src/genStepper.h"

#include "src/Stepper.h"


// because we are not compiling the .cpp
int32_t s_0 = 0;
int32_t move_distance = 0;
uint32_t vstart = 0;
uint32_t vend = 0;
uint32_t vtarget = 0;
int64_t vs_sqr = 0;
int64_t ve_sqr = 0;
int64_t vt_sqr = 0;
uint32_t two_a = 0;
int32_t accEnd = 0;
int32_t  decStart = 0;


// static init
Gear::State GenStepper::State::mygear;
rmtStepper::State GenStepper::State::zstepper;
int GenStepper::State::nom;
int GenStepper::State::den;
int GenStepper::State::position;
int32_t MoveConfig::State::moveTargetSteps ;
int32_t MoveConfig::State::moveDistanceSteps ;
bool MoveConfig::State::startSync;
bool MoveConfig::State::moveDirection ;
//int32_t MoveConfig::State::moveSyncTarget ;
int MoveConfig::State::stopPos ;
int MoveConfig::State::stopNeg ;
bool MoveConfig::State::spindle_handedness ;
double MoveConfig::State::movePitch ;
double MoveConfig::State::rapidPitch ;
double MoveConfig::State::oldPitch ;
//bool MoveConfig::State::syncMoveStart ;
//bool MoveConfig::State::isAbs  ;
int MoveConfig::State::accel;
bool MoveConfig::State::useStops ;
bool MoveConfig::State::feeding_ccw;
int Gear::State::next;
int Gear::State::prev;
int Gear::State::last;

Log::Msg lm;
GenStepper::Config gconf = {
        0,
        "Z",
        2.0, // lead screw pitch
        2400, // spindle enc resolution
        200, // native steps
        8 // microsteps
    };
GenStepper::State gs = GenStepper::init("z",lm,gconf);
MoveConfig::State mc = MoveConfig::init();



void test_the_algo(){
    std::cout << "testing algo\n";

    // prepareMovement(int32_t currentPos, int32_t targetPos, 
    //    uint32_t targetSpeed, uint32_t pullInSpeed, uint32_t pullOutSpeed, uint32_t accel)
    int test_distance = 100;
    int start_vel = prepareMovement(0,test_distance,1000,10,10,100);
    std::cout << "start velocity was: " << start_vel << "\n";
    int i = updateSpeed(&gs);

    for (auto i = 0;i < test_distance;i++){
        gs.position++;
        auto n = updateSpeed(&gs);
        std::cout << "i was" << i <<  " n was: " << n <<"\n";
    }


    // how does this work?

    // 2 timers, 1 generating stepper pulses, the 2nd slower timer is checking 
    // the result of update_speed and updating the 1st timer frequency as needed
    
}

int main( int argc, char **argv) {
    UNITY_BEGIN();
    std::cout << "starting teensy step algo tests\n";


    RUN_TEST(test_the_algo); 
    UNITY_END();
    return 0;
}
#endif
