#pragma once

#include <ArduinoJson.h>


bool validateJson(const char* input) {
  StaticJsonDocument<0> doc, filter;
  //return deserializeJson(doc, json, DeserializationOption::Filter(filter)) == DeserializationError::Ok;
  return deserializeJson(doc, input, DeserializationOption::Filter(filter)) == DeserializationError::Ok;

};

struct MCDOC {
    int movePitch = 0;
    int rapidPitch = 0;
    int accel = 0;
    int dwell = 0;
    bool f = true;
} ;

struct Vres {
    bool test;
    MCDOC d;
} ;


Vres validateMoveConfig(JsonObject& doc){
    MCDOC m;
    Vres vr = {false,m};
    if(doc.isNull()){
        printf("json doc was null\n");
        vr.test = false;
        return vr;
    }
    if(!doc.containsKey("f") ){
        printf("moveConfig doc: no f param\n");
        return vr;
    }else{
        bool f = doc["f"].as<bool>();
        vr.d.f = f;
    }
    if(!doc.containsKey("movePitch") ){
        printf("moveConfig doc: no movePitch param\n");
        return vr;
    }else if(doc["movePitch"].is<double>()){
        double mp = doc["movePitch"].as<double>();
    }else{
        return vr;
    }


    char buff[1000];
    serializeJsonPretty(doc,buff);
    printf("moveConfig doc workie %s\n",buff);
    vr.test = true;
    return vr;
}
