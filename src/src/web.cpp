
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include "config.h"
#include "log.h"
#include "Encoder.h"
#include "state.h"
#include "Stepper.h"
#include "BounceMode.h"
#include "Machine.h"
#include "Controls.h"
#include "SlaveMode.h"
#include "myperfmon.h"
#include "display.h"
#include "DebugMode.h"
#include "Stepper.h"
#include "motion.h"

bool web = true;

// This defines ssid and password for the wifi configuration
//TODO move the location of this into a platformio variable or something? Maybe the location of the file as a constant in config.h
#include "../../wifisecret.h"

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

String wsData;

Neotimer update_timer = Neotimer(1000);

/*  not used for dhcp
// Put IP Address details 
IPAddress local_ip(192,168,1,1);
IPAddress gateway(192,168,1,1);
IPAddress subnet(255,255,255,0);
*/

AsyncWebServer server(80);
AsyncWebSocket ws("/els");
AsyncWebSocketClient * globalClient = NULL;
char outBuffer[450];
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
  motor acceleration
  motor max_speed

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
    microsteps = nvConfigDoc["microsteps"];
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
  statusDoc["rap"] = rapiding;
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
  statusDoc["cmd"] = "status";
  statusDoc["fd"] = z_feeding_dir;
  statusDoc["sw"] = syncWaiting;
  statusDoc["rpm"] = rpm;
  statusDoc["cpr"] = encoder.cpr;
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
  //stateDoc["d"] = (int)display_mode;
  stateDoc["js"] = jog_steps;
  // this is the distance to jog in mm
  stateDoc["jm"] = jog_mm;
  stateDoc["sc"] = jog_scaler;
  stateDoc["f"] = feeding_ccw;
  stateDoc["ja"] = jogAbs;
  stateDoc["s"] = syncStart;

  sendState();
  
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
      case (int)RunMode::HOB_READY :
        //hob_yasm.next(HobReadyState);
        HobReadyState();
        break;
      default: 
        btn_yasm.next(startupState);
        break;
    }
}

void sendState(){
  len = serializeMsgPack(stateDoc, outBuffer);  
  Serial.print(" s ");
  ws.binaryAll(outBuffer,len);
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
  // TODO: need to check if we are rapiding also somewhere!
  if(!jogging){
    Serial.println(jogAbs);

    // config["f"] is the feeding_ccw flag from the UI
    // TODO: why not set feeding_ccw like we do for handleJog()?
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
  }
}

//  Update the virtual encoder
void handleVencSpeed(){
  
  JsonObject config = inDoc["config"];
  vEncSpeed = config["encSpeed"];
  Serial.print(vEncSpeed);
  Serial.println("Changing Virtual Encoder! ");
  if(vEncSpeed == 0){
    stopVenc();
  }
  if(vEncSpeed > 0 && vEncStopped){
    startVenc();
  }
}

void handleRapid(){
  //  This just sets the pitch to "rapids" and runs as a normal jog with a faster pitch

  // TODO: calculate speed from current RPM and perhaps warn if accel is a problem?
  // really need the acceleration curve 
  Serial.println("Rapid! ");
  JsonObject config = inDoc["config"];
  jog_mm = config["jm"].as<double>();
  //start_rapid(jog_mm);
  oldPitch = pitch;
  pitch = rapids;
  rapiding = true;
  handleJog();
}
void setStops(){
  // TODO:  what happens when the factor changes and the encoder positoin is wrong?
      setFactor();
      targetToolRelPosMM = toolRelPosMM + jog_mm;
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
}

