#include "state.h"

#include "genStepper.h"
#include "moveConfig.h"
//Initialize the starting memory. 
//TODO: move this into a class constructor
#include "ArduinoJson.h"
//Initialize the starting memory.

// TODO: can this be automagical somehow?
const char* vsn = "0.0.3";

//common variables used by multipe things
volatile int rpm = 0;
double mmPerStep = 0;
int32_t stepsPerMM = 0;
int32_t relativePosition = 0;
int32_t absolutePosition = 0;
bool sendDebug = false;


//State Machine stuff
bool syncStart = true;
bool syncWaiting = false;
volatile bool jogging = false;
volatile bool rapiding = false;


RunMode run_mode = RunMode::STARTUP;


GenStepper::State gs = GenStepper::init("Z",el);
MoveConfig::State mc = MoveConfig::init();
// json docs

// config stateDoc
StaticJsonDocument<1000> stateDoc;

//  items to store in NV ram/EEPROM
StaticJsonDocument<1000> nvConfigDoc;

// Used for msgs from UI
StaticJsonDocument<1000> inDoc;

// used to send status to UI
StaticJsonDocument<600> statusDoc;

// Used to log to UI
StaticJsonDocument<5000> logDoc;

// used for debugging, to slim down status doc 
StaticJsonDocument<500> debugStatusDoc;

StaticJsonDocument<100> pongDoc;
char pongBuf[100];
int pong_len = serializeMsgPack(pongDoc,pongBuf);
