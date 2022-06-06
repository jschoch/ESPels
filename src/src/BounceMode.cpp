#include <Arduino.h>
#include "config.h"
#include <yasm.h>

YASM bounce_yasm;
volatile bool bouncing = false;
double old_jog_mm = 0;

Neotimer state_timer(100);

void do_state(){
    bounce_yasm.run();
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
        Serial.print(toolPos);
        Serial.println("Entering Bounce Jog Mode");
        run_mode = RunMode::BounceJog;
        start_jog();
        updateStateDoc();
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
        Serial.print(toolPos);
        Serial.println(" Entering Rapid Mode");
        oldPitch = pitch;
        pitch = rapids;
        old_jog_mm = jog_mm;
        jog_mm = -jog_mm;
        rapiding = true;
        setStops();
        start_jog();
        updateStateDoc();
        return;
    }
    else if(!rapiding){
        Serial.println("Rapid Done, going Idle");
        pitch = oldPitch;
        jog_mm = old_jog_mm;
        bouncing = false;
        bounce_yasm.next(BounceIdleState);
    }

}
