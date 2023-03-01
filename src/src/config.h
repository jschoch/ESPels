#pragma once

// #  Sourced from prototypicall/Didge on github.  no license when sourced
#ifndef _CONFIG_H
#define _CONFIG_H

#define ARDUINOJSON_USE_LONG_LONG 1
#define ARDUINOJSON_USE_DOUBLE 1

// For including Bounce2.h
//#define HAS_BUTTONS 

#include <Arduino.h>
#include "neotimer.h"
#include "util.h"
#include "display.h"
#include "Machine.h"
#include "Controls.h"
#include "Stepper.h"
#include "Encoder.h"
#include "motion.h"
#include "freertos/task.h"
namespace Log{
    class Msg;
};
#include "web.h"
#include "SlaveMode.h"
#include "DebugMode.h"

#include <yasm.h>
#include "myperfmon.h"
#include "rmtStepper.h"
#include "log.h"
#include "BounceMode.h"
#include "hob.h"


// Pins

#define Z_STEP_PIN 13
#define Z_DIR_PIN 12

// Encoder pins
#define EA 25
#define EB 26 
#define error_led 14

#define USESSD1306 1
constexpr const char* myname = "mx210lathe";
#define encoder_pullup Pullup::EXTERN


/*
// ############################## az-deliver devkit c

//    Set the machine name for wifi and websockets
//char* myname = "els";
//char* myname = "divider";
constexpr const char* myname = "divider";

//   Motor pins
#define Z_STEP_PIN 13
#define Z_DIR_PIN 12
#define Z_FAULT_PIN 14

// Encoder pins
//  set for internal pullup or external
//#define encoder_pullup Pullup::INTERN_PULLDOWN
#define encoder_pullup Pullup::INTERN_PULLUP

#define EA 16
#define EB 17

#define error_led 25
*/
// Machine Settings

#define MOTOR_TYPE 1 // 1 is 1.8 degree, 2 is .9 degree

// read the cpu usage via the web server, only use for testing as it comes with bloat
//#define DEBUG_CPU_STATS

 
extern double lead_screw_pitch;
extern int microsteps;
extern int native_steps;
extern int motor_steps;
extern int tpi;
extern int motor_steps;
extern double pitch; //  the thread pitch
extern double depth;                                       // a parameter to define the thread depth in mm on the compound slide. This is set at 75% of the pitch which seems to work
extern int display_mode;
extern volatile int16_t encoder0Pos;
extern volatile int64_t spindlePos;


extern volatile int delta;
extern int32_t left_limit_max ;
extern volatile int32_t left_limit;

extern uint8_t menu ; 

extern volatile int64_t toolPos;

 
extern int spindle_encoder_resolution;   // the number of pulses per revolution of the spindle encoder
extern volatile int64_t calculated_stepper_pulses;




extern uint8_t menu;

extern const char* FEED_MODE [];
extern const char* DISPLAY_MODE [];
extern volatile bool z_moving;
extern volatile bool z_pause;
extern volatile int delay_ticks;
extern class Encoder encoder;
extern volatile bool feeding;
extern volatile bool z_feeding_dir;
extern volatile bool feeding_ccw;
//extern class BtnState btnState;
extern int rpm;
extern double rapids;
extern uint8_t motor_type;
extern double backlash;
extern bool web;
extern class YASM btn_yasm;
extern class YASM bounce_yasm;
extern class YASM hob_yasm;
extern struct Bd lbd;
extern int pitch_menu;
extern Bd rbd;
extern Bd ubd;
extern Bd dbd;
extern Bd sbd;
extern Bd mbd;
extern volatile int32_t jog_steps;
extern volatile bool jogging;
extern volatile bool jog_done;
extern volatile uint16_t vel;
extern int stepsPerMM;
extern int32_t relativePosition;
extern int32_t absolutePosition;
extern RunMode run_mode;
extern double jog_mm;
extern volatile double jog_scaler;
extern volatile double toolRelPos;
extern volatile double toolRelPosMM;
extern volatile double targetToolRelPos;
extern volatile double targetToolRelPosMM;
extern int virtEncoderCount;
extern volatile bool useStops;
extern volatile double stopPos;
extern volatile double stopNeg;
extern volatile bool stopPosEx;
extern volatile bool stopNegEx;
extern int8_t cpu0;
extern int8_t cpu1;
extern volatile int exDelta;
extern uint8_t statusCounter;
extern volatile bool pos_feeding;
extern struct rmtStepper::State zstepper;
extern double mmPerStep;
extern double oldPitch;
extern class Log::Msg el;
extern bool syncStart;
extern bool syncWaiting;
extern volatile bool rapiding;
extern volatile bool bouncing;
extern volatile int vEncSpeed;
extern volatile bool vEncStopped;
#endif
