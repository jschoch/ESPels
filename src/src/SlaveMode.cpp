#include <Arduino.h>
#include "config.h"
#include <yasm.h>
#include "state.h"
#include "web.h"
#include "Controls.h"
#include "motion.h"


void SlaveModeReadyState(){
   if(btn_yasm.isFirstRun()){
    updateMode(RunMode::SLAVE_READY);
    web = true;
  }
}

void slaveStatusState(){
    if(btn_yasm.isFirstRun()){
        updateMode(RunMode::RUNNING);
        web = false;
    }
}

void slaveFeedingState(){
    if(btn_yasm.isFirstRun()){

        // TODO: should check for RPM 0 for this state?
        updateMode(RunMode::RUNNING);
        feeding = true;
        web = false;
    }

}
