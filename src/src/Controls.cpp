#include "Controls.h"

#include "config.h"

#define BOUNCE_LOCK_OUT
#include <Bounce2.h>
#include "neotimer.h"
#include <yasm.h>
#include "rmtStepper.h"

//Neotimer button_read_timer = Neotimer(10);
Neotimer button_print_timer = Neotimer(2000);
Neotimer dro_timer = Neotimer(600);
Log::Msg el;


uint8_t menu = 3; 
int pitch_menu= 1;
volatile bool feeding = false;
volatile bool z_feeding_dir = true;



int feed_mode_select = 0;

#ifdef DEBUG_CPU_STATS
char stats[ 2048];
#undef configGENERATE_RUN_TIME_STATS
#define configGENERATE_RUN_TIME_STATS 1
#undef configUSE_STATS_FORMATTING_FUNCTIONS
#define configUSE_STATS_FORMATTING_FUNCTIONS 1
#endif

YASM btn_yasm;


enum class BtnState{
  Startup,
  Ready,
  Status,
  Feeding
}; 

BtnState btnState = BtnState::Startup;

// Debouncer

// Buttons Gutted!
/*
Bounce debLBP = Bounce(); 
Bounce debRBP = Bounce();
Bounce debSBP = Bounce();
Bounce debUBP = Bounce();
Bounce debDBP = Bounce();
Bounce debMBP = Bounce();

const int NUM_BUTTONS = 6;



// Button data structs


Bd lbd  {LBP,"left",false,true,true,&debLBP,0};
Bd rbd  {RBP,"right",false,true,true,&debRBP,1};
Bd ubd  {UBP,"up",false,true,true,&debUBP,2};
Bd dbd  {DBP,"down",false,true,true,&debDBP,3};
Bd sbd  {SBP,"menu",false,true,true,&debSBP,4};
Bd mbd  {MBP,"mod",false,true,true,&debMBP,5};

Bd bdata[NUM_BUTTONS] = {
    lbd,
    rbd,
    ubd,
    dbd,
    sbd,
    mbd
  };
*/ //  button gutting

int32_t stepsPerMM = 0;
int32_t relativePosition = 0;
int32_t absolutePosition = 0;


void init_controls(){
  btn_yasm.next(startupState);  

  Serial.println(display_mode);
}



void read_buttons(){
  /*  Buttons gutted
  if(button_read_timer.repeat()){
    for(int i = 0; i < NUM_BUTTONS;i++){
      Bd bd = bdata[i];
      bd.deb->update();
    }
    btn_yasm.run();
  }
  */

  if(button_print_timer.repeat()){
    //debugButtons();
    /*
    Serial.print("enc: ");
    Serial.print((int)encoder.getCount());
    Serial.print(",target:");
    Serial.print(targetToolRelPos);
    Serial.print(",delta: ");
    Serial.print((int)delta);
    Serial.print(",TP: ");
    Serial.print((int)toolPos);
    Serial.print(",calc: ");
    Serial.print((int)calculated_stepper_pulses);
    Serial.print(",feed "); 
    Serial.print(feeding);
    Serial.print(",jogging: ");
    Serial.print(jogging);
    Serial.print(",jog_done: ");
    Serial.println(jog_done);
    */
    Serial.print("%");
    updateStatusDoc();

  }
  if(dro_timer.repeat()){
    updateStatusDoc();
  }
     
}

void updateMode(display_mode_t newDisplayMode,RunMode run){
  Serial.print("updating modes: ");
  Serial.print(display_mode);
  Serial.print(":");
  Serial.println((int)run);
  display_mode = newDisplayMode;
  run_mode = run;
  updateStateDoc();
}

void startupState(){
  if(btn_yasm.isFirstRun()){
    updateMode(STARTUP,RunMode::STARTUP);
    web = true;
  }
}

void slaveJogReadyState(){
  if(btn_yasm.isFirstRun()){
    updateMode(READY,RunMode::SLAVE_JOG_READY);
    setFactor();
    web = true;
  }
}

