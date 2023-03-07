#include <Arduino.h>
#include "config.h"
#include <yasm.h>
#include "BounceMode.h"
#include "state.h"
#include "Machine.h"
#include "web.h"
#include "motion.h"
#include "genStepper.h"
#include "moveConfig.h"

YASM bounce_yasm;
volatile bool bouncing = false;
//double old_jog_mm = 0;
int old_moveDistanceSteps = 0;

Neotimer state_timer(200);

void do_state(){
    if(state_timer.repeat()){
        bounce_yasm.run();
    }
    
}

void start_jog(){
    Serial.println("Start Jog Called");
    jogging = true;
    init_pos_feed();
}

void BounceIdleState(){
   if(bounce_yasm.isFirstRun()){
    //updateMode(SLAVE_READY,RunMode::SLAVE_READY);
    //run_mode = RunMode::BounceIdle;
    updateStateDoc();
    return;
  }
  else if(bouncing){
      Serial.println("Bounce Init");
      bounce_yasm.next(BounceJogState);
  }

}

void BounceJogState(){
    if(bounce_yasm.isFirstRun()){
        printf("Entering Bounce Jog Mode pitch: %f\n",mc.pitch);
        gs.setELSFactor(mc.pitch);
        updateMode(RunMode::BounceJog);
        start_jog();
        return;
    }
    else if(!jogging){
        updateStateDoc();
        Serial.println("Jog Done, starting rapid");
        bounce_yasm.next(BounceRapidState);
    }
}

void BounceRapidState(){
    if(bounce_yasm.isFirstRun()){
        printf(" Entering Rapid Mode pitch: %f",mc.rapidPitch);
        // TODO: yuck refactor this
        mc.oldPitch = mc.pitch;
        //pitch = rapids;
        mc.pitch = mc.rapidPitch;
        old_moveDistanceSteps = mc.moveDistanceSteps;
        mc.moveDistanceSteps = -mc.moveDistanceSteps;
        rapiding = true;
        gs.setELSFactor(mc.pitch);
        mc.setStops(gs.currentPosition());
        start_jog();
        updateStateDoc();
        return;
    }
    else if(!rapiding){
        Serial.println("Rapid Done, going Idle");
        mc.pitch = mc.oldPitch;
        //jog_mm = old_jog_mm;
        mc.moveDistanceSteps = old_moveDistanceSteps;
        bouncing = false;
        // TOOD this ends up undefined in the UI
        bounce_yasm.next(BounceIdleState);
    }

}
