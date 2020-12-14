#pragma once
#ifndef _CONFIG_H
#define _CONFIG_H



#include <Arduino.h>
#include <ESP32Encoder.h>

#include "neotimer.h"
#include "util.h"
#include "display.h"
#include "Machine.h"
#include "Controls.h"
#include "Stepper.h"
#include "Encoder.h"
#include "motion.h"
#include "freertos/task.h"
#include "web.h"
#include "Vector.h"
#include "SlaveMode.h"
#include "DebugMode.h"
#include "c:\Users\jesse\Documents\Arduino\config.h"
#include <yasm.h>



// Encoder pins
#define EA 25
#define EB 26 

#define USESSD1306 1

// Machine Settings

// lead screw pitch in mm
#define LEAD_SCREW_PITCH 5
#define MICROSTEPS 32
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


extern volatile int64_t delta;
extern int32_t left_limit_max ;
extern volatile int32_t left_limit;

extern uint8_t menu ; 

extern volatile int64_t toolPos;
extern volatile uint8_t err;

 
extern int spindle_encoder_resolution;   // the number of pulses per revolution of the spindle encoder
extern volatile int64_t calculated_stepper_pulses;
extern volatile double factor;  




extern uint8_t menu;

extern const char* FEED_MODE [];
extern const char* DISPLAY_MODE [];
extern volatile bool z_moving;
extern volatile bool z_pause;
extern volatile int delay_ticks;
extern ESP32Encoder encoder;
extern volatile bool feeding;
extern volatile bool feeding_dir;
//extern class BtnState btnState;
extern int rpm;
extern float rapids;
extern uint8_t motor_type;
extern float backlash;
extern bool web;
extern long mm_per_tick;
typedef Vector<const char*> Menu;
extern Menu feed_menu_items;
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

#endif