/*

When in slaveJogReadyState if the webUI sends the "do jog" this state is entered.
Display should prompt user to press a button to execute the jog
*/
void slaveJogPosState(){
  if(btn_yasm.isFirstRun()){
    // TODO update the modes correctly
    //updateMode()

    // TODO: should the webui be disabled?
    // web = false;
    Serial.println("entering slaveJogPosState() press sbd to start jogging");
    //setFactor();
  }
}

// This is "slave jog" status mode, "slave" status is in SlaveMode.cpp
void slaveJogStatusState(){
  if(btn_yasm.isFirstRun()){
    updateMode(DSTATUS,RunMode::RUNNING);
    web = false;
  }
}



void setFactor(){

  // TODO: timer refactor  get rid of factor and all related code
  //factor= (motor_steps*pitch)/(lead_screw_pitch*spindle_encoder_resolution);            
  stepsPerMM = motor_steps / lead_screw_pitch;
  mmPerStep = (double) 1/stepsPerMM;
  int den = lead_screw_pitch * spindle_encoder_resolution ;
  int nom = motor_steps * pitch;

  Serial.printf("nom: %d den: %d",nom,den);
    if (!zstepper.gear.setRatio(nom,den)){
      sprintf(el.buf,"Bad Ratio: Den: %d Nom: %d\n",nom,den);
      el.error();
      pitch = oldPitch;
      return;
    }

  /*   TODO: add back imperial threads
  else
    {
    if(menu<20)
      {
        // the depth of cut in mm on the compound slide I need for each thread pitch.  
        // I use this during operation rather than looking it up each time

        depth=pitch_factor*25.4/tpi;

        // the imperial factor needed to account for details of lead screw pitch, 
        // stepper motor #pulses/rev and encoder #pulses/rev
        factor= motor_steps*25.4/(tpi*lead_screw_pitch*spindle_encoder_resolution);  // imperial
        }
      else
        {
        // the depth of cut in mm on the compound slide
        depth=pitch_factor*pitch; 
        // the metric factor needed to account for details of lead screw pitch, 
        // stepper motor #pulses/rev and encoder #pulses/rev
        factor=pitch*motor_steps/(lead_screw_pitch*spindle_encoder_resolution); // keep redundant
        }
      }
  */              
} 

//this defines the parameters for the thread and turning for both metric and imperial threads

void thread_parameters()                                           
  { 
    /* TODO: add back when you work on thread cycle
  switch(menu) {
    case(1):     pitch=0.085;                  break;  // Normal Turning
  */
}

void feed_parameters(){
  
  if(pitch_menu> 21) pitch_menu= 1;

  // do not wrap!!
  if(pitch_menu< 1) pitch_menu= 1;

  switch(pitch_menu) {
    case(1):     pitch=0.05;                  break;  // Normal Turning
    case(2):     pitch=0.085;                  break;  // Fine Turning
    case(3):     pitch=0.16;                  break;  // Coarse Turning
    case(4):     pitch=0.2;                  break;  // Coarse Turning
    case(5):    pitch=0.4;   break;      
    case(6):    pitch=0.4;   break;      
    case(7):    pitch=0.5;   break;      
    case(8):    pitch=0.7;   break;      
    case(9):    pitch=0.75;  break;      
    case(10):    pitch=0.8;   break;      
    case(11):    pitch=1.0;   break;      
    case(12):    pitch=1.25;  break;      
    case(13):    pitch=1.5;   break;      
    case(14):    pitch=1.75;  break;      
    case(15):    pitch=2.0;   break;      
    case(16):    pitch=2.5;   break;      
    case(17):    pitch=3.0;   break;      
    case(18):    pitch=3.5;   break;      
    case(19):    pitch=4.0;   break;      
    case(20):    pitch=5.0;   break;
    case(21):    pitch=7.0;   break;
  }

  setFactor();
  // TODO:  this needs some thought
  updateStateDoc();
}
