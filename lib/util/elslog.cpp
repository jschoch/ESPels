#include "elslog.h"
#include <Arduino.h>


#ifdef PIO_UNIT_TESTING

#else
#include "../../src/src/web.h"
#endif

Log::Msg el;

void Log::Msg::maybeSend(){
    Serial.println("MaybeSend borked");
    printf("test");
    /* TODO: figure this out
    #ifndef PIO_UNIT_TESTING
    Serial.println("maybeSend fucker");
    sendLogP(this);
    #endif
    #ifdef PIO_UNIT_TESTING
    //printf("mock send");
    #endif
    */
}

void Log::Msg::error(){
    if(this->t == T::WS){
        printf("\nWS Log: %s\n",this->buf);
        maybeSend();
    }
}

void Log::Msg::error(const char* s){
    if(this->t == T::WS){
        sprintf(buf,s);
        maybeSend();
        printf(s);
    }
}
void Log::Msg::halt(const char* s){
    if(this->t == T::WS){
        sprintf(buf,s);
        maybeSend();
        printf(s);
    }
}

#ifndef PIO_UNIT_TESTING

void Log::Msg::errorTaskImpl(void* _this){
    static_cast<Log::Msg *>(_this)->error();
    vTaskDelete(NULL);
}
void Log::Msg::errorTask(){
    xTaskCreate(this->errorTaskImpl,"errorTask",2048,this, 5,NULL);
}

#endif

void Log::Msg::addMsg(const char* s){
    sprintf(buf,s);
}
