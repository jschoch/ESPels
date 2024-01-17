#pragma once

#ifndef UNIT_TEST
#define UNIT_TEST
#endif

#include <yasm.h>
#include "../../src/src/AsyncBounceMode.h"
#include "../../src/src/genStepper.h"
#include "../../src/src/moveConfig.h"
//#include "../../src/src/genStepper.h"







extern YASM main_yasm;


inline void startStepperTimer(int x){};

//void startStepperTimer(int i){};



/*
// static init
extern Gear::State GenStepper::State::mygear;
extern rmtStepper::State GenStepper::State::zstepper;
extern int GenStepper::State::nom;
extern int GenStepper::State::den;
extern int GenStepper::State::position;
extern int32_t MoveConfig::State::moveTargetSteps ;
extern int32_t MoveConfig::State::moveDistanceSteps ;
extern bool MoveConfig::State::startSync;
extern bool MoveConfig::State::moveDirection ;
//int32_t MoveConfig::State::moveSyncTarget ;
extern int MoveConfig::State::stopPos ;
extern int MoveConfig::State::stopNeg ;
extern bool MoveConfig::State::spindle_handedness ;
extern double MoveConfig::State::movePitch ;
extern double MoveConfig::State::rapidPitch ;
extern double MoveConfig::State::oldPitch ;
//bool MoveConfig::State::syncMoveStart ;
//bool MoveConfig::State::isAbs  ;
extern int MoveConfig::State::accel;
extern bool MoveConfig::State::useStops ;
extern bool MoveConfig::State::feeding_ccw;
extern int MoveConfig::State::moveSpeed;
extern int Gear::State::next;
extern int Gear::State::prev;
extern int Gear::State::last;
extern Log::Msg lm;
*/
extern GenStepper::State gs;
extern MoveConfig::State mc;
