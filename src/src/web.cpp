
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
#include "hob.h"

#include "web.h"

bool web = true;

// buffer for msgpack
char outBuffer[6000];



size_t serialize_len = 0;

// stores websocket data from asyncWebServer that gets fed to arduinoJSON
String wsData;

// sends updates (statusDoc) to UI every interval
Neotimer update_timer = Neotimer(1000);

// TOOD: consider configs from config.h to update this stuff
AsyncWebServer server(80);
AsyncWebSocket ws("/els");
AsyncWebSocketClient * globalClient = NULL;


// This is a bit like a ping counter, increments each time a new status is sent to the UI
uint8_t statusCounter = 0;

// I htink this is sent from the UI and used to set the absolute target postion, in the Doc it is "ja"
double jogAbs = 0;

void saveNvConfigDoc(){
  EepromStream eepromStream(0, 512);
  serializeJson(nvConfigDoc, eepromStream);
  eepromStream.flush();
  loadNvConfigDoc();
}

void initNvConfigDoc(){
  nvConfigDoc.clear();
  // this flag says we've updated from the default
  nvConfigDoc["i"] = 1;

  // the config
  nvConfigDoc["lead_screw_pitch"] = LEADSCREW_LEAD;
  nvConfigDoc["spindle_encoder_resolution"] = ENCODER_RESOLUTION;
  nvConfigDoc["microsteps"] = Z_MICROSTEPPING;
  // the communication version, used to detect UI compatability
  nvConfigDoc["vsn"] = vsn;

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

    lead_screw_pitch = nvConfigDoc["lead_screw_pitch"];
    motor_steps = nvConfigDoc["motor_steps"];
    microsteps = nvConfigDoc["microsteps"];
    spindle_encoder_resolution = nvConfigDoc["spindle_encoder_resolution"]; 
    Serial.printf("Loaded Configuration com version %s lead screw pitch: %f\n",vsn,lead_screw_pitch);
    init_machine();
    setFactor();
  }
}



void updateStatusDoc(){
  // types the message as a status update for the UI
  statusDoc["t"] = "status";
  
  // Currently used for the UI DRO display, 
  //TODO: needs renamed and this is likely better to compute this in the browser!
  // TODO:  this means that we need to convert any "distance" move to steps befor it gets send to the controller 
  statusDoc["pmm"] = toolRelPosMM;
  // defined in util.h and helps UI figure out what to display
  statusDoc["m"] = (int)run_mode;
  // the position in steps, only used in the "debug" screen of the ui
  // TODO: could just send this and remove "pmm" and calculate position in MM in the browser
  // TODO: toolPos and toolRelPos should be consolidated as they seem largely redundant
  statusDoc["p"] = toolRelPos;
  // tool positoin in stepper steps
  statusDoc["tp"] = toolPos;

  // encoder postion in cpr pulses
  statusDoc["encoderPos"] = encoder.getCount();

  // I think this is used for aboslute movements
  // TODO: We should convert to steps in the UI and not have to do the conversion in the controller
  statusDoc["targetPosMM"] = targetToolRelPosMM;
  // bool for constant run mode, TODO:  bad name though
  statusDoc["feeding"] = feeding;
  // bools for sync movements, TODO:  bad name
  statusDoc["jogging"] = jogging;
  // bool for rapid sync movement TODO: bad name
  statusDoc["rap"] = rapiding;
  // main bool to turn movement on/off
  statusDoc["pos_feed"] = pos_feeding;
  
  // Wait for the spindle/ncoder "0" position
  // this acts like a thread dial
  statusDoc["sw"] = syncWaiting;
  // generated in the encoder and displayed in the UI
  // TODO: consider making the smoothing configurable or done in the browser
  statusDoc["rpm"] = rpm;

   // the virtual stop in the Z + direction, used to calculate "distance to go" in the UI
  statusDoc["sp"] = stopPos;
  // the stop in the Z - direction
  statusDoc["sn"] = stopNeg;
  sendStatus();
}

void updateDebugStatusDoc(){
  // types as a debug status msg
  debugStatusDoc["t"] = "dbg_st";
  // intended feeding "handiness" CCW or CW 
  // so if the spindle is rotating CW, and the intension is to feed in the Z- direction this should be false
  // but this somewhat depends on the setup  
  debugStatusDoc["fd"] = z_feeding_dir;
 
  // This is the number of encoder pulses needed before the next stepper pulse
  // TODO: make this optional and move to a "debug" doc
  debugStatusDoc["delta"] = delta;
  // this doesn't appear to be used now, but...
  debugStatusDoc["targetPos"] = targetToolRelPos;
  // for cpu stats
  // TODO: move this to a debug doc
  debugStatusDoc["c0"] = cpu0;
  debugStatusDoc["c1"] = cpu1;
  // this is incremented every status update
  debugStatusDoc["c"] = statusCounter++;
}

