#pragma once

#include "config.h"
#include "WiFi.h"
#include "ESPAsyncWebServer.h"
//#include "c:\Users\jesse\Documents\Arduino\config.h"
#include <ESPmDNS.h>
#include <ArduinoJson.h>
#include <StreamUtils.h>

extern bool web;

void init_web(void);
void do_web(void);
void init_ota(void);
void updateStateDoc();
void updateStatusDoc();
void sendState();
void sendStatus();
void sendLogP(Log::Msg *msg);
void loadNvConfigDoc();
void handleJog();
void setStops();
void sendUpdates();
