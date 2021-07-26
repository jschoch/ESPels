
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include "config.h"


// This defines ssid and password for the wifi configuration
#include "c:\Users\jesse\Documents\Arduino\config.h"

// json buffer

// config stateDoc
StaticJsonDocument<600> stateDoc;

//  items to store in NV ram/EEPROM
StaticJsonDocument<500> nvConfigDoc;

// Used for msgs from UI
StaticJsonDocument<500> inDoc;

// used to send status to UI
StaticJsonDocument<600> statusDoc;

// Used to log to UI
StaticJsonDocument<600> logDoc;

size_t len = 0;


/*  not used for dhcp
// Put IP Address details 
IPAddress local_ip(192,168,1,1);
IPAddress gateway(192,168,1,1);
IPAddress subnet(255,255,255,0);
*/

AsyncWebServer server(80);
AsyncWebSocket ws("/test");
AsyncWebSocketClient * globalClient = NULL;
bool web = true;
char outBuffer[450]; 
RunMode run_mode = RunMode::STARTUP;
uint8_t statusCounter = 0;

double jogAbs = 0;

void saveNvConfigDoc(){
  EepromStream eepromStream(0, 512);
  serializeJson(nvConfigDoc, eepromStream);
  //EEPROM.commit();
  eepromStream.flush();
  loadNvConfigDoc();
}

void initNvConfigDoc(){
  nvConfigDoc.clear();
  // this flag says we've updated from the default
  nvConfigDoc["i"] = 1;

  // the config
  nvConfigDoc["lead_screw_pitch"] = lead_screw_pitch;
  nvConfigDoc["spindle_encoder_resolution"] = spindle_encoder_resolution;
  nvConfigDoc["microsteps"] = microsteps;

  // TODO list of things to add
  /*

  UI_update_rate
  last_pitch ?  do I want to do that?

  */
  //nvConfigDoc[""] = ;
  saveNvConfigDoc();
}

void loadNvConfigDoc(){
  EepromStream eepromStream(0, 512);
  deserializeJson(nvConfigDoc, eepromStream);
  if(!nvConfigDoc["i"]){
    Serial.print("Doc!?  ");
    Serial.println((int)nvConfigDoc["i"]);
    el.error("no config found this is bad");
  }else{
    Serial.println("Loaded Configuration");
    lead_screw_pitch = nvConfigDoc["lead_screw_pitch"];
    motor_steps = nvConfigDoc["motor_steps"];
    spindle_encoder_resolution = nvConfigDoc["spindle_encoder_resolution"]; 
    init_machine();
    setFactor();
  }
}



void updateStatusDoc(){
  statusDoc["p"] = toolRelPos;
  statusDoc["pmm"] = toolRelPosMM;
  statusDoc["m"] = (int)run_mode;
  statusDoc["tp"] = toolPos;
  statusDoc["encoderPos"] = encoder.pulse_counter;
  statusDoc["delta"] = delta;
  statusDoc["calcPos"] = calculated_stepper_pulses;
  statusDoc["targetPos"] = targetToolRelPos;
  statusDoc["targetPosMM"] = targetToolRelPosMM;
  statusDoc["feeding"] = feeding;
  statusDoc["jogging"] = jogging;
  statusDoc["pos_feed"] = pos_feeding;
  statusDoc["jog_dong"] = jog_done;
  statusDoc["sne"] = stopNegEx;
  statusDoc["spe"] = stopPosEx;
  statusDoc["sp"] = stopPos;
  statusDoc["sn"] = stopNeg;
  statusDoc["c0"] = cpu0;
  statusDoc["c1"] = cpu1;
  statusDoc["xd"] = exDelta;
  statusDoc["c"] = statusCounter++;
  statusDoc["f"] = factor;
  statusDoc["cmd"] = "status";
  statusDoc["fd"] = z_feeding_dir;
  statusDoc["sw"] = syncWaiting;
  statusDoc["rpm"] = rpm;
  sendStatus();
}

