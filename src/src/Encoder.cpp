#include <ESP32Encoder.h>
#include "Encoder.h"
#include "Stepper.h"
#include "motion.h"

volatile int64_t spindlePos = 0;

int spindle_encoder_resolution=2400 ;
int64_t last_count = 0;
int rpm = 0;

//ESP32Encoder encoder;

Neotimer rpm_timer = Neotimer(100);

int virtEncoderCount = 0;
bool virtEncoderEnable = false;
bool virtEncoderDir = true;
// encoder instance
Encoder encoder = Encoder(25, 26, 500);

void doA(){encoder.handleA();}
void doB(){encoder.handleB();}

void init_encoder(){
  encoder.init();
  encoder.enableInterrupts(doA, doB); 
}

void do_rpm(){
  if(rpm_timer.repeat()){
    // TODO: put this in the webUI
    long count_diff = abs(last_count - encoder.pulse_counter);
    float revolutions = (float) count_diff / spindle_encoder_resolution;
    rpm = revolutions * 10 * 60;
    last_count = encoder.pulse_counter;    
  }
}



// B channel
void Encoder::handleB() {
  int B = digitalRead(pinB);
  switch (quadrature){
    case Quadrature::ON:
  //     // CPR = 4xPPR
      if ( B != B_active ) {
        pulse_counter += (A_active != B_active) ? 1 : -1;
        pulse_timestamp = esp_timer_get_time();// _micros();
        B_active = B;
        processMotion();
      }
      break;
    case Quadrature::OFF:
      // CPR = PPR
      if(B && !digitalRead(pinA)){
        pulse_counter--;
        pulse_timestamp =  esp_timer_get_time();//_micros();
        processMotion();
      }
      break;
  }
}

//  Encoder interrupt callback functions
// A channel
void Encoder::handleA() {
  int A = digitalRead(pinA);
  switch (quadrature){
    case Quadrature::ON:
      // CPR = 4xPPR
      if ( A != A_active ) {
        pulse_counter += (A_active == B_active) ? 1 : -1;
        pulse_timestamp = esp_timer_get_time();// _micros();
        A_active = A;
        processMotion();
      }
      break;
    case Quadrature::OFF:
      // CPR = PPR
      if(A && !digitalRead(pinB)){
        pulse_counter++;
        pulse_timestamp = esp_timer_get_time();//_micros();
        processMotion();
      }
      break;
  }
}

Encoder::Encoder(int _encA, int _encB , float _ppr){

  // Encoder measurement structure init
  // hardware pins
  pinA = _encA;
  pinB = _encB;
  // counter setup
  pulse_counter = 0;
  pulse_timestamp = 0;

  cpr = _ppr;
  A_active = 0;
  B_active = 0;

  // velocity calculation variables
  prev_Th = 0;
  pulse_per_second = 0;
  prev_pulse_counter = 0;
  prev_timestamp_us = esp_timer_get_time();//_micros();

  // extern pullup as default
  pullup = Pullup::EXTERN;
  // enable quadrature encoder by default
  quadrature = Quadrature::ON;
}
float Encoder::getAngle(){
  return  natural_direction * _2PI * (pulse_counter) / ((float)cpr);
}
// initialize counter to zero
float Encoder::initRelativeZero(){
  long angle_offset = -pulse_counter;
  pulse_counter = 0;
  pulse_timestamp = esp_timer_get_time();//_micros();
  return _2PI * (angle_offset) / ((float)cpr);
}

void Encoder::init(){

  // Encoder - check if pullup needed for your encoder
  if(pullup == Pullup::INTERN){
    pinMode(pinA, INPUT_PULLUP);
    pinMode(pinB, INPUT_PULLUP);
    //if(hasIndex()) pinMode(index_pin,INPUT_PULLUP);
  }else{
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
  prev_pulse_counter = 0;
  prev_timestamp_us = esp_timer_get_time();//_micros();

  // initial cpr = PPR
  // change it if the mode is quadrature
  if(quadrature == Quadrature::ON) cpr = 4*cpr;

}

// function enabling hardware interrupts of the for the callback provided
// if callback is not provided then the interrupt is not enabled
void Encoder::enableInterrupts(void (*doA)(), void(*doB)()){
  // attach interrupt if functions provided
  switch(quadrature){
    case Quadrature::ON:
      // A callback and B callback
      if(doA != nullptr) attachInterrupt(digitalPinToInterrupt(pinA), doA, CHANGE);
      if(doB != nullptr) attachInterrupt(digitalPinToInterrupt(pinB), doB, CHANGE);
      break;
    case Quadrature::OFF:
      // A callback and B callback
      if(doA != nullptr) attachInterrupt(digitalPinToInterrupt(pinA), doA, RISING);
      if(doB != nullptr) attachInterrupt(digitalPinToInterrupt(pinB), doB, RISING);
      break;
  }
}

void Encoder::setCount(int64_t count){
  pulse_counter = count;
  processMotion();
}
int64_t Encoder::getCount(){
  return pulse_counter;
}
