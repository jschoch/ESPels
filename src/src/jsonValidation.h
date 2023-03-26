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
    // is f the same as feeding_ccw !???
    bool f = true;
    bool startSync = true;
    bool feeding_ccw = true;
    int moveSpeed = 0;
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
           lhs.feeding_ccw == rhs.feeding_ccw &&
           lhs.moveSpeed == rhs.moveSpeed &&
           lhs.f == rhs.f;
}

void printMCDOC(MCDOC& d){
    std::cout << "doc: movePitch: \n\t"<< d.movePitch << " rapidPitch: " << d.rapidPitch << "\n";
    std::cout << "\t accel: " << d.accel << " dwell: " << d.dwell << " movesteps: " << d.moveSteps<< "\n";
    std::cout << "\t f: " << d.f  << " moveSpeed: " << d.moveSpeed<<" feeding_ccw: " << d.feeding_ccw << " startSync: " << d.startSync << "\n";
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
    }else if(doc["f"].is<bool>()){
        bool f = doc["f"].as<bool>();
        m.f = f;
    }else{
        printf("moveConfig doc: f param type error");
    }

    if(!doc.containsKey("feeding_ccw") ){
        printf("moveConfig doc: no feeding_ccw param\n");
        return m;
    }else if(doc["feeding_ccw"].is<bool>()){
        bool feeding_ccw = doc["feeding_ccw"].as<bool>();
        m.feeding_ccw = feeding_ccw;
    }else{
        printf("moveConfig doc: feeding_ccw param type error");
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
        m.startSync = doc["startSync"].as<bool>();
    }else{
        printf("moveConfig doc: startSync param type error\n");
        return m;
    }

    if(!doc.containsKey("moveSpeed") ){
        printf("moveConfig doc: no moveSpeed param\n");
        return m;
    }else if(doc["moveSpeed"].is<int>()){
        m.moveSpeed= doc["moveSpeed"].as<int>();
    }else{
        printf("moveConfig doc: moveSpeed param type error\n");
        return m;
    }

    
    m.valid = true;
    return m;
}
