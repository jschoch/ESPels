#include <Arduino.h>
#include <Ticker.h>


int error_led_delay = 100;
int wifi_led_delay = 100;

TaskHandle_t error_th = NULL;
TaskHandle_t wifi_th = NULL;

void error_task(void * parameter){
    while(1){
        digitalWrite(error_led,HIGH);
        delay(error_led_delay);
        digitalWrite(error_led,LOW);
        delay(error_led_delay);
    }
    vTaskDelete( NULL );
}

void my_wifi_task(void * parameter){
    while(1){
        digitalWrite(wifi_led,HIGH);
        delay(wifi_led_delay);
        digitalWrite(wifi_led,LOW);
        delay(wifi_led_delay);
    }
    vTaskDelete( NULL );
}

void startErrorTask(){
    char task1Param[12] = "taskParam";
  /* we create a new task here */
  xTaskCreate(
      error_task,           /* Task function. */
      "errorTask",        /* name of task. */
      1000,                    /* Stack size of task */
      NULL,                     /* parameter of the task */
      1,                        /* priority of the task */
      &error_th);                    /* Task handle to keep track of created task */
}

void startWifiTask(){

  /* let task1 run first then create task2 */
  xTaskCreate(
      my_wifi_task,           /* Task function. */
      "task2",        /* name of task. */
      10000,                    /* Stack size of task */
      NULL,                     /* parameter of the task */
      1,                        /* priority of the task */
      &wifi_th);                    /* Task handle to keep track of created task */
}


void led_init(){
    pinMode(error_led,OUTPUT);
    pinMode(wifi_led,OUTPUT);
    startErrorTask();
    startWifiTask();


}


void set_error_led_blink(int i){
    error_led_delay = i;
}


void set_wifi_led_blink(int i){
    wifi_led_delay = i;
}