void updateStateDoc(){
  stateDoc["absP"] = absolutePosition;
  stateDoc["P"] = relativePosition;
  stateDoc["pitch"] = pitch;
  stateDoc["rapid"] = rapids;
  stateDoc["stopA"] = 1.0;
  stateDoc["stopB"] = 2.0;
  stateDoc["zero"] = 3.0;
  stateDoc["jogD"] = 0.1;
  stateDoc["lead"] = lead_screw_pitch;
  stateDoc["enc"] = spindle_encoder_resolution;
  stateDoc["micro"] = microsteps;
  stateDoc["e"] = 0;
  stateDoc["u"] = 0;
  stateDoc["m"] = (int)run_mode; 
  stateDoc["d"] = (int)display_mode;
  stateDoc["js"] = jog_steps;
  stateDoc["jm"] = jog_mm;
  stateDoc["sc"] = jog_scaler;
  stateDoc["f"] = feeding_ccw;
  stateDoc["ja"] = jogAbs;
  stateDoc["s"] = syncStart;

  sendState();
  // this needs a timer to send on interval
  
}

void setRunMode(int mode){
    switch(mode){
      case (int)RunMode::DEBUG_READY :
        // NEed to be able to stop what is currently running 
        btn_yasm.next(debugState);
        break;
      case (int)RunMode::SLAVE_JOG_READY :
        btn_yasm.next(slaveJogReadyState);
        break;
      case (int)RunMode::SLAVE_READY :
        btn_yasm.next(SlaveModeReadyState);
        break;
      default: 
        btn_yasm.next(startupState);
        break;
    }
}

void sendState(){
  len = serializeJson(stateDoc, outBuffer);  
  //Serial.print("sending config: ");
  //Serial.println(outBuffer);
  // send it! 
  ws.textAll(outBuffer,len);
}

void sendLogP(Log::Msg *msg){
  logDoc["msg"] = msg->buf;
  logDoc["level"] = (int)msg->level;
  logDoc["cmd"] = "log";
  len = serializeMsgPack(logDoc,outBuffer);
  ws.binaryAll(outBuffer,len);
}

void sendStatus(){
  len = serializeMsgPack(statusDoc, outBuffer);
  ws.binaryAll(outBuffer,len);

}

void sendNvConfigDoc(){
  nvConfigDoc["cmd"] = "nvConfig";
  len = serializeMsgPack(nvConfigDoc, outBuffer);
  ws.binaryAll(outBuffer,len);
}

void handleJogAbs(){
  JsonObject config = inDoc["config"];
  jogAbs = config["ja"];
  syncStart = config["s"];
  targetToolRelPosMM = jogAbs;
  if(!jogging){
    Serial.println(jogAbs);
    if(config["f"]){
      if((toolRelPosMM - jogAbs) < 0 ){
        z_feeding_dir = true;
        stopPos = jogAbs;
        stopNeg = toolRelPosMM;
        zstepper.setDir(true);
      }
      else{
        z_feeding_dir = false;
        stopNeg = jogAbs;
        stopPos = toolRelPosMM;
      }
    }else{
      if((toolRelPosMM - jogAbs) > 0 ){
        z_feeding_dir = true;
        stopPos = jogAbs;
        stopNeg = toolRelPosMM;
        zstepper.setDir(true);
      }
      else{
        z_feeding_dir = false;
        stopNeg = jogAbs;
        stopPos = toolRelPosMM;
      }
    }
    init_pos_feed();
    updateStatusDoc();
    btn_yasm.next(slaveJogPosState);
  }
}

