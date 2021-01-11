#pragma once

#include "config.h"
#include "WiFi.h"
#include "ESPAsyncWebServer.h"
//#include "c:\Users\jesse\Documents\Arduino\config.h"
#include <ESPmDNS.h>
#include <ArduinoJson.h>

void init_web(void);
void do_web(void);
void init_ota(void);
void updateConfigDoc();
void updateStatusDoc();
void sendConfig();
void sendStatus();
void sendLogP(Log::Msg *msg);
