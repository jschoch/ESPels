#include "log.h"

Log::Msg el;

void Log::Msg::error(){
    Serial.println("error");
    //sendLog(this);
    if(this->t == T::WS){
        Serial.print("\nWS Log: ");
        Serial.println(this->buf);
        sendLogP(this);
    }
}

void Log::Msg::error(const char* s){
    if(this->t == T::WS){
        sprintf(buf,s);
        sendLogP(this);
        Serial.println(s);
    }
}

void Log::Msg::errorTaskImpl(void* _this){
    //(Log::Msg)_this->error();
    static_cast<Log::Msg *>(_this)->error();
    vTaskDelete(NULL);
}
void Log::Msg::errorTask(){
    xTaskCreate(this->errorTaskImpl,"errorTask",2048,this, 5,NULL);
}


void Log::Msg::addMsg(const char* s){
    sprintf(buf,s);
}