#pragma once

#include <iostream>
#include <ArduinoJson.h>


bool validateJson(const char* input) {
  StaticJsonDocument<0> doc, filter;
  //return deserializeJson(doc, json, DeserializationOption::Filter(filter)) == DeserializationError::Ok;
  return deserializeJson(doc, input, DeserializationOption::Filter(filter)) == DeserializationError::Ok;

};

struct MCDOC {
    double movePitch = 0.0;
    double rapidPitch = 0.0;
    int accel = 0;
    int dwell = 0;
    int moveSteps = 0;
    bool f = true;
    bool startSync = true;
    bool valid = false;
} ;

inline bool operator==(const MCDOC& lhs, const MCDOC& rhs)
{
    return lhs.movePitch == rhs.movePitch &&
           lhs.rapidPitch == rhs.rapidPitch &&
           lhs.accel == rhs.accel &&
           lhs.dwell == rhs.dwell &&
           lhs.moveSteps == rhs.moveSteps&&
           lhs.startSync == rhs.startSync &&
           lhs.f == rhs.f;
}

void printMCDOC(MCDOC& d){
    std::cout << "doc: movePitch: \n\t"<< d.movePitch << " rapidPitch: " << d.rapidPitch << "\n";
    std::cout << "\t accel: " << d.accel << " dwell: " << d.dwell << " movesteps: " << d.moveSteps<< "\n";
    std::cout << "\t f: " << d.f << " startSync: " << d.startSync << "\n";
}



MCDOC validateMoveConfig(JsonObject& doc){
    MCDOC m;
    if(doc.isNull()){
        printf("json doc was null\n");
        return m;
    }
    if(!doc.containsKey("f") ){
        printf("moveConfig doc: no f param\n");
        return m;
    }else{
        bool f = doc["f"].as<bool>();
        m.f = f;
    }

    if(!doc.containsKey("movePitch") ){
        printf("moveConfig doc: no movePitch param\n");
        return m;
    }else if(doc["movePitch"].is<double>()){
        m.movePitch = doc["movePitch"].as<double>();
        if(m.movePitch == 0){
            printf("moveConfig: move pitch can't be zero");
        }
    }else{
        printf("moveConfig doc: movePitch param type error\n");
        return m;
    }

    if(!doc.containsKey("rapidPitch") ){
        printf("moveConfig doc: no rapidPitch param\n");
        return m;
    }else if(doc["rapidPitch"].is<double>()){
        m.rapidPitch = doc["rapidPitch"].as<double>();
        if(m.rapidPitch == 0){
            printf("moveConfig: rapid pitch can't be zero");
        }
    }else{
        printf("moveConfig doc: rapidPitch param type error\n");
        return m;
    }

    if(!doc.containsKey("moveSteps") ){
        printf("moveConfig doc: no moveSteps param\n");
        return m;
    }else if(doc["moveSteps"].is<int>()){
        m.moveSteps= doc["moveSteps"].as<int>();
    }else{
        printf("moveConfig doc: moveSteps param type error\n");
        return m;
    }

    if(!doc.containsKey("accel") ){
        printf("moveConfig doc: no accel param\n");
        return m;
    }else if(doc["accel"].is<int>()){
        m.accel = doc["accel"].as<int>();
    }else{
        printf("moveConfig doc: accel param type error\n");
        return m;
    }

    if(!doc.containsKey("dwell") ){
        printf("moveConfig doc: no dwell param\n");
        return m;
    }else if(doc["dwell"].is<int>()){
        m.dwell = doc["dwell"].as<int>();
    }else{
        printf("moveConfig doc: dwell param type error\n");
        return m;
    }

    if(!doc.containsKey("startSync") ){
        printf("moveConfig doc: no startSync param\n");
        return m;
    }else if(doc["startSync"].is<bool>()){
        m.dwell = doc["startSync"].as<bool>();
    }else{
        printf("moveConfig doc: startSync param type error\n");
        return m;
    }

    
    m.valid = true;
    return m;
}
