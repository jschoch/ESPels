#include <Arduino.h>
#include "motion.h"

#include "config.h"

#include "gear.h"
// should just be using accel/decel calcs
#include "AccelStepper.h"
#include "Encoder.h"
#include "Stepper.h"
#include "state.h"
#include "log.h"
#include "Controls.h"
#include "Machine.h"

volatile bool feeding = false;

//TODO there's a direction enum that can be used, maybe do it at the same time the state machine is refactored
volatile bool feeding_ccw = true;

volatile int64_t thetimes = 0;
volatile int badTicks = 0;
char err[200] = "";

int perfCount = 0;
Gear::State gear;

void init_gear(){
    gear.is_setting_dir = false;
    gear.calc_jumps(encoder.getCount(),true);
    gear.jumps.last = gear.jumps.prev;
}

void init_pos_feed(){
  setFactor();
  if(!pos_feeding){
    
    //wait for the start to come around
    if(syncStart){
      Serial.println("waiting for spindle sync");
      syncWaiting = true;
      pos_feeding = true;
    }
    else{
      // this resets the "start" but i'm not sure if it works correctly
      // TODO:  should warn that turning off sync will  loose the "start"
      Serial.println("jog without spindle sync");
      init_gear();
      pos_feeding = true;
    }
  }else{
  el.error("already started pos_feeding, can't do it again");
  }
}
void init_hob_feed(){
  setHobbFactor();
  if(!pos_feeding){
    
    //wait for the start to come around
    if(syncStart){
      Serial.println("waiting for spindle sync");
      syncWaiting = true;
      pos_feeding = true;
    }
    else{
      // this resets the "start" but i'm not sure if it works correctly
      // TODO:  should warn that turning off sync will  loose the "start"
      Serial.println("jog without spindle sync");
      init_gear();
      pos_feeding = true;
    }
  }else{
  el.error("already started pos_feeding, can't do it again");
  }
}

int64_t last_step_time = 0;
int aSteps = 0;
volatile bool useAccel = false;
float acceleration = 10;


// TODO:
/*
  Need to know how far we need to go and decide if we should accel or decel
*/
bool calcAccelDelay(){
  // TODO: need to simplify to get accel working
  if(useAccel){
    aSteps += 1;
    float n = 1000000 / sqrt(2 * aSteps * acceleration);
    if(esp_timer_get_time() - last_step_time >= n){
      return true;
    }else{
      return false;
    }
  }else{
    return true;
  }
}

void stepPos(){
  if(calcAccelDelay()){
    zstepper.step();
    toolPos++;
    toolRelPos++;
    toolRelPosMM += mmPerStep;
    last_step_time = esp_timer_get_time();
  }
}

void stepNeg(){
  zstepper.step();
  toolPos--;
  toolRelPos--;
  toolRelPosMM -= mmPerStep;
  last_step_time = esp_timer_get_time();
}

// ensure we don't send steps after changing dir pin until the proper delay has expired
void waitForDir(){
  if(zstepper.dir_has_changed){
    while(zstepper.dir_has_changed && ((zstepper.dir_change_timer + 5) - esp_timer_get_time() > 0)){
        zstepper.dir_has_changed = false;
        if(!zstepper.dir){
          // reset stuff for dir changes guard against swapping when we just moved
          if(gear.jumps.last < encoder.pulse_counter){
            gear.jumps.prev = gear.jumps.last;
            gear.jumps.last = gear.jumps.next;
          }

        }else{
          // reset stuff for dir changes
          if(gear.jumps.last  > encoder.pulse_counter){
            gear.jumps.next = gear.jumps.last;
            gear.jumps.last = gear.jumps.prev;
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

  z_feeding_dir : the intended direction to feed
    if true: 
      stopNeg : the current tool position, this ensures if the spindle is reversed we will not jog beyond this point
      stopPos : the target postion set from the "config" doc 
    if false:
      swap Neg/Pos.  Pos becomes the current position, Neg the target
  syncStart : if true this will ensure we start motion at spindle position 0 (the starting spindle angle)
*/


void do_pos_feeding(){

    // Sanity check
    //  make sure we are not getting a huge jump in encoder values

    // only read this once for the whole loop
    //
    int64_t pulse_counter = encoder.getCount();

    if(pulse_counter > gear.jumps.next+1 || pulse_counter < gear.jumps.prev -1){
      sprintf(err,"Tool outside expected range.  encPos: %lld next: %i  pos %i dirChang? %i",
        pulse_counter,
        gear.jumps.next,
        gear.jumps.prev,
        gear.is_setting_dir);
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

      
      if((pulse_counter == gear.jumps.next) || (pulse_counter== gear.jumps.prev)){
        gear.calc_jumps(pulse_counter,true);

        if(zstepper.dir){
          stepPos();
        }else{
          stepNeg();
        }
      }

      // evaluate stops, no motion if motion would exceed stops

      if (useStops && z_feeding_dir == true && toolRelPosMM >= targetToolRelPosMM){
        finish_jog();
        return;
      }
      if(useStops && z_feeding_dir == false && toolRelPosMM <= targetToolRelPosMM){
        finish_jog();
        return;
      }

      if(useStops && (toolRelPosMM < stopNeg)){
        el.addMsg("Tool past stopNeg: HALT");
        el.hasError = true;
        finish_jog();
        return;
      }

      if(useStops && toolRelPosMM > stopPos){
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
      init_gear();
      do_pos_feeding();
    }
  }
  else if(pos_feeding){
      do_pos_feeding();
    }
    
    /*
    int64_t startTime = esp_timer_get_time();
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
    perfCount++;
    */

  //thetimes += esp_timer_get_time() - startTime;
  
  if(el.hasError){
      el.errorTask();
      el.hasError = false;
    }

}


/*  
This didn't workout for rapids

void do_rapid(void * param){
  //while(stepper.currentPosition() != rapid_target){
  //}
  rapiding = false;
  vTaskDelete(NULL);
}

void start_rapid(double distance){
  rapiding = true; 
  //rapid_target = stepper.currentPosition() + (stepsPerMM * distance);
  //stepper.runToNewPosition(stepper.currentPosition()+(stepsPerMM * distance));
  //stepper
  xTaskCreatePinnedToCore(
    do_rapid,    // Function that should be called
    "rapid step",  // Name of the task (for debugging)
    2000,            // Stack size (bytes)
    NULL,            // Parameter to pass
    1,               // Task priority
    NULL,             // Task handle
    0 // pin to core 0, arduino loop runs core 1
);

}
*/

void init_motion(){
  esp_timer_init();
  setFactor();
}
