#include "Controls.h"

#include "config.h"

#define BOUNCE_LOCK_OUT
#include <Bounce2.h>
#include "neotimer.h"
#include <yasm.h>
#include "rmtStepper.h"
#include "gear.h"
#include "Machine.h"
#include "Encoder.h"
#include <elslog.h>
#include "state.h"
#include "Stepper.h"
#include "display.h"
#include "web.h"
#include "genStepper.h"
#include "motion.h"
#include "BounceMode.h"

Neotimer button_print_timer = Neotimer(2000);
Neotimer dro_timer = Neotimer(600);


uint8_t menu = 3; 
int pitch_menu= 1;


#ifdef DEBUG_CPU_STATS
char stats[ 2048];
#undef configGENERATE_RUN_TIME_STATS
#define configGENERATE_RUN_TIME_STATS 1
#undef configUSE_STATS_FORMATTING_FUNCTIONS
#define configUSE_STATS_FORMATTING_FUNCTIONS 1
#endif


void init_controls(){
  bounce_yasm.next(startupState);  
}




void updateMode(RunMode run){
  Serial.print("updating modes: ");
  Serial.println((int)run);
  run_mode = run;
  updateStateDoc();
}

void startupState(){
  if(bounce_yasm.isFirstRun()){
    updateMode(RunMode::STARTUP);
    web = true;
  }
}

void slaveJogReadyState(){
  if(bounce_yasm.isFirstRun()){
    updateMode(RunMode::SLAVE_JOG_READY);
    //setFactor();
    //gs.setELSFactor(pitch);
    web = true;
    return;
  }
}


// This is "slave jog" status mode, "slave" status is in SlaveMode.cpp
void slaveJogStatusState(){
  if(bounce_yasm.isFirstRun()){
    updateMode(RunMode::RUNNING);
    web = false;
  }
}



/* replace with genStepper impl
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
*/

//this defines the parameters for the thread and turning for both metric and imperial threads


