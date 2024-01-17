#include "Encoder.h"
#include "Stepper.h"
#include "motion.h"
#include "config.h"
#include "state.h"

//Encoder encoder = Encoder(EA, EB, 600);
// Removing PPR, confusing...
Encoder encoder = Encoder(EA,EB);
volatile int64_t spindlePos = 0;
volatile int vEncSpeed = 0;
volatile bool vEncStopped = true;

int64_t last_count = 0;

Neotimer rpm_timer = Neotimer(1000);

int virtEncoderCount = 0;
bool virtEncoderEnable = false;
bool virtEncoderDir = true;

void IRAM_ATTR doA(){encoder.handleA();}
void IRAM_ATTR doB(){encoder.handleB();}

static int64_t pulse_start = 0;
static int64_t pulse_stop = 0;
static int64_t total_times = 0;
double avg_times = 0;
static int64_t timerdelta = 0;
int runs = 0;
static const char* TAG_a  = "T";



void init_encoder(){
  encoder.init();
  encoder.enableInterrupts(doA, doB); 
}

void do_rpm(){
  if(rpm_timer.repeat()){
    // TODO: put this in the webUI
    int64_t this_count = encoder.getCount();
    int count_diff = abs(last_count - this_count);
    rpm = (count_diff * 60)/gs.c.spindle_encoder_resolution;
    last_count = this_count;
    //Serial.printf(" rpms: %f count_diff: %d\n",rpm,count_diff);
  }
}

TaskHandle_t vEncHandle = NULL;

void vEncTask(void *param){
  for(;;){
    if(vEncSpeed > 0){
      for(int i =0;i<11;i++){
        encoder.setCount((encoder.pulse_counter+ 1));
        taskYIELD();
      }
      
    }else{
      for(int i =0;i<11;i++){
        encoder.setCount((encoder.pulse_counter- 1)); 
        taskYIELD();
      }
      
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

void maybeProcessMotion(){
  if(MOTION_MODE == 1){
    processMotion();
  }

  #ifndef MOTION_MODE
      #error "YOU MUST DEFINE either  MODE_MOTION_INTERRUPT or MOTION_MODE_TIMER or this can't run"
  #endif
}



// B channel
void IRAM_ATTR Encoder::handleB() {
  int B = digitalRead(pinB);
  switch (quadrature){
    case Quadrature::ON:
      if ( B != B_active ) {
        dir = (A_active != B_active);
        pulse_counter += dir ? 1 : -1;
        B_active = B;
        maybeProcessMotion();
      }
      break;
    case Quadrature::OFF:
      if(B && !digitalRead(pinA)){
        pulse_counter--;
        dir = false;
        maybeProcessMotion();
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
      if ( A != A_active ) {
        dir = (A_active == B_active);
        pulse_counter += dir ? 1 : -1;
        A_active = A;

        maybeProcessMotion();
      }
      break;
    case Quadrature::OFF:
      if(A && !digitalRead(pinB)){
        pulse_counter++;
        dir = true;
        maybeProcessMotion();
      }
      break;
  }
}

Encoder::Encoder(int _encA, int _encB ){

  // Encoder measurement structure init
  // hardware pins
  pinA = _encA;
  pinB = _encB;
  // counter setup
  pulse_counter = 0;
  pulse_timestamp = 0;

  dir = true;
  start = 1;

  A_active = 0;
  B_active = 0;

  // velocity calculation variables
  prev_Th = 0;
  pulse_per_second = 0;
  prev_pulse_counter = 100;
  prev_timestamp_us = esp_timer_get_time();//_micros();

  //pullup = ENCODER_PULLUP;
  pullup = EXTERN;
  // enable quadrature encoder by default
  quadrature = Quadrature::ON;
}

double Encoder::getAngle(){
  // let the UI do this!!!
  return  natural_direction * _2PI * (pulse_counter) / ((double)cpr);
}
// initialize counter to zero
// TODO: not implmeented, need some nottion of work coordinates developed
double Encoder::initRelativeZero(){
  long angle_offset = -pulse_counter;
  pulse_counter = 0;
  return _2PI * (angle_offset) / ((double )cpr);
}

void Encoder::init(){

  // Encoder - check if pullup needed for your encoder
  if(pullup == Pullup::INTERN_PULLUP){
    Serial.println("Using Internal PULLUP");
    pinMode(pinA, INPUT_PULLUP);
    pinMode(pinB, INPUT_PULLUP);
    //if(hasIndex()) pinMode(index_pin,INPUT_PULLUP);
  }else if(pullup == Pullup::INTERN_PULLDOWN){
    Serial.println("Using Internal PULLDOWN");
    pinMode(pinA, INPUT_PULLDOWN);
    pinMode(pinB, INPUT_PULLDOWN);
  }else
  {
    Serial.println("Using External PULLUP/PULLDOWN");
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

  cpr = gs.c.spindle_encoder_resolution;
  start = cpr;

}


void Encoder::enableInterrupts(void (*doA)(), void(*doB)()){
  // attach interrupt if functions provided
  // pinning to core 0 is slow and creates jitter
  attachInterrupt(digitalPinToInterrupt(encoder.pinA), doA, CHANGE);
  attachInterrupt(digitalPinToInterrupt(encoder.pinB), doB, CHANGE);
}

void IRAM_ATTR Encoder::setCount(int64_t count){
  pulse_counter = count;
  maybeProcessMotion();
}
int64_t  IRAM_ATTR Encoder::getCount(){
  // TODO: why not just read the value? what are the speed implications?
  return pulse_counter;
}


// benchmarking crap

void IRAM_ATTR statTask(void *ptr){
  // do stuff
  if(runs < 10000){
    timerdelta = pulse_stop - pulse_start;
    total_times = total_times + timerdelta;
    runs++;
  }else{
    avg_times = (double)(total_times / runs);
    startPrintTask();
    runs = 0;
    total_times = 0;
  }
  vTaskDelete( NULL );
}

void IRAM_ATTR printTask(void *ptr){
  ESP_LOGE(TAG_a,"\n\t avg: %lf  start: %lld : stop %lld\n",avg_times,pulse_start,pulse_stop);
  vTaskDelete( NULL);
}

void startStatTask(){
  xTaskCreate(
                    statTask,             /* Task function. */
                    "statTask",           /* String with name of task. */
                    1000,                     /* Stack size in words. */
                    NULL,      /* Parameter passed as input of the task */
                    3,                         /* Priority of the task. */
                    NULL); 
}

void startPrintTask(){
  xTaskCreate(
                    printTask,             /* Task function. */
                    "printTask",           /* String with name of task. */
                    10000,                     /* Stack size in words. */
                    NULL,      /* Parameter passed as input of the task */
                    1,                         /* Priority of the task. */
                    NULL); 
}

