#pragma once
//State that's common to multiple contexts. 
//TODO: (Chris) Eventually move this to a class, perhaps the state machine. Then we don't have to 
// extern all these things to make the linker happy, and each object can guarantee nothing else
// is writing to this memory when it shouldn't be (makes future expansion easier)
//TODO: identify which of these belong to other objects and make those objects "own" these
#include "Encoder.h"
#include "Controls.h"
#include "genStepper.h"
#include "moveConfig.h"
#include <yasm.h>

extern const char* vsn;

#include "ArduinoJson.h"


void do_state();
void init_state();
//common variables used by multipe things
extern volatile float rpm;

extern bool sendDebug;
extern YASM main_yasm;


extern GenStepper::State gs;
extern MoveConfig::State mc;

//State Machine stuff
extern bool syncStart;
extern bool syncWaiting;
extern volatile bool jogging;
extern volatile bool rapiding;

extern RunMode run_mode;

#define SSE_EVENT_SIZE 2000


// config stateDoc
extern StaticJsonDocument<1000> stateDoc;

//  items to store in NV ram/EEPROM
extern StaticJsonDocument<1000> nvConfigDoc;

// Used for msgs from UI
extern StaticJsonDocument<1000> inDoc;

// used to send status to UI
extern StaticJsonDocument<600> statusDoc;

// Used to log to UI
extern StaticJsonDocument<5000> logDoc;

// used for debugging, to slim down status doc 
//extern StaticJsonDocument<500> debugStatusDoc;

// for pong
extern StaticJsonDocument<100> pingDoc;

// for sending upated move config
extern StaticJsonDocument<1000> moveConfigDoc;

// for sending events
extern StaticJsonDocument<SSE_EVENT_SIZE> eventDoc;

//extern char pongBuf[100];
//extern int pong_len;


