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
#include "AsyncBounceMode.h"

volatile bool bouncing = false;
int old_moveDistanceSteps = 0;



void start_move(){
    Serial.println("Bounce Start Move Called");
    jogging = true;
    init_pos_feed();
}

void BounceIdleState(){
   if(main_yasm.isFirstRun()){
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
    if( maybeStopBounce())
        return;
    if(main_yasm.isFirstRun()){
        printf("Entering Bounce Move Mode pitch: %f\n",mc.movePitch);
        bool d = mc.setStops(gs.position);
        bool z_dir = gs.zstepper.setDir(d);
        printf("z_dir was: %d\n",z_dir);
        gs.setELSFactor(mc.movePitch);
        start_move();
        updateMode(RunMode::BounceMove);
        //main_yasm.next(BounceMoveState);
        return;
    }
    else if(!jogging){
        updateStateDoc();
        Serial.println("Move Done, starting rapid");
        //main_yasm.next(BounceRapidState);
        main_yasm.next(BounceMoveDwellState,true);
    }
}
bool maybeStopBounce(){
    if(!bouncing){
        printf("stopping bounce move now\n");
        stepTimerIsRunning = false;
        main_yasm.next(BounceIdleState,true);
        return true;
    }
    return false;
}

void BounceMoveDwellState(){
    if( maybeStopBounce())
        return;
    if(main_yasm.isFirstRun()){
        printf("bounce dwell first run\n");
        vTck::O::startTimer();
        return;
    }
    printf("bounce dwell N run %d\n",vTck::O::timerRunning());
    if(!vTck::O::timerRunning()){
        printf("timer done, moving back\n");
        //main_yasm.next(AsyncBounceMoveFromState,true);
        main_yasm.next(BounceRapidState);
    }
};

void BounceRapidState(){
    if( maybeStopBounce())
        return;
    if(main_yasm.isFirstRun()){
        printf(" Entering Rapid Mode pitch: %f",mc.rapidPitch);
        // TODO: yuck refactor this
        mc.oldPitch = mc.movePitch;
        old_moveDistanceSteps = mc.moveDistanceSteps;
        mc.moveDistanceSteps = -mc.moveDistanceSteps;
        rapiding = true;
        gs.setELSFactor(mc.rapidPitch);
        mc.setStops(gs.position);
        start_move();
        updateStateDoc();
        return;
    }
    else if(!rapiding){
        Serial.println("Rapid Done, going Idle");
        mc.movePitch = mc.oldPitch;
        mc.moveDistanceSteps = old_moveDistanceSteps;
        bouncing = false;
        jogging = false;
        
        // not sure how isFirstRun works !?
        //main_yasm.next(BounceIdleState);
        //main_yasm.next(slaveJogReadyState);
        //RunMode(SLAVE_JOG_READY);
        //updateStateDoc();
        //main_yasm.next(slaveJogReadyState,true);
        setRunMode((int)RunMode::SLAVE_JOG_READY);
        Serial.println("Bounce Mode done");
        return;
    }

}
