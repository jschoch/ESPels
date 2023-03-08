#include <Arduino.h>
#include "motion.h"

#include "config.h"

#include "gear.h"
// should just be using accel/decel calcs

#include "Encoder.h"
#include "Stepper.h"
#include "state.h"
#include "log.h"
#include "Controls.h"
#include "Machine.h"
#include "genStepper.h"
#define LOG_LOCAL_LEVEL ESP_LOG_INFO
#include <esp_log.h>

static const char* TAG = "Mo";

// not sure this is needed
//volatile bool feeding = false;

//TODO there's a direction enum that can be used, maybe do it at the same time the state machine is refactored
volatile bool feeding_ccw = true;

char err[500] = "";




void init_pos_feed(){
  //setFactor();
  gs.setELSFactor(pitch);
  if(!pos_feeding){
    
    //wait for the start to come around
    if(mc.syncMoveStart){
      Serial.println("Move: waiting for spindle sync");
      syncWaiting = true;
      pos_feeding = true;
    }
    else{
      // this resets the "start" but i'm not sure if it works correctly
      // TODO:  should warn that turning off sync will  loose the "start"
      Serial.println("jog without spindle sync");
      gs.init_gear(encoder.getCount());
      pos_feeding = true;
    }
  }else{
  el.error("already started pos_feeding, can't do it again");
  }
}
void init_hob_feed(){
  el.error("this needs updated to work with genStepper.h");
  //setHobbFactor();
  if(!pos_feeding){
    
    //wait for the start to come around
    if(mc.syncMoveStart){
      Serial.println("waiting for spindle sync");
      syncWaiting = true;
      pos_feeding = true;
    }
    else{
      // this resets the "start" but i'm not sure if it works correctly
      // TODO:  should warn that turning off sync will  loose the "start"
      Serial.println("jog without spindle sync");
      gs.init_gear(encoder.getCount());
      pos_feeding = true;
    }
  }else{
  el.error("already started pos_feeding, can't do it again");
  }
}

int64_t last_step_time = 0;


// ensure we don't send steps after changing dir pin until the proper delay has expired
void waitForDir(){
  if(gs.stepper.dir_has_changed){
    while(gs.stepper.dir_has_changed && ((gs.stepper.dir_change_timer + 5) - esp_timer_get_time() > 0)){
        gs.stepper.dir_has_changed = false;
        if(!gs.stepper.dir){
          // reset stuff for dir changes guard against swapping when we just moved
          if(gs.mygear.jumps.last < encoder.pulse_counter){
            gs.mygear.jumps.prev = gs.mygear.jumps.last;
            gs.mygear.jumps.last = gs.mygear.jumps.next;
          }

        }else{
          // reset stuff for dir changes
          if(gs.mygear.jumps.last  > encoder.pulse_counter){
            gs.mygear.jumps.next = gs.mygear.jumps.last;
            gs.mygear.jumps.last = gs.mygear.jumps.prev;
          }
        }
    }
  }
}


// clean up vars on finish

void finish_jog(){
  if(rapiding){
    pitch = oldPitch;
    pos_feeding = false;
    rapiding = false;
  }else{
    jogging = false;
    pos_feeding = false;
  }
}


/*     
  this decides which direction to move the stepper and if it has reached the 
target position or if it has reached a virtual stop.  it also decides when
to calculate the next set of jumps (positions to step based on ratio/factor)

  this requires that everything is setup and ready to go: the vars that need to be set are:

  mc.moveDirection: the intended direction to feed
    if true: 
      mc.stopNeg : the current tool position, this ensures if the spindle is reversed we will not jog beyond this point
      mc.stopPos : the target postion set from the "config" doc 
    if false:
      swap Neg/Pos.  Pos becomes the current position, Neg the target
  mc.syncMoveStart: if true this will ensure we start motion at spindle position 0 (the starting spindle angle)
*/


