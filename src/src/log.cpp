#include "log.h"

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