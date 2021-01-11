#include <Arduino.h>
#include "motion.h"
#include "config.h"

double mmPerStep = 0;
volatile double targetToolRelPosMM = 0.0;
volatile double toolRelPosMM = 0;
volatile int64_t thetimes = 0;
volatile int badTicks = 0;
char err[200] = "";
bool syncStart = true;
bool syncWaiting = false;


// TODO this should be set via gui or btns
volatile bool feeding_ccw = true;
volatile bool tick = false;

int perfCount = 0;

void init_gear(){
    zstepper.gear.is_setting_dir = false;
    zstepper.gear.calc_jumps(encoder.getCount(),true);
    zstepper.gear.jumps.last = zstepper.gear.jumps.prev;


}


// this was too slow
void waitForSyncStart(void * param){
  while(encoder.pulse_counter % encoder.start != 0){
    // this may be too slow
    vTaskDelay(1);
  }
  init_gear();
  pos_feeding = true;
  syncWaiting = false;
  updateStatusDoc();
  vTaskDelete(NULL);
}


void init_pos_feed(){
  if(!pos_feeding){
    
    //wait for the start to come around
    if(syncStart){
      syncWaiting = true;
      //xTaskCreate(waitForSyncStart,"syncTask",2048,NULL, 2,NULL);
      pos_feeding = true;
    }
    else{
      init_gear();
       pos_feeding = true;
    }
  }else{
  el.error("already started pos_feeding, can't do it again");
  }
}

void stepPos(){
  zstepper.step();
  toolPos++;
  toolRelPos++;
  toolRelPosMM += mmPerStep;
}
void stepNeg(){
  zstepper.step();
  toolPos--;
  toolRelPos--;
  toolRelPosMM -= mmPerStep;
}

void waitForDir(){
  if(zstepper.gear.is_setting_dir){
    while(zstepper.gear.is_setting_dir && ((zstepper.dir_change_timer + 5) - esp_timer_get_time() > 0)){
        zstepper.gear.is_setting_dir = false;
        if(!zstepper.dir){
          // reset stuff for dir changes guard against swapping when we just moved
          if(zstepper.gear.jumps.last < encoder.pulse_counter){
            zstepper.gear.jumps.prev = zstepper.gear.jumps.last;
            zstepper.gear.jumps.last = zstepper.gear.jumps.next;
          }

        }else{
          // reset stuff for dir changes
          if(zstepper.gear.jumps.last  > encoder.pulse_counter){
            zstepper.gear.jumps.next = zstepper.gear.jumps.last;
            zstepper.gear.jumps.last = zstepper.gear.jumps.prev;
          }
        }
    }
  }
}

void do_pos_feeding(){

    // Sanity check
    //  make sure we are not getting a huge jump in encoder values

    if(encoder.pulse_counter > zstepper.gear.jumps.next+1 || encoder.pulse_counter < zstepper.gear.jumps.prev -1){
      sprintf(err,"Tool outside expected range.  encPos: %lld next: %i  pos %i dirChang? %i",
        encoder.pulse_counter,
        zstepper.gear.jumps.next,
        zstepper.gear.jumps.prev,
        zstepper.gear.is_setting_dir);
      el.addMsg(err);
      el.hasError = true;
      pos_feeding = false;
      return;
    }

    // Deal with direction changes
    // Encoder decrementing
    if(!encoder.dir){ // dir neg and not pausing for the direction change
      if(feeding_ccw && z_feeding_dir && zstepper.dir){
        zstepper.setDir(false);
      }else if(feeding_ccw && !z_feeding_dir && !zstepper.dir){
        zstepper.setDir(true);
      }
      
      // reverse spindle case 
      else if(!feeding_ccw && !z_feeding_dir && zstepper.dir){
        zstepper.setDir(false);
      }else if(!feeding_ccw && z_feeding_dir && !zstepper.dir){
        zstepper.setDir(true);
      }
      waitForDir();
    }else {

    // encoder incrementing
    //if((encoder.dir) && !zstepper.gear.is_setting_dir){   //
      if(feeding_ccw && z_feeding_dir && !zstepper.dir){
        zstepper.setDir(true);
      }else if(feeding_ccw && !z_feeding_dir && zstepper.dir){
        zstepper.setDir(false);
      }
      
     // reverse spindle case 
      else if(!feeding_ccw && !z_feeding_dir && !zstepper.dir){
        zstepper.setDir(true);
      }else if(!feeding_ccw && z_feeding_dir && zstepper.dir){
        zstepper.setDir(false);
      }
      waitForDir();  
    }

      // calculate jumps and delta

      //if(encoder.pulse_counter== zstepper.gear.jumps.next ){
      if((encoder.pulse_counter == zstepper.gear.jumps.next) || (encoder.pulse_counter== zstepper.gear.jumps.prev)){
        zstepper.gear.calc_jumps(encoder.pulse_counter,true);

        if(zstepper.dir){
          stepPos();
        }else{
          stepNeg();
        }
      }


      /*
      // Encoder decreasing
      if(encoder.pulse_counter == zstepper.gear.jumps.prev){
        zstepper.gear.calc_jumps(encoder.pulse_counter,true);
     }
      */

      // evaluate stops, no motion if motion would exceed stops

      if (z_feeding_dir == true && toolRelPosMM >= targetToolRelPosMM){
      
        // TODO: need to tell everything we are done e.g move the lever to neutral!
        // popup here is annoying maybe log a msg
        //el.error("Tool reached target");
        //el.addMsg("Tool reached Pos target");
        //el.hasError = true;
        pos_feeding = false;
        return;
      }
      if(z_feeding_dir == false && toolRelPosMM <= targetToolRelPosMM){
        // TODO: need to tell everything we are done e.g move the lever to neutral!
        // popup annoying
        //el.addMsg("Tool reached Neg target");
        //el.hasError = true;
        pos_feeding = false;
        return;
      }

      if(toolRelPosMM < stopNeg){
        el.addMsg("Tool past stopNeg: HALT");
        el.hasError = true;
        pos_feeding = false;
      }

      if(toolRelPosMM > stopPos){
        el.addMsg("tool past stopPos: HALT");
        el.hasError = true;

        // TODO: should we halt or just stop feeding?
        pos_feeding = false;
      }

      

}
void IRAM_ATTR processMotion(){

  //int64_t startTime = esp_timer_get_time();
  if(syncWaiting && pos_feeding){
    if(encoder.pulse_counter % encoder.start == 0){
      syncWaiting = false;
      init_gear();
      do_pos_feeding();
    }
  }
  else if(pos_feeding){
      do_pos_feeding();
    }
    
    /*
    if(perfCount >= 100000){
      int avgTime = thetimes / 100000;
      sprintf(err,"Time took %i badTicks was %i\n",avgTime,badTicks);
      el.addMsg(err);
      el.hasError = true;
      perfCount = 0;
      badTicks = 0;
      thetimes = 0;
      startTime = esp_timer_get_time();
    }
    */
    tick = false;

  //thetimes += esp_timer_get_time() - startTime;
  perfCount++;
  if(el.hasError){
      el.errorTask();
      el.hasError = false;
    }

}


void init_motion(){
  esp_timer_init();
  setFactor();



}