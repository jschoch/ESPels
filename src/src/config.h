#pragma once

// #  Sourced from prototypicall/Didge on github.  no license when sourced
#ifndef _CONFIG_H
#define _CONFIG_H

#define ARDUINOJSON_USE_LONG_LONG 1
#define ARDUINOJSON_USE_DOUBLE 1

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



// Pins

#define Z_STEP_PIN 13
#define Z_DIR_PIN 12

// Encoder pins
#define EA 25
#define EB 26 

#define USESSD1306 1

// Machine Settings

#define MOTOR_TYPE 1 // 1 is 1.8 degree, 2 is .9 degree

// read the cpu usage via the web server, only use for testing as it comes with bloat
//#define DEBUG_CPU_STATS

// User parameters to be altered depending on lathe parameters...................................................... 
// To use this code one must replace these parameters with correct values for your lathe. These are 'spindle_encoder_resolution','lead_screw_pitch' and 'motor_steps'.  
// For example, if you had a spindle encoder of 200 step/rev... then edit the current 'spindle_encoder_resolution=1024' to read 'spindle_encoder_resolution=200'.
// The applies to the 'lead_screw_pitch parameter' and the 'motor_steps' parameter. I have used numbers which apply to a Wabeco D6000 lathe. You must edit for your particular lathe. 
 
 
extern float lead_screw_pitch;
extern int microsteps;
extern int native_steps;
extern int motor_steps;
extern int tpi;
//int microsteps = 32;
//int native_steps = 200;  // 1.8 degree == 200, 0.9 degree == 400, 3 phase is 1.2 degree etc etc
//int motor_steps= (microsteps * native_steps) / lead_screw_pitch;                   // the number of steps per revolution of the lead screw

// the number of steps per revolution of the lead screw
extern int motor_steps;
//int motor_steps = (microsteps * native_steps) /lead_screw_pitch; 

//float pitch=0.085;                     // the pitch to be cut in millimeters.  It also defines the lathe pitch for turning when first power on. 

extern float pitch; // TODO: for testing set back to a decent feed rate
 
// the parameter for the menu select

extern int feed_mode_select;                                 // a parameter to define the programming versus operation settings


extern float depth;                                       // a parameter to define the thread depth in mm on the compound slide. This is set at 75% of the pitch which seems to work
extern float pitch_factor;                           // a parameter to define how deep to push the oblique cutter for each thread pitch in mm. May differ depending on thread design. This one works
//extern enum display_mode_t display_mode;
extern int display_mode;
extern volatile int16_t encoder0Pos;
extern volatile int64_t spindlePos;


extern volatile int delta;
extern int32_t left_limit_max ;
extern volatile int32_t left_limit;

extern uint8_t menu ; 

extern volatile int64_t toolPos;
//extern volatile uint8_t err;

 
extern int spindle_encoder_resolution;   // the number of pulses per revolution of the spindle encoder
extern volatile int64_t calculated_stepper_pulses;
extern volatile double factor;  




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
extern float rapids;
extern uint8_t motor_type;
extern float backlash;
extern bool web;
extern long mm_per_tick;
extern class YASM btn_yasm;
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
extern volatile int step_delta;
extern volatile uint16_t vel;
extern int stepsPerMM;
extern int32_t relativePosition;
extern int32_t absolutePosition;
extern RunMode run_mode;
extern float jog_mm;
extern volatile float jog_scaler;
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
extern float encoder_factor;
extern struct rmtStepper::State zstepper;
extern double mmPerStep;
extern float oldPitch;
extern class Log::Msg el;
extern bool syncStart;
extern bool syncWaiting;
//extern struct Gear::State gear;

#endif
