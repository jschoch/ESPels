#include "state.h"

#include "genStepper.h"
#include "moveConfig.h"
#include "BounceMode.h"

//Initialize the starting memory. 
//TODO: move this into a class constructor
#include "ArduinoJson.h"
//Initialize the starting memory.

#include "AsyncBounceMode.h"

YASM main_yasm;



Neotimer state_timer(200);

void init_state(){
    main_yasm.next(startupState);
}

void do_state(){
    
    if(state_timer.repeat()){
        //Serial.printf("0");
        //TODO: do we need to run this when in startup or some idle state?
        main_yasm.run();
    }
    
}
// TODO: can this be automagical somehow?
const char* vsn = "0.0.6";




// TODO:  put all these globals in a  struct or somethign

//common variables used by multipe things
volatile float rpm = 0;
bool sendDebug = true;
//State Machine stuff
bool syncStart = true;
bool syncWaiting = false;
volatile bool jogging = false;
volatile bool rapiding = false;
extern Ticker vTcker;


RunMode run_mode = RunMode::STARTUP;


GenStepper::Config gconf = {
        0,
        "Z",
        LEADSCREW_LEAD, // lead screw pitch
        ENCODER_RESOLUTION, // spindle enc resolution
        Z_NATIVE_STEPS_PER_REV, // native steps
        Z_MICROSTEPPING, // microsteps
        (Z_NATIVE_STEPS_PER_REV * Z_MICROSTEPPING) // motorsteps
    };

GenStepper::State gs = GenStepper::init("Z",&el,gconf);
MoveConfig::State mc = MoveConfig::init();



// json docs

// config stateDoc
//StaticJsonDocument<1000> stateDoc;

//  items to store in NV ram/EEPROM
StaticJsonDocument<1000> nvConfigDoc;

// Used for msgs sent from UI
StaticJsonDocument<1000> inDoc;

// used to send status to UI
//StaticJsonDocument<600> statusDoc;

// Used to log to UI
StaticJsonDocument<5000> logDoc;

// used for debugging, to slim down status doc 
//StaticJsonDocument<500> debugStatusDoc;

// for sending updates to move config
StaticJsonDocument<1000> moveConfigDoc;


StaticJsonDocument<SSE_EVENT_SIZE> eventDoc;

//char pongBuf[100];
//int pong_len = serializeMsgPack(pongDoc,pongBuf);