void updateStateDoc(){
  stateDoc["t"] = "state"; 
  // don't appear to be used
  //stateDoc["absP"] = absolutePosition;
  //stateDoc["P"] = relativePosition;

  // the pitch for the sync move calculation in MM
  stateDoc["pitch"] = pitch;
  // the pitch for sync rapid moves
  stateDoc["rapid"] = rapids;

  // TODO: this redundant to nvConfigDoc, remove after testing?
  //stateDoc["lead"] = lead_screw_pitch;
  //stateDoc["enc"] = spindle_encoder_resolution;
  //stateDoc["micro"] = microsteps;

  // the run mode 
  stateDoc["m"] = (int)run_mode; 
  // TODO: not used but this shold be used instead of jog_mm and the UI should do the conversion
  stateDoc["js"] = jog_steps;
  // this is the distance to jog in mm
  // TODO: depricate and have the UI convert and use steps
  stateDoc["jm"] = jog_mm;
  // this sets the desired "handedness" CW or CCW
  stateDoc["f"] = feeding_ccw;
  // this is the target postion for sync absolute movements
  stateDoc["ja"] = jogAbs;
  // flag to wait for encocer "0" position
  stateDoc["s"] = syncStart;
  // TODO: add angle for angle readout in UI

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

void sendDoc(const JsonDocument & doc){
  serialize_len = serializeMsgPack(doc, outBuffer);  
  // TODO: debug flag?
  ws.binaryAll(outBuffer,serialize_len);
}

void sendState(){
  sendDoc(stateDoc);
}

void sendLogP(Log::Msg *msg){
  logDoc["msg"] = msg->buf;
  logDoc["level"] = (int)msg->level;
  logDoc["t"] = "log";
  sendDoc(logDoc);
}

void sendStatus(){
  sendDoc(statusDoc);
  if(sendDebug){
    sendDoc(debugStatusDoc);
  }

}


void sendNvConfigDoc(){
  nvConfigDoc["t"] = "nvConfig";
  sendDoc(nvConfigDoc);
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
  if(inDoc["basic"]){

    auto t1 = inDoc["basic"];
    auto t = t1.as<int>();
    if(t ==1){
      encoder.setCount(encoder.getCount()+ 2400);
    }else if (t == 0){
      encoder.setCount(encoder.getCount() - 2400);
    }else if( t==2){
      encoder.dir = true;
      encoder.setCount((encoder.getCount() + 1));
    }else if (t == 3){
      encoder.dir = false;
      encoder.setCount((encoder.getCount() - 1));
    }
  }

  if(inDoc["ticks"]){
    auto ticks = inDoc["ticks"];
    encoder.setCount(encoder.getCount() + ticks.as<int>());
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
    updateStateDoc();
}
void handleNvConfig(){
  Serial.println("saving configuration");
    inDoc.remove("t");
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
void parseObj(AsyncWebSocketClient * client){
  DeserializationError error = deserializeJson(inDoc,wsData);
  if (error) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.f_str());
    return;
  }
  
  const char * cmd = inDoc["cmd"];
  if(strcmp(cmd,"helo") == 0){
    Serial.println("helo received, sending nvconfig");
    const char * uiVsn = inDoc["vsn"];
    if(strcmp(uiVsn, vsn)== 0){
      sendNvConfigDoc();
      updateStateDoc();
    }else{
      el.halt("bad version");
      client->close();
    }
    
  }
  else if(strcmp(cmd,"fetch") == 0){
    // regenerate config and send it along
    // TODO: compare version here
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
        parseObj(client);
        //pinned_parseObj();
      }
    }
 
  }
}



void init_web(){
  // Connect to WiFi
  Serial.println("Setting up WiFi");
  WiFi.setHostname(HOSTNAME);
  WiFi.mode(WIFI_MODE_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  WiFi.setTxPower(WIFI_POWER_19_5dBm);
  //WiFi.setOut
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(100);
  }
  Serial.print("Connected. IP=");
  Serial.println(WiFi.localIP());

  
  //  MDNS hostname must be lowercase
  if (!MDNS.begin(HOSTNAME)) {
        Serial.println("Error setting up MDNS responder!");
        while(1) {
          Serial.print("*");
            delay(100);
        }
    }
  MDNS.setInstanceName(HOSTNAME);
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
  // called in main loop
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


