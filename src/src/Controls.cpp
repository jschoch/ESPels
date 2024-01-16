#include "Controls.h"

#include "config.h"

#include <yasm.h>
#include "BounceMode.h"
#include "web.h"
#include "state.h"



/*/
#ifdef DEBUG_CPU_STATS
char stats[ 2048];
#undef configGENERATE_RUN_TIME_STATS
#define configGENERATE_RUN_TIME_STATS 1
#undef configUSE_STATS_FORMATTING_FUNCTIONS
#define configUSE_STATS_FORMATTING_FUNCTIONS 1
#endif
*/


void init_controls(){
  main_yasm.next(startupState);  
}




void updateMode(RunMode run){
  Serial.print("updating modes: ");
  Serial.println((int)run);
  run_mode = run;
  updateStateDoc();
}

void startupState(){
  if(main_yasm.isFirstRun()){
    updateMode(RunMode::STARTUP);
    web = true;
  }
}

void slaveJogReadyState(){
  if(main_yasm.isFirstRun()){
    updateMode(RunMode::SLAVE_JOG_READY);
    web = true;
    return;
  }
}


// This is "slave jog" status mode, "slave" status is in SlaveMode.cpp
void slaveJogStatusState(){
  if(main_yasm.isFirstRun()){
    updateMode(RunMode::RUNNING);
    web = false;
  }
}



/* 

TODO: move state stuff out of here, this is very old from when there were physical controls

replace with genStepper impl
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



