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
    int distance = 0;
    bool f = true;
    bool valid = false;
} ;

inline bool operator==(const MCDOC& lhs, const MCDOC& rhs)
{
    return lhs.movePitch == rhs.movePitch &&
           lhs.rapidPitch == rhs.rapidPitch &&
           lhs.accel == rhs.accel &&
           lhs.dwell == rhs.dwell &&
           lhs.distance == rhs.distance &&
           lhs.f == rhs.f;
}

void printMCDOC(MCDOC& d){
    std::cout << "doc: movePitch: "<< d.movePitch << " rapidPitch: " << d.rapidPitch << "\n";
    std::cout << "\t accel: " << d.accel << " dwell: " << d.dwell << " dist: " << d.distance << "\n";
    std::cout << "\t f: " << d.f << "\n";
    std::cout << "1 == 1.0" << (1 == 1.0) << "\n";
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

    if(!doc.containsKey("distance") ){
        printf("moveConfig doc: no distance param\n");
        return m;
    }else if(doc["distance"].is<int>()){
        m.distance = doc["distance"].as<int>();
    }else{
        printf("moveConfig doc: distance param type error\n");
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
    
    m.valid = true;
    return m;
}
