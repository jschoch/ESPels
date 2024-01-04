/*

TODO:  This seems 100% not used
        get rid of it if so
*/

#include <Arduino.h>
#include "config.h"
#include <yasm.h>
#include "state.h"
/*
#include "web.h"
#include "Controls.h"
#include "motion.h"
#include "BounceMode.h"


void SlaveModeReadyState(){
   if(main_yasm.isFirstRun()){
    updateMode(RunMode::SLAVE_READY);
    web = true;
  }
}

void slaveStatusState(){
    if(main_yasm.isFirstRun()){
        updateMode(RunMode::RUNNING);
        // ?? this was off, why?
        web = true;
    }
}
*/

void FeedModeReadyState(){
    if(main_yasm.isFirstRun()){
        updateMode(RunMode::FEED_READY);

    }
}

/*

void slaveFeedingState(){
    if(main_yasm.isFirstRun()){

        // TODO: should check for RPM 0 for this state?
        updateMode(RunMode::RUNNING);
        feeding = true;
        web = false;
    }

}
*/