void handleJog(){
  Serial.println("got jog command");
  if(run_mode == RunMode::SLAVE_JOG_READY){
    JsonObject config = inDoc["config"];
    jog_mm = config["jm"].as<double>();
    feeding_ccw = (bool)config["f"];
    /*
    Serial.println("slaveJog mode ok");
    Serial.print("Jog steps: ");
    Serial.println(stepsPerMM * jog_mm);
    Serial.print("current tool position: ");
    Serial.println(toolRelPos);
    */
    if(!pos_feeding){
      setStops();   
      //Serial.print("updated targetToolRelPos");
      //Serial.println(targetToolRelPos);
      jogging = true;
      init_pos_feed();
      updateStatusDoc();
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

void handleHobRun(){
  if(run_mode == RunMode::HOB_READY){
    Serial.println("You should send log msgs to the webapp using the el.xxx thingy.");
    Serial.println("got Hob Run");
    // check that we are not already running
    if(!pos_feeding){
      // initialize hob run state?
      JsonObject config = inDoc["config"];
      pitch = config["pitch"].as<double>();
      Serial.printf("Pitch %f \n",pitch);
      feeding_ccw = (bool)config["f"];
      //syncStart = (bool)config["s"];
      // TODO: do I need to sync the spindle in this mode?
      syncStart = false;
      //hob_yasm.next(HobRunState,true);
      HobRunState();

    }else{
      el.error("already feeding, can't set mode to HobRunState");
    }
  }else{
    el.error("previous state wrong, problem!!!");
  }
}

void handleHobStop(){
  Serial.println("got hob stop");
  HobStopState();
}

void handleBounce(){
  Serial.println("Bounce! ");
  JsonObject config = inDoc["config"];

  // parse config
  pitch = config["pitch"].as<double>();
  rapids = config["rapid"].as<double>();
  jog_mm = config["jog_mm"].as<double>();
  feeding_ccw = (bool)config["f"];
  setStops();
  bouncing = true;
}

void handleDebug(){
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

}

void handleSend(){
  // TODO: i dont' think I use this really

  // This seems redundant, why not just access inDoc?
  JsonObject config = inDoc["config"];
    Serial.println("getting config");
    Serial.print("got pitch: ");
    double p = config["pitch"];
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
    double sc = config["sc"];
    if(sc != jog_scaler){
      Serial.println("updating jog scaler");
      jog_scaler = sc;
    }
    updateStateDoc();
}
void handleNvConfig(){
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
}


//void parseObj(String msg){
// This handles deserializing UI msgs and handling commands
//void parseObj(void * param){
void parseObj(){
  DeserializationError error = deserializeJson(inDoc,wsData);
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
  }else if(strcmp(cmd,"debug") ==0){
    // Debugging tools
    handleDebug(); 
  //  JOG COMMANDS
  }else if(strcmp(cmd,"jogcancel") == 0){
    // TODO wheat cleanup needs to be done?
    pos_feeding = false;  
    // TODO: need rapid cancel
  }else if(strcmp(cmd,"jogAbs") == 0){
    handleJogAbs();  
  }else if(strcmp(cmd,"jog") == 0){
    handleJog();
  }else if(strcmp(cmd,"sendConfig") == 0){
    handleSend(); 
  }else if(strcmp(cmd,"hobrun") ==0){
    handleHobRun();
  }else if(strcmp(cmd,"hobstop") == 0){
    handleHobStop();
  }else if(strcmp(cmd,"setNvConfig") == 0){
    handleNvConfig();
  }else if(strcmp(cmd,"getNvConfig") == 0){
    sendNvConfigDoc();

  }else if(strcmp(cmd,"resetNvConfig") == 0){
      Serial.println("resetting nv config to defaults");
      initNvConfigDoc(); 
      sendNvConfigDoc();
  }else if(strcmp(cmd,"rapid") == 0){
    handleRapid();
      
  }else if(strcmp(cmd,"updateEncSpeed") == 0){
    handleVencSpeed();
  }else if(strcmp(cmd,"bounce")== 0){
    handleBounce();
  }else{
    Serial.println("unknown command");
    Serial.println(cmd);
  }
  //vTaskDelete(NULL);
}

/*
void pinned_parseObj(){

  xTaskCreatePinnedToCore(
    parseObj,    // Function that should be called
    "ws server",  // Name of the task (for debugging)
    32000,            // Stack size (bytes)
    NULL,            // Parameter to pass
    3,               // Task priority
    NULL,             // Task handle
    0 // pin to core 0, arduino loop runs core 1
);
}
*/

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
    if(info->final && info->index == 0 && info->len == len){
      if(info->opcode == WS_TEXT){
        data[len] = 0;
        //parseObj(String((char*) data));
        wsData = String((char*) data);
        parseObj();
        //pinned_parseObj();
      }
    }
 
  }
}



void init_web(){
  // Connect to WiFi
  Serial.println("Setting up WiFi");
  WiFi.setHostname(myname);
  WiFi.mode(WIFI_MODE_STA);
  WiFi.begin(ssid, password);
  WiFi.setTxPower(WIFI_POWER_19_5dBm);
  //WiFi.setOut
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(100);
  }
  Serial.print("Connected. IP=");
  Serial.println(WiFi.localIP());

  
  //  MDNS hostname must be lowercase
  if (!MDNS.begin(myname)) {
        Serial.println("Error setting up MDNS responder!");
        while(1) {
          Serial.print("*");
            delay(100);
        }
    }
  MDNS.setInstanceName(myname);
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
void sendUpdates(){
  if(update_timer.repeat()){
    // only send state when it changes
    //updateStateDoc();
    //Serial.printf(" %d ",(int)run_mode);
    Serial.printf(" %d %d ",(int)run_mode,WiFi.RSSI());
    updateStatusDoc();
  }
}
void do_web(){
  if(web){
    
    ArduinoOTA.handle();
  }
  
}