void handleJog(){
  Serial.println("got jog command");
    /*  TODO  needs refactor
    if(run_mode == RunMode::DEBUG_READY){
      JsonObject config = inDoc["config"];
      jog_mm = config["jm"].as<float>();
      Serial.println("debug mode ok");
      Serial.print("Jog steps: ");
      Serial.println(stepsPerMM * jog_mm);
      if(!jogging){
        feeding_ccw = config["f"];
        if(jog_mm < 0){
          z_feeding_dir = false;
          jog_steps = (float)stepsPerMM * jog_mm * -1;
        }else{
          z_feeding_dir = true;
          jog_steps = (float)stepsPerMM * jog_mm;
        }
        
        jogging = true;
      }
    }else if(run_mode == RunMode::SLAVE_JOG_READY){
    */

    if(run_mode == RunMode::SLAVE_JOG_READY){
      JsonObject config = inDoc["config"];
      jog_mm = config["jm"].as<float>();
      /*
      Serial.println("slaveJog mode ok");
      Serial.print("Jog steps: ");
      Serial.println(stepsPerMM * jog_mm);
      Serial.print("current tool position: ");
      Serial.println(toolRelPos);
      */
      if(!pos_feeding){
        // TODO:  what happens when the factor changes and the encoder positoin is wrong?
        setFactor();
        targetToolRelPosMM = toolRelPosMM + jog_mm;
        feeding_ccw = (bool)config["f"];
        if(jog_mm < 0){
          z_feeding_dir = false;
          stopNeg = toolRelPosMM + jog_mm;
          stopPos = toolRelPosMM;
          zstepper.setDir(false);
       }else{
          z_feeding_dir = true;
          stopPos = targetToolRelPosMM;
          stopNeg = toolRelPosMM;
          zstepper.setDir(true);
        }
        //Serial.print("updated targetToolRelPos");
        //Serial.println(targetToolRelPos);

        init_pos_feed();
        updateStatusDoc();
        btn_yasm.next(slaveJogPosState);
      }
      else{
        //Serial.print("already feeding, can't feed");
        el.error("already set feeding, wait till done or cancel");
      }
    }else{
      //Serial.println("can't jog, failed mode check");
      el.error("can't jog, no jogging mode is set ");
    }
}

void parseObj(String msg){
  DeserializationError error = deserializeJson(inDoc, msg);
  if (error) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.f_str());
    return;
  }
  
  const char * cmd = inDoc["cmd"];

  if(strcmp(cmd,"fetch") == 0){
    // regenerate config and send it along

    Serial.println("fetch received: sending config");
    updateStateDoc();
    
  // Fake encoder commands
  }else if(strcmp(cmd,"debug") ==0){
    int t = inDoc["dir"];
    if(t ==1){
      encoder.setCount(encoder.pulse_counter+ 2400);
    }else if (t == 0){
      encoder.setCount(encoder.pulse_counter- 2400);
    }else if( t==2){
      encoder.dir = true;
      encoder.setCount((encoder.pulse_counter+ 1));
    }else if (t == 3){
      encoder.dir = false;
      encoder.setCount((encoder.pulse_counter - 1));
    }

    //Serial.printf("fccw: %d  fz: %d sd: %d encDir: %i \n",feeding_ccw,z_feeding_dir,zstepper.dir, encoder.dir);

  //  JOG COMMANDS
  }else if(strcmp(cmd,"jogcancel") == 0){
    // TODO wheat cleanup needs to be done?
    pos_feeding = false;  
  }else if(strcmp(cmd,"jogAbs") == 0){
    handleJogAbs();  
  }else if(strcmp(cmd,"jog") == 0){
    handleJog();
  }else if(strcmp(cmd,"send") == 0){
    JsonObject config = inDoc["config"];
    Serial.println("getting config");
    Serial.print("got pitch: ");
    float p = config["pitch"];
    Serial.println(p);
    if(p != pitch){
      Serial.println("new pitch");
      oldPitch = pitch;
      pitch = p;
      setFactor();
    }
    if(config["rapid"] != rapids){
      Serial.println("updating rapids");
      rapids = config["rapid"];
      //stateDoc["rapid"] = rapids;
    }
    if(config["m"] != (int)run_mode){
      Serial.print("setting new mode from webUI: ");
      int got_run_mode = config["m"];
      run_mode = RunMode(got_run_mode);
      Serial.println((int)run_mode);
      setRunMode(got_run_mode);
    }
    float sc = config["sc"];
    if(sc != jog_scaler){
      Serial.println("updating jog scaler");
      jog_scaler = sc;
    }
    updateStateDoc();
  }else if(strcmp(cmd,"setNvConfig") == 0){
    Serial.println("saving configuration");
    inDoc.remove("cmd");
    // TODO better checks than this~
    if(inDoc["lead_screw_pitch"]){
      // Save 
      inDoc["i"] = 1;
      EepromStream eepromStream(0, 512);
      serializeJson(inDoc, eepromStream);
      eepromStream.flush();
      loadNvConfigDoc();
      sendNvConfigDoc();
    }else{
      el.error("error format of NV config bad");
    }

  }else if(strcmp(cmd,"getNvConfig") == 0){
    sendNvConfigDoc();

  }else if(strcmp(cmd,"resetNvConfig") == 0){
      Serial.println("resetting nv config to defaults");
      initNvConfigDoc(); 
      sendNvConfigDoc();
  }else{
    Serial.println("unknown command");
    Serial.println(cmd);
  }
}

