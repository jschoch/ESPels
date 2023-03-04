#pragma once
#include <Arduino.h>
#include "util.h"
/************* Machine Configuration *************/

//web interface hostname
#define HOSTNAME "mx210lathe"

#define LEADSCREW_LEAD 2.0

// Stepper driver Pins
//NOTE: it's recommended to avoid pins 12 to 18 if you want to use JTAG for on circuit debugging

// jesse's pin config
#define Z_STEP_PIN 13
#define Z_DIR_PIN 12

// Chris's pin config
//#define Z_STEP_PIN 18
//#define Z_DIR_PIN 33
//I don't think this is needed, we can infer this by microstepping and native steps per rev
#define MOTOR_TYPE 1 // 1 is 1.8 degree, 2 is .9 degree
#define Z_NATIVE_STEPS_PER_REV 200
#define Z_MICROSTEPPING 8

// Spindle Encoder pins
#define EA 25
#define EB 26
/*
choices are: 
Pullup::EXTERN //your hardware handles the pullup or pulldown, for example with a resistor to 3v3 or to gnd
Pullup::INTERN_PULLDOWN //your encoder expects the MCU to pull low, and sends a high for a pulse
Pullup::INTERN_PULLUP //your esp handles the pullup and your encoder pulls it down.
For most common quadrature encoders, choose Extern and apply 3.3v to your encoder pins through 1.5k to 4.7k ohm resistors */
#define ENCODER_PULLUP Pullup::EXTERN

//this should be ON, unless you have a good reason not to. Otherwise your leadscrew will not track reverses.
#define QUADRATURE_MODE Quadrature::ON
//this is counts per rev *4 for a quadrature encoder
#define ENCODER_RESOLUTION 2400

//use the display or not. Recommend not, it doesn't have much functionality. Set to 1 to enable.
#define USESSD1306 1

#define error_led 21

// read the cpu usage via the web server, only use for testing as it comes with bloat
//#define DEBUG_CPU_STATS

//TODO I think this isn't necessary since we can calculate microsteps per rev with the above and can divide the distance into steps instead of degrees
#define STEPPER_DEGREES_PER_STEP 1.8