void do_pos_feeding(){

    // Sanity check
    //  make sure we are not getting a huge jump in encoder values

    // only read this once for the whole loop
    //
    int64_t pulse_counter = encoder.getCount();

    if(pulse_counter > gs.mygear.jumps.next+1 || pulse_counter < gs.mygear.jumps.prev -1){
      sprintf(err,"Tool outside expected range.  encPos: %lld next: %i  pos %i dirChang? %i",
        pulse_counter,
        gs.mygear.jumps.next,
        gs.mygear.jumps.prev,
        gs.mygear.is_setting_dir);
      el.addMsg(err);
      el.hasError = true;
      pos_feeding = false;
      return;
    }

    // Deal with direction changes
    // Encoder decrementing
    if(!encoder.dir){ // dir neg and not pausing for the direction change
      if(feeding_ccw && mc.moveDirection && gs.stepper.dir){
        gs.stepper.setDir(false);
      }else if(feeding_ccw && !mc.moveDirection && !gs.stepper.dir){
        gs.stepper.setDir(true);
      }
      
      // reverse spindle case 
      else if(!feeding_ccw && !mc.moveDirection && gs.stepper.dir){
        gs.stepper.setDir(false);
      }else if(!feeding_ccw && mc.moveDirection && !gs.stepper.dir){
        gs.stepper.setDir(true);
      }
      waitForDir();
    }else {

    // encoder incrementing
      if(feeding_ccw && mc.moveDirection && !gs.stepper.dir){
        gs.stepper.setDir(true);
      }else if(feeding_ccw && !mc.moveDirection && gs.stepper.dir){
        gs.stepper.setDir(false);
      }
      
     // reverse spindle case 
      else if(!feeding_ccw && !mc.moveDirection && !gs.stepper.dir){
        gs.stepper.setDir(true);
      }else if(!feeding_ccw && mc.moveDirection && gs.stepper.dir){
        gs.stepper.setDir(false);
      }
      waitForDir();  
    } // done with direction changes


      // calculate jumps and delta

      
      if((pulse_counter == gs.mygear.jumps.next) || (pulse_counter== gs.mygear.jumps.prev)){
        gs.mygear.calc_jumps(pulse_counter);
        gs.step();
        /*
        if(gs.stepper.dir){
          gs.stepPos();
        }else{
          gs.stepNeg();
        }
        */
      }

      // evaluate stops, no motion if motion would exceed stops

      if (mc.useStops && mc.moveDirection == true && gs.currentPosition() >= mc.moveSyncTarget){
        ESP_LOGE(TAG,"reached target %d final position was: %d", mc.moveSyncTarget, gs.currentPosition());
        finish_jog();
        return;
      }
      if(mc.useStops && mc.moveDirection == false && gs.currentPosition() <= mc.moveSyncTarget){
        ESP_LOGE(TAG,"reached -target %d final position was: %d", mc.moveSyncTarget, gs.currentPosition());
        finish_jog();
        return;
      }

      if(mc.useStops && (mc.moveSyncTarget < mc.stopNeg)){
        el.addMsg("Tool past stopNeg: HALT");
        el.hasError = true;
        finish_jog();
        return;
      }

      if(mc.useStops && mc.moveSyncTarget > mc.stopPos){
        el.addMsg("tool past stopPos: HALT");
        el.hasError = true;

        // TODO: should we halt or just stop feeding?
        finish_jog();
        return;
      }
}


//  processMotion is called by the encoder class when it gets an update
//  
void IRAM_ATTR processMotion(){

  // check if we want to sync our start position
  if(syncWaiting && pos_feeding){
    // faster to pass this count here or store it in do_pos_feeding?
    if(encoder.getCount() % encoder.start == 0){
      syncWaiting = false;
      gs.init_gear(encoder.getCount());
      do_pos_feeding();
    }
  }
  else if(pos_feeding){
      do_pos_feeding();
    }
    
  
  if(el.hasError){
      el.errorTask();
      el.hasError = false;
    }

}



void init_motion(){
  esp_timer_init();
  //setFactor();
  gs.setELSFactor(pitch);
}
