#include <test_lib_helper.h>
#include <yasm.h>
#include "../../src/src/mocks.h"
#include "../../src/src/genStepper.h"
#include "../../src/src/moveConfig.h"



YASM async_bounce_yasm;



// static vars for gs and mc

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
int MoveConfig::State::moveSpeed;
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