void onWsEvent(AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len){
  //Serial.println("ws event");
  if(type == WS_EVT_CONNECT){
 
    Serial.println("Websocket client connection received");
    globalClient = client;
    
 
  } else if(type == WS_EVT_DISCONNECT){
    Serial.println("Client disconnected");
    Serial.println("-----------------------");
 
  } else if(type == WS_EVT_DATA){
    AwsFrameInfo * info = (AwsFrameInfo*)arg;
    /*
    Serial.print("Data received: ");
    for(int i=0; i < len; i++) {
          Serial.print((char) data[i]);
    }
    */

    if(info->final && info->index == 0 && info->len == len){

      if(info->opcode == WS_TEXT){
        data[len] = 0;
        parseObj(String((char*) data));
      }
    }
 
  }
}
void init_web(){
  // Connect to WiFi
  Serial.println("Setting up WiFi");
  WiFi.setHostname("elsd");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(100);
  }
  Serial.print("Connected. IP=");
  Serial.println(WiFi.localIP());

  
  //  MDNS hostname must be lowercase
  if (!MDNS.begin("elsd")) {
        Serial.println("Error setting up MDNS responder!");
        while(1) {
          Serial.print("*");
            delay(100);
        }
    }
  MDNS.setInstanceName("mydeskels");
  MDNS.addService("http", "tcp", 80);
  ws.onEvent(onWsEvent);
  server.addHandler(&ws);
  server.begin();
  Serial.println("HTTP websocket server started");

  updateStateDoc();
  init_ota();
  EEPROM.begin(512);
  EepromStream eepromStream(0, 512);
  deserializeJson(nvConfigDoc, eepromStream);
  Serial.println("NV Config? ");
  Serial.println((int)nvConfigDoc["i"]);
  if(!nvConfigDoc["i"]){
    Serial.println("creating default nvConfig");
    initNvConfigDoc();
  }else{
    loadNvConfigDoc();
  }

}

void init_ota(){
  ArduinoOTA
    .onStart([]() {
      String type;
      if (ArduinoOTA.getCommand() == U_FLASH)
        type = "sketch";
      else // U_SPIFFS
        type = "filesystem";

      // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
      Serial.println("Start updating " + type);
    })
    .onEnd([]() {
      Serial.println("\nEnd");
    })
    .onProgress([](unsigned int progress, unsigned int total) {
      Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
    })
    .onError([](ota_error_t error) {
      Serial.printf("Error[%u]: ", error);
      if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
      else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
      else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
      else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
      else if (error == OTA_END_ERROR) Serial.println("End Failed");
    });

  ArduinoOTA.begin();

}
void do_web(){
  if(web){
    
    ArduinoOTA.handle();
  }
  
}


