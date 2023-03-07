#pragma once
#include <Arduino.h>
#include "util.h"

// TODO: need a way to tie versions of the firmware to compatable versions of the UI
// also need to have a compiled UI version linked in firmware releases
#define VSN "0.0.2"

#ifndef configured
#error WARNING: no config.ini detected. Please copy config.ini.template to config.ini and customize it.
#endif
/************* Machine Configuration *************/

//web interface hostname
#ifndef HOSTNAME
#error Set your HOSTNAME in your config.ini
#endif

#ifndef WIFI_SSID
#error Set your WIFI_SSID in your config.ini
#endif

#ifndef WIFI_PASSWORD
#error Set your WIFI_PASSWORD in your config.ini
#endif
#ifndef LEADSCREW_LEAD
#error Set your LEADSCREW_LEAD in your config.ini
#endif

// Stepper driver Pins
//NOTE: it's recommended to avoid pins 12 to 18 if you want to use JTAG for on circuit debugging
#ifndef Z_STEP_PIN
#error Set your Z_STEP_PIN in your config.ini
#endif

#ifndef Z_DIR_PIN
#error Set your Z_DIR_PIN in your config.ini
#endif

//I don't think this is needed, we can infer this by microstepping and native steps per rev
#define MOTOR_TYPE 1 // 1 is 1.8 degree, 2 is .9 degree

#ifndef Z_NATIVE_STEPS_PER_REV
#error Set your Z_NATIVE_STEPS_PER_REV in your config.ini
#endif

#ifndef Z_MICROSTEPPING
#error Set your Z_MICROSTEPPING in your config.ini
#endif

// Spindle Encoder pins

#ifndef EA
#error Set your EA in your config.ini
#endif
#ifndef EA
#error Set your EA in your config.ini
#endif
/*
choices are: 
Pullup::EXTERN //your hardware handles the pullup or pulldown, for example with a resistor to 3v3 or to gnd
Pullup::INTERN_PULLDOWN //your encoder expects the MCU to pull low, and sends a high for a pulse
Pullup::INTERN_PULLUP //your esp handles the pullup and your encoder pulls it down.
For most common quadrature encoders, choose Extern and apply 3.3v to your encoder pins through 1.5k to 4.7k ohm resistors */
#ifndef ENCODER_PULLUP_TYPE
#error Set your ENCODER_PULLUP_TYPE in your config.ini
#else
#define ENCODER_PULLUP Pullup::ENCODER_PULLUP_TYPE
#endif


//this should be ON, unless you have a good reason not to. Otherwise your leadscrew will not track reverses.
#ifndef ENCODER_QUADRATURE_MODE
#error Set your ENCODER_QUADRATURE_MODE in your config.ini
#else
#define QUADRATURE_MODE Quadrature::ENCODER_PULLUP_TYPE
#endif

//this is counts per rev *4 for a quadrature encoder
#ifndef ENCODER_RESOLUTION
#error Set your ENCODER_RESOLUTION in your config.ini
#endif

//use the display or not. Recommend not, it doesn't have much functionality. Set to 1 to enable.
#ifndef USESSD1306
#define USESSD1306 0
#endif

#ifndef error_led
#define error_led 21
#endif

// read the cpu usage via the web server, only use for testing as it comes with bloat
//#define DEBUG_CPU_STATS

//TODO I think this isn't necessary since we can calculate microsteps per rev with the above and can divide the distance into steps instead of degrees
#define STEPPER_DEGREES_PER_STEP 1.8
