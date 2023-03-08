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
int old_moveDistanceSteps = 0;

Neotimer state_timer(200);

void do_state(){
    if(state_timer.repeat()){
        bounce_yasm.run();
    }
    
}

void start_move(){
    Serial.println("Bounce Start Move Called");
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
  }

}

void BounceMoveState(){
    if(bounce_yasm.isFirstRun()){
        printf("Entering Bounce Move Mode pitch: %f\n",mc.pitch);
        gs.setELSFactor(mc.pitch);
        updateMode(RunMode::BounceMove);
        start_move();
        return;
    }
    else if(!jogging){
        updateStateDoc();
        Serial.println("Move Done, starting rapid");
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
        start_move();
        updateStateDoc();
        return;
    }
    else if(!rapiding){
        Serial.println("Rapid Done, going Idle");
        mc.pitch = mc.oldPitch;
        mc.moveDistanceSteps = old_moveDistanceSteps;
        bouncing = false;
        jogging = false;
        bounce_yasm.next(BounceIdleState);
    }

}
