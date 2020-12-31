#include <Arduino.h>
#include "config.h"
#include <yasm.h>


void SlaveModeReadyState(){
   if(btn_yasm.isFirstRun()){
    updateMode(SLAVE_READY,RunMode::SLAVE_READY);
    web = true;
  }
  if(lbd.deb->rose()){
    
    Serial.println("slave_ready -> status");
    
    btn_yasm.next(slaveStatusState);
  } 
}

void slaveStatusState(){
    if(btn_yasm.isFirstRun()){
        updateMode(DSTATUS,RunMode::RUNNING);
        web = false;
    }
    if(sbd.deb->fell()){
        Serial.println("slave_status -> startup");
       btn_yasm.next(startupState); 

    }
    if(lbd.deb->rose()){
        Serial.println("slave_status -> slave feed left");
        feeding_dir = true;
        btn_yasm.next(slaveFeedingState);
    }
    if(rbd.deb->rose()){
        feeding_dir = false;
        Serial.println("slave_status -> slave feed right");
        btn_yasm.next(slaveFeedingState);
    }

    if(ubd.deb->rose()){
        pitch_menu++;
        feed_parameters();
    }
    if(dbd.deb->rose()){
        pitch_menu--;
        feed_parameters();
    }
}

void slaveFeedingState(){
    if(btn_yasm.isFirstRun()){

        // TODO: should check for RPM 0 for this state?
        updateMode(DSTATUS,RunMode::RUNNING);
        feeding = true;
        web = false;
    }

    if(sbd.deb->fell()){

        // TODO, store position so you can resync to thread?
        feeding = false;
        Serial.println("slave_feeding -> slave status");
       btn_yasm.next(slaveStatusState); 

    }
}
