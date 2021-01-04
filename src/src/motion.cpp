#include <Arduino.h>
#include "motion.h"
#include "config.h"



hw_timer_t * timer2 = NULL;
volatile SemaphoreHandle_t timer2Semaphore;
portMUX_TYPE timer2Mux = portMUX_INITIALIZER_UNLOCKED;
int timer2tics = 10;
double mmPerStep = 0;
volatile double targetToolRelPosMM = 0.0;
volatile double toolRelPosMM = 0;
volatile int64_t thetimes = 0;
volatile int badTicks = 0;
char err[100] = "";


// TODO this should be set via gui or btns
volatile bool feeding_ccw = true;
volatile bool tick = false;

int perfCount = 0;

void init_pos_feed(){
  if(!pos_feeding){
    xstepper.gear.is_setting_dir = false;
    xstepper.gear.calc_jumps(encoder.getCount(),xstepper.dir);
    xstepper.gear.jumps.last = xstepper.gear.jumps.prev;
    /*
    Serial.print(" jumps: ");
    Serial.print(xstepper.gear.jumps.next);
    Serial.print(" prev: ");
    Serial.println(xstepper.gear.jumps.prev);
    */
    // TODO:  DRO object attached to the stepper to track stops and positions?
    pos_feeding = true;
  }else{
  //Serial.println("already pos feeding cant' start new feeding");
  el.error("already started pos_feeding, can't do it again");
  }
}

void stepPos(){
  xstepper.step();
  toolPos++;
  toolRelPos++;
  toolRelPosMM += mmPerStep;
}
void stepNeg(){
  xstepper.step();
  toolPos--;
  toolRelPos--;
  toolRelPosMM -= mmPerStep;
}

void do_pos_feeding(){

    // Sanity check
    //  make sure we are not getting a huge jump in encoder values
    if(encoder.pulse_counter == encoder.prev_pulse_counter){
      //Serial.println("IMPOSSIBLE");
      encoder.prev_pulse_counter = encoder.prev_pulse_counter;
      return;
    }

    if(encoder.pulse_counter > xstepper.gear.jumps.next+1 || encoder.pulse_counter < xstepper.gear.jumps.prev -1){
      sprintf(err,"Tool outside expected range.  prevEnc %lld encPos: %lld next: %i  pos %i dirChang? %i",
        encoder.prev_pulse_counter,
        encoder.pulse_counter,
        xstepper.gear.jumps.next,
        xstepper.gear.jumps.prev,
        xstepper.gear.is_setting_dir);
      el.addMsg(err);
      el.hasError = true;
      pos_feeding = false;
      return;
    }

    // Deal with direction changes
    // Encoder decrementing
    if((encoder.pulse_counter < encoder.prev_pulse_counter) && !xstepper.gear.is_setting_dir){ // dir neg and not pausing for the direction change
      if(feeding_ccw && z_feeding_dir && xstepper.dir){
        xstepper.setDir(false);
        return;
      }else if(feeding_ccw && !z_feeding_dir && !xstepper.dir){
        xstepper.setDir(true);
        return;
      }else if(!feeding_ccw && !z_feeding_dir && !xstepper.dir){
        xstepper.setDir(true);
        return;
      }else if(!feeding_ccw && z_feeding_dir && xstepper.dir){
        xstepper.setDir(false);
        return;
      }
    }

    // encoder incrementing
    if((encoder.pulse_counter > encoder.prev_pulse_counter) && !xstepper.gear.is_setting_dir){   //
      if(feeding_ccw && z_feeding_dir && !xstepper.dir){
        xstepper.setDir(true);
        return;
      }else if(feeding_ccw && !z_feeding_dir && xstepper.dir){
        xstepper.setDir(false);
        return;
      }else if(!feeding_ccw && !z_feeding_dir && xstepper.dir){
        xstepper.setDir(false);
        return;
      }else if(!feeding_ccw && z_feeding_dir && !xstepper.dir){
        xstepper.setDir(true);
        return;
      }
      //xstepper.setDir(true);
    }


      // Skip a timer tick if we have changed direction
      if(xstepper.gear.is_setting_dir){
        xstepper.gear.is_setting_dir = false;
        if(!xstepper.dir){
          // reset stuff for dir changes guard against swapping when we just moved
          if(xstepper.gear.jumps.last < encoder.pulse_counter){
            xstepper.gear.jumps.prev = xstepper.gear.jumps.last;
            xstepper.gear.jumps.last = xstepper.gear.jumps.next;
          }

        }else{
          // reset stuff for dir changes
          if(xstepper.gear.jumps.last  > encoder.pulse_counter){
            xstepper.gear.jumps.next = xstepper.gear.jumps.last;
            xstepper.gear.jumps.last = xstepper.gear.jumps.prev;
          }
        }
        return;
      }
      
      // evaluate done?


      // calculate jumps and delta

      // TODO: make next/prev relative to starting position so they don't have to be int64_t
      //  Encoder increasing case
      //if(encoder.pulse_counter== xstepper.gear.jumps.next ){
      if((encoder.pulse_counter == xstepper.gear.jumps.next) || (encoder.pulse_counter== xstepper.gear.jumps.prev)){
        xstepper.gear.calc_jumps(encoder.pulse_counter,true);
        //xstepper.gear.calc_jumps(encoder.pulse_counter,feeding_ccw);
        if(feeding_ccw && z_feeding_dir ){
          stepPos();
        }else if(!feeding_ccw && !z_feeding_dir){
          stepNeg();
       }else if(!feeding_ccw && z_feeding_dir){
         stepPos();
       }else{
         stepNeg();
       }
      }


      /*
      // Encoder decreasing
      if(encoder.pulse_counter == xstepper.gear.jumps.prev){
        xstepper.gear.calc_jumps(encoder.pulse_counter,true);
          if(z_feeding_dir){
            stepPos();
          }else{
            stepNeg();
          }
     }
      */

      // evaluate stops, no motion if motion would exceed stops

      // handle feeding_dir

      // handle dir change

      // evaluate done?
      if (z_feeding_dir == true && toolRelPosMM >= targetToolRelPosMM){
      
        // TODO: need to tell everything we are done e.g move the lever to neutral!
        // popup here is annoying
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
        //Log.error();
        //el.error("Tool past stopNeg: HALT");
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
    if(pos_feeding){
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

  encoder.prev_pulse_counter = encoder.pulse_counter;
}


void init_motion(){
  esp_timer_init();
  setFactor();



}