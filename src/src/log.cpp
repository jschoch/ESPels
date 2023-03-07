#include "log.h"

Log::Msg el;

void Log::Msg::maybeSend(){
    #ifndef UNIT_TEST
    sendLogP(this);
    #endif
    #ifdef UNIT_TEST

    #endif
}

void Log::Msg::error(){
    if(this->t == T::WS){
        Serial.print("\nWS Log: ");
        Serial.println(this->buf);
        maybeSend();
    }
}

void Log::Msg::error(const char* s){
    if(this->t == T::WS){
        sprintf(buf,s);
        maybeSend();
        Serial.println(s);
    }
}
void Log::Msg::halt(const char* s){
    if(this->t == T::WS){
        sprintf(buf,s);
        maybeSend();
        Serial.println(s);
    }
}

void Log::Msg::errorTaskImpl(void* _this){
    static_cast<Log::Msg *>(_this)->error();
    vTaskDelete(NULL);
}
void Log::Msg::errorTask(){
    xTaskCreate(this->errorTaskImpl,"errorTask",2048,this, 5,NULL);
}


void Log::Msg::addMsg(const char* s){
    sprintf(buf,s);
}
