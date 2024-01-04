#pragma once

//#include "config.h"
#include "WiFi.h"
#include "ESPAsyncWebServer.h"
#include <elslog.h>
#include <ESPmDNS.h>
#include <ArduinoJson.h>
#include <StreamUtils.h>
// #include <StaticJsonDocument.hpp>

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
void handleMove();
void sendUpdates();
void sendDoc(const JsonDocument& doc);
void setRunMode(int mode);
void doMoveSync();
