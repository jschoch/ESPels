#include "Controls.h"

#include "config.h"

#define BOUNCE_LOCK_OUT

#ifdef HAS_BUTTONS
#include <Bounce2.h>
#endif

#include "neotimer.h"
#include <yasm.h>
#include "rmtStepper.h"
#include "gear.h"

Neotimer button_print_timer = Neotimer(2000);
Neotimer dro_timer = Neotimer(600);
Log::Msg el;


uint8_t menu = 3; 
int pitch_menu= 1;
volatile bool feeding = false;
volatile bool z_feeding_dir = true;




#ifdef DEBUG_CPU_STATS
char stats[ 2048];
#undef configGENERATE_RUN_TIME_STATS
#define configGENERATE_RUN_TIME_STATS 1
#undef configUSE_STATS_FORMATTING_FUNCTIONS
#define configUSE_STATS_FORMATTING_FUNCTIONS 1
#endif

YASM btn_yasm;


int32_t stepsPerMM = 0;
int32_t relativePosition = 0;
int32_t absolutePosition = 0;


void init_controls(){
  btn_yasm.next(startupState);  
}



void read_buttons(){
  // this just prints out the mode every 2 seconds
  if(button_print_timer.repeat()){
    Serial.printf(" %d %d ",(int)run_mode,WiFi.RSSI());
  }
}

void updateMode(RunMode run){
  Serial.print("updating modes: ");
  Serial.println((int)run);
  run_mode = run;
  updateStateDoc();
}

void startupState(){
  if(btn_yasm.isFirstRun()){
    updateMode(RunMode::STARTUP);
    web = true;
  }
}

void slaveJogReadyState(){
  if(btn_yasm.isFirstRun()){
    updateMode(RunMode::SLAVE_JOG_READY);
    setFactor();
    web = true;
  }
}


// This is "slave jog" status mode, "slave" status is in SlaveMode.cpp
void slaveJogStatusState(){
  if(btn_yasm.isFirstRun()){
    updateMode(RunMode::RUNNING);
    web = false;
  }
}


extern struct Gear::State gear;

void setFactor(){
  
  int den = lead_screw_pitch * spindle_encoder_resolution ;
  int nom = motor_steps * pitch;

  //
  Serial.printf("nom: %d den: %d",nom,den);

  // check ratio to be sure it can be done
  if (!gear.setRatio(nom,den)){
    sprintf(el.buf,"Bad Ratio: Den: %d Nom: %d\n",nom,den);
    el.error();
    pitch = oldPitch;
    return;
  }
} 

void setHobbFactor(){
  int den = lead_screw_pitch * motor_steps;
  int nom = spindle_encoder_resolution * pitch;

  //
  Serial.printf("nom: %d den: %d",nom,den);

  // check ratio to be sure it can be done
  if (!gear.setRatio(nom,den)){
    sprintf(el.buf,"Bad Ratio: Den: %d Nom: %d\n",nom,den);
    el.error();
    pitch = oldPitch;
    return;
  }
}

//this defines the parameters for the thread and turning for both metric and imperial threads

void thread_parameters()                                           
  { 
    // TODO: add imperial back when you work on thread cycle
}

