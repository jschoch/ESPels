#include <Arduino.h>
#include "motion.h"
#include "config.h"


hw_timer_t * timer2 = NULL;
volatile SemaphoreHandle_t timer2Semaphore;
portMUX_TYPE timer2Mux = portMUX_INITIALIZER_UNLOCKED;
int timer2tics = 2;
double mmPerStep = 0;
int64_t prevEncPos = 0;
double targetToolRelPosMM = 0.0;
double toolRelPosMM = 0;
bool feeding_left = true;

void init_pos_feed(){
  if(!pos_feeding){

    //  TODO: timer refactor this was moved to controls setFactor()
    /*
    int den = lead_screw_pitch * spindle_encoder_resolution ;
    int nom = motor_steps * pitch;
    Serial.printf("nom: %d den: %d",nom,den);
    if (!xstepper.gear.setRatio(nom,den)){
      // TODO:  send error to GUI
      Serial.println(" ratio no good!!!!  too big!!!!");
      pos_feeding = false;
      return;
    }
    */
    xstepper.gear.is_setting_dir = false;
    xstepper.gear.calc_jumps(0,xstepper.dir);
    xstepper.gear.jumps.last = xstepper.gear.jumps.prev;
    Serial.print(" jumps: ");
    Serial.print(xstepper.gear.jumps.next);
    Serial.print(" prev: ");
    Serial.println(xstepper.gear.jumps.prev);
    // TODO:  DRO object attached to the stepper to track stops and positions?
    pos_feeding = true;
  }else{
  Serial.println("already pos feeding cant' start new feeding");
  }
}

void IRAM_ATTR stepPos(){
  xstepper.step();
  //xstepper.gear.calc_jumps(encPos,true);
  toolPos++;
  toolRelPos++;
  toolRelPosMM += mmPerStep;
}
void IRAM_ATTR stepNeg(){
  xstepper.step();
  //xstepper.gear.calc_jumps(encPos,true);
  toolPos--;
  toolRelPos--;
  toolRelPosMM -= mmPerStep;
}

void IRAM_ATTR do_pos_feeding(){
    // read encoder
    int64_t encPos = encoder.getCount();


    // CLUSTERFUCK
    if(encPos < prevEncPos && !xstepper.gear.is_setting_dir){ // && xstepper.dir){
      if(feeding_left && feeding_dir && !xstepper.dir){
          xstepper.setDir(true);
          return;
      }else if(feeding_left && !feeding_dir && xstepper.dir){
        xstepper.setDir(false);
        return;
      }
    }

    if(encPos > prevEncPos && !xstepper.gear.is_setting_dir){   // && !xstepper.dir){
      if(feeding_left && feeding_dir && xstepper.dir){
        xstepper.setDir(false);
        return;
      }else if(feeding_left && !feeding_dir && !xstepper.dir){
        xstepper.setDir(true);
        return;
      }
      //xstepper.setDir(true);
    }

    // nothing to do if the encoder hasn't moved
    if(encPos != prevEncPos || xstepper.gear.is_setting_dir ){

      // Skip a timer tick if we have changed direction
      if(xstepper.gear.is_setting_dir){
        xstepper.gear.is_setting_dir = false;
        if(!xstepper.dir){
          // reset stuff for dir changes guard against swapping when we just moved
          if(xstepper.gear.jumps.last < encPos){
            xstepper.gear.jumps.prev = xstepper.gear.jumps.last;
            xstepper.gear.jumps.last = xstepper.gear.jumps.next;
          }

        }else{
          // reset stuff for dir changes
          if(xstepper.gear.jumps.last  > encPos){
            xstepper.gear.jumps.next = xstepper.gear.jumps.last;
            xstepper.gear.jumps.last = xstepper.gear.jumps.prev;
          }
        }
        return;
      }
      
      // evaluate done?


      // calculate jumps and delta

      // TODO: make next/prev relative to starting position so they don't have to be int64_t
      if(encPos == xstepper.gear.jumps.next){
        xstepper.gear.calc_jumps(encPos,true);
        if(feeding_dir && feeding_left){
          stepNeg();
        }else{
          stepPos();
       }
      }

      if(encPos == xstepper.gear.jumps.prev){
        xstepper.gear.calc_jumps(encPos,true);
          if(feeding_dir && feeding_left){
            stepPos();
          }else{
            stepNeg();
          }
     }
      prevEncPos = encPos;

      // evaluate stops, no motion if motion would exceed stops

      // handle feeding_dir

      // handle dir change

      // evaluate done?
      if (feeding_dir == zNeg && toolRelPosMM >= targetToolRelPosMM){
      
        // TODO: need to tell everything we are done e.g move the lever to neutral!
        //el.error("Tool reached target");
        el.addMsg("Tool reached Pos target");
        el.hasError = true;
        pos_feeding = false;
        return;
      }
      if(feeding_dir == zPos && toolRelPosMM <= targetToolRelPosMM){
        // TODO: need to tell everything we are done e.g move the lever to neutral!
        //el.error("Tool reached target");
        el.addMsg("Tool reached Neg target");
        el.hasError = true;
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

      
    }// guard for encPos != pevEncPos

}
void IRAM_ATTR onTimer2(){
  portENTER_CRITICAL_ISR(&timer2Mux);

  if(pos_feeding){
    do_pos_feeding();
  }
  portEXIT_CRITICAL_ISR(&timer2Mux);
  // Give a semaphore that we can check in the loop
  xSemaphoreGiveFromISR(timer2Semaphore, NULL);
  // It is safe to use digitalRead/Write here if you want to toggle an output
  if(el.hasError){
    el.errorTask();
    el.hasError = false;
  }
  
}


void init_motion(){
  setFactor();


  // Setup timer to check encoder and decide if steps are needed
  timer2Semaphore = xSemaphoreCreateBinary();

  // divisor 400 should run this loop at 200 khz and should be plenty for the encoder signals.
  // Assuming an encoder ppr of 1024 and a speed of 5000 RPM the encoder frequency shoudl be 
  // generating pulses at 85khz.  timerticks = 2 runs at 100khz.

  timer2 = timerBegin(1, 400, true);
  timerAttachInterrupt(timer2, &onTimer2, true);

  // wait in us
  timerAlarmWrite(timer2, timer2tics, true);
  timerAlarmEnable(timer2);
}