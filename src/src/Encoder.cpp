#include "Encoder.h"
#include "Stepper.h"
#include "motion.h"

volatile int64_t spindlePos = 0;

int spindle_encoder_resolution=2400 ;
int64_t last_count = 0;
int rpm = 0;

Neotimer rpm_timer = Neotimer(100);

int virtEncoderCount = 0;
bool virtEncoderEnable = false;
bool virtEncoderDir = true;
// encoder instance
// Pins defines in config.h
Encoder encoder = Encoder(EA, EB, 600);

void IRAM_ATTR doA(){encoder.handleA();}
void IRAM_ATTR doB(){encoder.handleB();}

void init_encoder(){
  encoder.init();
  encoder.enableInterrupts(doA, doB); 
}

void do_rpm(){
  if(rpm_timer.repeat()){
    // TODO: put this in the webUI
    long count_diff = abs(last_count - encoder.pulse_counter);
    double revolutions = (double) count_diff / spindle_encoder_resolution;
    rpm = revolutions * 10 * 60;
    last_count = encoder.pulse_counter;    
  }
}

TaskHandle_t vEncHandle = NULL;
volatile bool vEncStopped = true;

void vEncTask(void *param){
  for(;;){
    if(vEncSpeed > 0){
      encoder.setCount((encoder.pulse_counter+ 1));
    }else{
      encoder.setCount((encoder.pulse_counter- 1));
    }
    vTaskDelay(vEncSpeed/portTICK_PERIOD_MS);
  }
}

void stopVenc(){
  if(vEncHandle != NULL) {
    Serial.println("stopping venc");
    vTaskDelete(vEncHandle);
    vEncStopped = true;
  }
}

void startVenc(){
  Serial.println("Starting venc");
  xTaskCreate(
    vEncTask,
    "vEnc",
    4000,
    NULL,
    1,
    &vEncHandle 
  );
  vEncStopped = false;

}



// B channel
void IRAM_ATTR Encoder::handleB() {
  int B = digitalRead(pinB);
  switch (quadrature){
    case Quadrature::ON:
  //     // CPR = 4xPPR
      if ( B != B_active ) {
        dir = (A_active != B_active);
        pulse_counter += dir ? 1 : -1;
        pulse_timestamp = esp_timer_get_time();// _micros();
        B_active = B;
        processMotion();
      }
      break;
    case Quadrature::OFF:
      // CPR = PPR
      if(B && !digitalRead(pinA)){
        pulse_counter--;
        dir = false;
        pulse_timestamp =  esp_timer_get_time();//_micros();
        processMotion();
      }
      break;
  }
}

//  Encoder interrupt callback functions
// A channel
void IRAM_ATTR Encoder::handleA() {
  int A = digitalRead(pinA);
  switch (quadrature){
    case Quadrature::ON:
      // CPR = 4xPPR
      if ( A != A_active ) {
        dir = (A_active == B_active);
        pulse_counter += dir ? 1 : -1;
        pulse_timestamp = esp_timer_get_time();// _micros();
        A_active = A;
        processMotion();
      }
      break;
    case Quadrature::OFF:
      // CPR = PPR
      if(A && !digitalRead(pinB)){
        pulse_counter++;
        dir = true;
        pulse_timestamp = esp_timer_get_time();//_micros();
        processMotion();
      }
      break;
  }
}

Encoder::Encoder(int _encA, int _encB , double _ppr){

  // Encoder measurement structure init
  // hardware pins
  pinA = _encA;
  pinB = _encB;
  // counter setup
  pulse_counter = 0;
  pulse_timestamp = 0;

  dir = true;
  start = 1;

  cpr = _ppr;
  A_active = 0;
  B_active = 0;

  // velocity calculation variables
  prev_Th = 0;
  pulse_per_second = 0;
  prev_pulse_counter = 100;
  prev_timestamp_us = esp_timer_get_time();//_micros();

  // extern pullup as default
  //pullup = Pullup::EXTERN;
  pullup = Pullup::INTERN_PULLDOWN;
  // enable quadrature encoder by default
  quadrature = Quadrature::ON;
}
double Encoder::getAngle(){
  return  natural_direction * _2PI * (pulse_counter) / ((double)cpr);
}
// initialize counter to zero
double Encoder::initRelativeZero(){
  long angle_offset = -pulse_counter;
  pulse_counter = 0;
  pulse_timestamp = esp_timer_get_time();//_micros();
  return _2PI * (angle_offset) / ((double )cpr);
}

void Encoder::init(){

  // Encoder - check if pullup needed for your encoder
  if(pullup == Pullup::INTERN_PULLUP){
    pinMode(pinA, INPUT_PULLUP);
    pinMode(pinB, INPUT_PULLUP);
    //if(hasIndex()) pinMode(index_pin,INPUT_PULLUP);
  }else if(pullup == Pullup::INTERN_PULLDOWN){
    pinMode(pinA, INPUT_PULLDOWN);
    pinMode(pinB, INPUT_PULLDOWN);
  }
  {
    pinMode(pinA, INPUT);
    pinMode(pinB, INPUT);
    //if(hasIndex()) pinMode(index_pin,INPUT);
  }

  // counter setup
  pulse_counter = 0;
  pulse_timestamp =  esp_timer_get_time();//_micros();
  // velocity calculation variables
  prev_Th = 0;
  pulse_per_second = 0;
  prev_pulse_counter = -1;
  prev_timestamp_us = esp_timer_get_time();//_micros();
  dir = true;

  // initial cpr = PPR
  // change it if the mode is quadrature
  if(quadrature == Quadrature::ON) cpr = 4*cpr;
  start = cpr;

}

void startEncInt(void * p){
  switch(encoder.quadrature){
    case Quadrature::ON:
      // A callback and B callback
      attachInterrupt(digitalPinToInterrupt(encoder.pinA), doA, CHANGE);
      attachInterrupt(digitalPinToInterrupt(encoder.pinB), doB, CHANGE);
      break;
    case Quadrature::OFF:
      // A callback and B callback
      attachInterrupt(digitalPinToInterrupt(encoder.pinA), doA, RISING);
      attachInterrupt(digitalPinToInterrupt(encoder.pinB), doB, RISING);
      break;
  }
  vTaskDelete(NULL);
}

// function enabling hardware interrupts of the for the callback provided
// if callback is not provided then the interrupt is not enabled
void Encoder::enableInterrupts(void (*doA)(), void(*doB)()){
  // attach interrupt if functions provided

  // do this in a task so the interrupts get pinned to core 0
  xTaskCreatePinnedToCore(
    startEncInt,
    "startEncInt",
    4000,
    NULL,
    1,
    NULL,
    0 
  );
}

void IRAM_ATTR Encoder::setCount(int64_t count){
  // TODO: why do you need these?
  pulse_counter = count;
  processMotion();
}
int64_t  IRAM_ATTR Encoder::getCount(){
  // TODO: why not just read the value?
  return pulse_counter;
}
