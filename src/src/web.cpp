#include <Arduino.h>
#include "web.h"
#include "WiFi.h"
#include <esp_wifi.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <ArduinoJson.h>
#include "jsonValidation.h"
//#include "config.h"
//#include <elslog.h> 
#include "Encoder.h"
#include "state.h"
#include "Stepper.h"
#include "BounceMode.h"
#include "Machine.h"
#include "Controls.h"
#include "SlaveMode.h"
#include "myperfmon.h"
#include "display.h"
#include "motion.h"
#include "hob.h"
#include "moveConfig.h"
#include <Ticker.h>
#include "led.h"
#include "AsyncBounceMode.h"

// Stringify wifi stuff
#define ST(A) #A
#define STR(A) ST(A)
//const char* WIFI_P = STR(WIFI_PASSWORD);

#include "web.h"

// To force delete the nvconfig
#define  KILLNV 0

bool web = true;
//static const char* TAGweb = "Mc";


// buffer for msgpack
char outBuffer[6000];

// for events
int eventLen = 0;
char eventBuf[SSE_EVENT_SIZE];

size_t serialize_len = 0;

// stores websocket data from asyncWebServer that gets fed to arduinoJSON
String wsData;

// sends updates (statusDoc) to UI every interval
Neotimer update_timer = Neotimer(1000);
Neotimer sse_timer = Neotimer(300);
Neotimer ota_timer = Neotimer(200);

// TOOD: consider configs from config.h to update this stuff
AsyncWebServer server(80);
AsyncEventSource events("/events");
AsyncWebSocket ws("/els");
AsyncWebSocketClient *globalClient = NULL;

// This is a bit like a ping counter, increments each time a new status is sent to the UI
uint8_t statusCounter = 0;

// I htink this is sent from the UI and used to set the absolute target postion, in the Doc it is "ja"
double jogAbs = 0;

void saveNvConfigDoc()
{
  EepromStream eepromStream(0, 512);
  nvConfigDoc["motor_steps"] = nvConfigDoc["native_steps"].as<int>() * nvConfigDoc["microsteps"].as<int>();
  serializeJson(nvConfigDoc, eepromStream);
  eepromStream.flush();
  Serial.print(" Saving Doc: ");
  serializeJson(nvConfigDoc,Serial);
  loadNvConfigDoc();
  sendDoc(nvConfigDoc);
}

void initNvConfigDoc()
{
  nvConfigDoc.clear();
  // this flag says we've updated from the default
  nvConfigDoc["i"] = 1;

  // the config
  nvConfigDoc["lead_screw_pitch"] = LEADSCREW_LEAD;
  nvConfigDoc["spindle_encoder_resolution"] = ENCODER_RESOLUTION;
  nvConfigDoc["microsteps"] = Z_MICROSTEPPING;
  nvConfigDoc["EA"] = EA;
  nvConfigDoc["EB"] = EB;
  nvConfigDoc["ZP"] = gs.zstepper.config.stepPin;
  nvConfigDoc["ZD"] = gs.zstepper.config.dirPin;
  nvConfigDoc["motor_steps"] = Z_MICROSTEPPING * Z_NATIVE_STEPS_PER_REV;
  nvConfigDoc["native_steps"] = Z_NATIVE_STEPS_PER_REV;

  // the communication version, used to detect UI compatability
  nvConfigDoc["vsn"] = vsn;

  saveNvConfigDoc();
}



void loadNvConfigDoc()
{
  EepromStream eepromStream(0, 512);
  deserializeJson(nvConfigDoc, eepromStream);
  if (KILLNV == 1|| !nvConfigDoc["i"] ){
    Serial.print("Doc!?  ");
    Serial.println((int)nvConfigDoc["i"]);
    el.error("no config found this is bad");
    serializeJsonPretty(nvConfigDoc, Serial);
  }
  else
  {

    if(nvConfigDoc.containsKey("lead_screw_pitch")){
      gs.c.lead_screw_pitch = nvConfigDoc["lead_screw_pitch"].as<float>();
    }else{
      Serial.println("key missing: lead");
    }


    if(nvConfigDoc.containsKey("native_steps")){
      gs.c.native_steps = nvConfigDoc["native_steps"].as<int>();
    }else{
      Serial.println("key missing: nat step");
    }

    if(nvConfigDoc.containsKey("microsteps")){
      gs.c.microsteps = nvConfigDoc["microsteps"].as<int>();
    }else{
      Serial.println("key missing: microstesp");
    }

    if(nvConfigDoc.containsKey("spindle_encoder_resolution")){
      gs.c.spindle_encoder_resolution = nvConfigDoc["spindle_encoder_resolution"].as<int>();
    }else{
      Serial.println("key missing: enc");
    }

    gs.c.motor_steps = gs.c.native_steps * gs.c.microsteps;
    nvConfigDoc["motor_steps"] = gs.c.motor_steps;

    Serial.printf("Loaded Configuration com version %s lead screw pitch: %lf\n", vsn, gs.c.lead_screw_pitch);
    init_machine();
    // setFactor();
    Serial.print("Loaded this NvConfig doc");
    serializeJsonPretty(nvConfigDoc, Serial);
    if(!gs.setELSFactor(mc.movePitch,true)){
      Serial.println("Something wrong with this NvConfig!!!!!");
    }
  }
}

void updateMoveConfigDoc(){
  moveConfigDoc["t"] = "moveConfigDoc";
  moveConfigDoc["movePitch"] = mc.movePitch;
  moveConfigDoc["rapidPitch"] = mc.rapidPitch;
  moveConfigDoc["accel"] = mc.accel;
  moveConfigDoc["moveDirection"] = mc.moveDirection;
  moveConfigDoc["dwell"] = mc.dwell;
  moveConfigDoc["startSync"] = mc.startSync;
  moveConfigDoc["feeding_ccw"] = mc.feeding_ccw;
  moveConfigDoc["moveSpeed"] = mc.moveSpeed;
  sendDoc(moveConfigDoc);
}

void updateDebugStatusDoc()
{
  // types as a debug status msg
  debugStatusDoc["t"] = "dbg_st";

  // for cpu stats
  debugStatusDoc["c0"] = cpu0;
  debugStatusDoc["c1"] = cpu1;
  // this is incremented every status update
  debugStatusDoc["c"] = statusCounter++;
  debugStatusDoc["h"] = ESP.getFreeHeap();
  debugStatusDoc["ha"] = ESP.getHeapSize();
  debugStatusDoc["cc"] = ws.count();
  // average time of encoder ISR
  debugStatusDoc["at"] = avg_times;

  // TODO: add chip id so some silly fellow doesn't try to run this on a S3 or c6
  /*
  esp_chip_info_t* out_info = ESP.get_chip_info();
  make object .... parse this struct 
  debugStatusDoc["chip"] = chipInfo; 
  */
}

void updateStateDoc()
{
  stateDoc["t"] = "state";

  // the run mode
  stateDoc["m"] = (int)run_mode;
  stateDoc["js"] = mc.moveDistanceSteps;
  // this sets the desired "handedness" CW or CCW
  stateDoc["f"] = mc.moveDirection;
  // this is the target postion for sync absolute movements
  stateDoc["ja"] = jogAbs;
  // flag to wait for encocer "0" position
  stateDoc["s"] = syncStart;
  sendState();
}

void setRunMode(int mode)
{
  switch (mode)
  {
  case (int)RunMode::SLAVE_JOG_READY:
    main_yasm.next(slaveJogReadyState);
    break;
  case (int)RunMode::SLAVE_READY:
    //main_yasm.next(SlaveModeReadyState);
    break;
  case (int)RunMode::FEED_READY:
    main_yasm.next(FeedModeReadyState);
    break;
  case (int)RunMode::HOB_READY:
    // hob_yasm.next(HobReadyState);
    HobReadyState();
    break;
  default:
    main_yasm.next(startupState);
    break;
  }
}

void sendDoc(const JsonDocument &doc)
{
  serialize_len = serializeMsgPack(doc, outBuffer);
  if(ws.availableForWriteAll()){
      ws.binaryAll(outBuffer, serialize_len);
  }else{
      Serial.println("\n\tWS not available");
  }
  
}

void sendState()
{
  sendDoc(stateDoc);
}

void sendLogP(Log::Msg *msg)
{
  logDoc["msg"] = msg->buf;
  logDoc["level"] = (int)msg->level;
  logDoc["t"] = "log";
  sendDoc(logDoc);
}

void sendStatus()
{
  sendDoc(statusDoc);
  if (sendDebug)
  {
    sendDoc(debugStatusDoc);
  }
}

void sendNvConfigDoc()
{
  nvConfigDoc["t"] = "nvConfig";
  sendDoc(nvConfigDoc);
}

bool processDoc(){
          JsonObject config = inDoc["moveConfig"];
          MCDOC mcdoc = validateMoveConfig(config);
          if(mcdoc.valid){
            if(gs.validPitch(mcdoc.movePitch)){
              mc.moveDistanceSteps = mcdoc.moveSteps;
              mc.movePitch = mcdoc.movePitch;
              mc.rapidPitch = mcdoc.rapidPitch;
              mc.feeding_ccw = mcdoc.feeding_ccw;
              mc.accel = mcdoc.accel;
              mc.moveSpeed = mcdoc.moveSpeed;
              mc.dwell = mcdoc.dwell;
              updateMoveConfigDoc();
              return true;
            }else{
              el.error("move pitch greater than max pitch");
            }
            
          }else{
            printMCDOC(mcdoc);
            char err_buff[600] = "";
            serializeJsonPretty(inDoc,err_buff);
            sprintf(el.buf,"invalid move config: %s",err_buff);
            el.error();
          }
          return false;
        }


void handleMoveAsync(){


  Serial.println("got async move command");
  if (run_mode == RunMode::SLAVE_JOG_READY  && processDoc())
  {
    
    if(stepTimerIsRunning){
      Serial.printf("Stepper was running, can't issue new move command");
      return;
    }
    // set true (Z+) if moveDistanceSteps is > 0
    bool d = gs.zstepper.setDirNow((mc.moveDistanceSteps > 0 ? 1 : 0));
    if(d){
      Serial.println("changing dir");
      delay(5);
    }
    int32_t initial_speed = prepareMovement(gs.position, mc.moveDistanceSteps, mc.moveSpeed, 100,100,mc.accel);
    Serial.printf("Async step test start: distance in steps: %i, initial speed: %i\n",mc.moveDistanceSteps,initial_speed);
    Serial.printf("accel: %i speed: %i\n ",mc.accel,mc.moveSpeed);
    startStepperTimer(initial_speed);
  }
  else
  {
    el.error("can't move, no moving mode is set or bad doc");
  }

  
}

//  Update the virtual encoder
void handleVencSpeed()
{

  JsonObject config = inDoc["config"];
  vEncSpeed = config["encSpeed"];
  Serial.print(vEncSpeed);
  Serial.println("Changing Virtual Encoder! ");
  if (vEncSpeed == 0)
  {
    stopVenc();
  }
  if (vEncSpeed > 0 && vEncStopped)
  {
    startVenc();
  }
}

void handleRapid()
{
  //  This just sets the pitch to "rapids" and runs as a normal jog with a faster pitch
  // TODO: calculate speed from current RPM and perhaps warn if accel is a problem?
  
  Serial.println("got rapid move command");
  if (run_mode == RunMode::SLAVE_JOG_READY && processDoc())
  {
    mc.oldPitch = mc.movePitch;
    updateStateDoc();
    
    Serial.println("Rapid! ");
    mc.movePitch = mc.rapidPitch;
    rapiding = true;
    doMoveSync();
  }else{
    printf("problems with rapid\n");
  }
}

void handleMove()
{
  Serial.println("got move command");
  if (run_mode == RunMode::SLAVE_JOG_READY && processDoc())
  {
    // pitch rapid and distance should all be in the moveConfig now
    updateStateDoc();
    doMoveSync(); 
    
  }
  else
  {
    el.error("can't move, no moving mode is set ");
  }
}
void doMoveSync(){

  if (!pos_feeding)
    {
      bool thedir = mc.setStops(gs.position);
      bool step_dir_response = gs.zstepper.setDirNow(thedir);
      Serial.printf("Response from stepper: %d stepper current direction: %d\n",step_dir_response,gs.zstepper.dir);
      bool valid = gs.setELSFactor(mc.movePitch,true);
      if(valid){
         Serial.printf("doJog pitch: %lf target: %i\n",mc.movePitch,mc.moveDistanceSteps);
        Serial.printf("\t\tStops: stopNeg: %i stopPos: %i\n",mc.stopNeg,mc.stopPos);
        init_pos_feed(); 
      }else{
        el.error("Error setting pitch");
      }
     
    }
    else
    {
      el.error("already set feeding, wait till done or cancel");
    }
}

void handleHobRun()
{
  if (run_mode == RunMode::HOB_READY)
  {
    Serial.println("You should send log msgs to the webapp using the el.xxx thingy.");
    Serial.println("got Hob Run");
    // check that we are not already running
    if (!pos_feeding)
    {
      // initialize hob run state?
      JsonObject config = inDoc["moveConfig"];
      mc.movePitch = config["pitch"].as<double>();
      Serial.printf("Pitch %lf \n", mc.movePitch);
      mc.moveDirection = (bool)config["f"];
      // syncStart = (bool)config["s"];
      //  TODO: do I need to sync the spindle in this mode?
      syncStart = false;
      // hob_yasm.next(HobRunState,true);
      HobRunState();
    }
    else
    {
      el.error("already feeding, can't set mode to HobRunState");
    }
  }
  else
  {
    el.error("previous state wrong, problem!!!");
  }
}

void handleHobStop()
{
  Serial.println("got hob stop");
  HobStopState();
}

void handleBounce()
{
   Serial.println("got bounce move command");
  if (run_mode == RunMode::SLAVE_JOG_READY && processDoc())
  {
    Serial.printf("Bounce config: distance: %i rapid: %lf move: %lf\n",mc.moveDistanceSteps,mc.rapidPitch,mc.movePitch);
    updateStateDoc(); 
    bouncing = true;
    main_yasm.next(BounceMoveState,true);
  }
  else{
    printf("problem with bounce move config or state\n");
  }
}
void handleBounceAsync()
{
   Serial.println("got bounce move command");
  if (run_mode == RunMode::SLAVE_JOG_READY && processDoc())
  {
    Serial.printf("Bounce config: distance: %i rapid: %lf move: %lf\n",mc.moveDistanceSteps,mc.rapidPitch,mc.movePitch);
    updateStateDoc(); 
    async_bouncing = true;
    main_yasm.next(AsyncBounceMoveToState,true);
  }
  else{
    printf("problem with bounce move config or state\n");
  }
}

void handleFeed(){
   Serial.println("got bounce move command");
  if (run_mode == RunMode::FEED_READY && processDoc())
  {
    Serial.printf("\nFeed ccw: %d config pitch %lf\n",mc.feeding_ccw,mc.movePitch);
    //bool d = mc.setStops(gs.currentPosition());
    gs.zstepper.setDirNow(mc.feeding_ccw);
    gs.setELSFactor(mc.movePitch);
    gs.init_gear(encoder.getCount());
    mc.useStops = false;
    syncWaiting = false;
    updateStateDoc();
    pos_feeding = true;
  }else{
    printf("problem with feed config or state\n");
  }

}

void handleDebug()
{
  if (inDoc["basic"])
  {

    auto t1 = inDoc["basic"];
    auto t = t1.as<int>();
    if (t == 1)
    {
      encoder.setCount(encoder.getCount() + 2400);
    }
    else if (t == 0)
    {
      encoder.setCount(encoder.getCount() - 2400);
    }
    else if (t == 2)
    {
      encoder.dir = true;
      encoder.setCount((encoder.getCount() + 1));
    }
    else if (t == 3)
    {
      encoder.dir = false;
      encoder.setCount((encoder.getCount() - 1));
    }
  }

  if (inDoc["ticks"])
  {
    auto ticks = inDoc["ticks"];
    encoder.setCount(encoder.getCount() + ticks.as<int>());
  }

  // Serial.printf("fccw: %d  fz: %d sd: %d encDir: %i \n",feeding_ccw,z_feeding_dir,zstepper.dir, encoder.dir);
}

void handleSend()
{
  // this handles config changes sent from UI

  // This seems redundant, why not just access inDoc?
  JsonObject config = inDoc["config"];
  Serial.println("getting config");
  if (config["m"] != (int)run_mode)
  {
    Serial.print("setting new mode from webUI: ");
    int got_run_mode = config["m"].as<int>();
    run_mode = RunMode(got_run_mode);
    Serial.println((int)run_mode);
    setRunMode(got_run_mode);
  }
  updateStateDoc();
}
void handleMoveConfig(){
  if ((run_mode == RunMode::SLAVE_JOG_READY || run_mode == RunMode::FEED_READY) && processDoc()){

    Serial.printf("Setting Accel to: %i",mc.accel);
    gs.setAccel(mc.accel);
    updateStateDoc();
  }else{
    printf("handleMoveConfig problem with state or doc\n");
  }
}
void handleNvConfig()
{
  Serial.println("saving configuration");
  JsonObject config = inDoc["config"];
  // TODO better checks than this~
  if (config["lead_screw_pitch"])
  {
    // Save
    config["i"] = 1;
    EepromStream eepromStream(0, 512);
    serializeJson(config, eepromStream);
    eepromStream.flush();
    loadNvConfigDoc();
    sendNvConfigDoc();

    init_motion();
    init_machine();
    init_controls();
    init_state(); 
    init_encoder(); 

    int old = mc.movePitch;
    if( !gs.setELSFactor(0.1,true) ) {
      printf("nvconfig may be bad, reverting move pitch");
      mc.movePitch = old;
    }
    if(old != 0) {
      mc.movePitch = old;
      gs.setELSFactor(mc.movePitch);
    }
    
  }
  else
  {
    el.error("error format of NV config bad");
  }
}

void parseObj(AsyncWebSocketClient *client)
{
  DeserializationError error = deserializeJson(inDoc, wsData);
  if (error)
  {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.f_str());
    return;
  }

  const char *cmd = inDoc["cmd"];
  if (strcmp(cmd, "helo") == 0)
  {
    Serial.println("helo received, sending nvconfig");
    const char *uiVsn = inDoc["vsn"];
    if (strcmp(uiVsn, vsn) == 0)
    {
      sendNvConfigDoc();
      updateStateDoc();
    }
    else
    {
      Serial.printf("Bad version: my version %s ui version: %s",vsn,uiVsn);
      el.halt("bad version");
      client->close();
    }
  }
  else if (strcmp(cmd, "fetch") == 0)
  {
    Serial.println("fetch received: sending config");
    updateStateDoc();
  }
  else if (strcmp(cmd, "debug") == 0)
  {
    // Debugging tools
    handleDebug();
  }
  else if (strcmp(cmd, "moveCancel") == 0)
  {
    // Cleanup state and reset mode, ESTOP
    Serial.println("Move Canceled");
    syncWaiting = false;
    pos_feeding = false;
    jogging = false;
    rapiding = false;
    bouncing = false;
    async_bouncing = false;
    vTck::O::running = false;
    stopStepperTimer();
    main_yasm.next(startupState);


    // this must be reset for moveSync to work after running feed
    mc.feeding_ccw = true;
  }
  else if (strcmp(cmd, "moveSync") == 0)
  {
    handleMove();
  }
  else if (strcmp(cmd, "moveAsync") == 0){
    handleMoveAsync();
  }
  else if (strcmp(cmd, "sendConfig") == 0)
  {
    handleSend();
  }
  else if (strcmp(cmd, "sendMoveConfig") == 0){
    handleMoveConfig();
  }
  else if (strcmp(cmd, "hobrun") == 0)
  {
    handleHobRun();
  }
  else if (strcmp(cmd, "hobstop") == 0)
  {
    handleHobStop();
  }
  else if (strcmp(cmd, "setNvConfig") == 0)
  {
    handleNvConfig();
  }
  else if (strcmp(cmd, "getNvConfig") == 0)
  {
    sendNvConfigDoc();
  }
  else if (strcmp(cmd, "resetNvConfig") == 0)
  {
    Serial.println("resetting nv config to defaults");
    initNvConfigDoc();
    sendNvConfigDoc();
  }
  else if (strcmp(cmd, "rapid") == 0)
  {
    handleRapid();
  }
  else if (strcmp(cmd, "updateEncSpeed") == 0)
  {
    handleVencSpeed();
  }
  else if (strcmp(cmd, "bounce") == 0)
  {
    handleBounce();
  }
  else if (strcmp(cmd,"bounceAsync")==0){
    handleBounceAsync();
  }
  else if(strcmp(cmd,"feed") ==  0)
  {
    handleFeed();
  }
  else if(strcmp(cmd,"ping") == 0){
    Serial.print("^");
  }
  else if(strcmp(cmd,"sendDebug") == 0){
    Serial.println("toggle send debug");
    sendDebug = !sendDebug;
  }
  else if(strcmp(cmd,"update_stats_interval") == 0){
    
    int tmp = inDoc["value"].as<int>();
    if( tmp != 0){
      sse_timer.set(tmp);
      sse_timer.repeatReset();
      Serial.printf("update Stats interval %i",tmp);
    }
    

  }
  else
  {
    Serial.println("unknown command");
    Serial.println(cmd);
  }
  // vTaskDelete(NULL);
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

void onWsEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len)
{
  // Serial.println("ws event");
  if (type == WS_EVT_CONNECT)
  {
    Serial.printf("Websocket client connection received id: %d\n",client->id());
    globalClient = client;
  }
  else if (type == WS_EVT_DISCONNECT)
  {
    Serial.println("Client disconnected");
    Serial.println("-----------------------");
  }
  else if (type == WS_EVT_DATA)
  {
    AwsFrameInfo *info = (AwsFrameInfo *)arg;
    if (info->final && info->index == 0 && info->len == len)
    {
      if (info->opcode == WS_TEXT)
      {
        data[len] = 0;
        // parseObj(String((char*) data));
        wsData = String((char *)data);
        parseObj(client);
        // pinned_parseObj();
      }
    }
  }
}

Ticker reconnectTimer;


void connectToWifi() {
  Serial.println("reConnecting to Wi-Fi...");
  //WiFi.begin(STR(WIFI_SSID), STR(WIFI_PASSWORD));
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.printf("SSID: %s :p %s\n",STR(WIFI_SSID),STR(WIFI_PASSWORD));
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(100);
  }
  WiFi.setTxPower(WIFI_POWER_19_5dBm);
  WiFi.setAutoReconnect(true);
  //WiFi.persistent(true);
  esp_wifi_set_ps(WIFI_PS_NONE);
  WiFi.setSleep(false);
  Serial.print("Connected. IP=");
  Serial.println(WiFi.localIP());
  //  MDNS hostname must be lowercase
  if (!MDNS.begin(HOSTNAME))
  {
    Serial.println("Error setting up MDNS responder!");
    while (1)
    {
      Serial.print("*");
      delay(100);
    }
  }
  MDNS.setInstanceName(HOSTNAME);
  MDNS.addService("http", "tcp", 80);

  events.onConnect([](AsyncEventSourceClient *client){
    if(client->lastId()){
      Serial.printf("Client reconnected! Last message ID that it got is: %u\n", client->lastId());
    }
    // send event with message "hello!", id current millis
    // and set reconnect delay to 1 second
    client->send("hello!", NULL, millis(), 10000);
  });

  ws.onEvent(onWsEvent);
  server.on("/events",HTTP_OPTIONS,[](AsyncWebServerRequest * request) {
    printf("got preflight");
    int headers = request->headers();
    int i;
    for(i=0;i<headers;i++){
      AsyncWebHeader* h = request->getHeader(i);
      Serial.printf("HEADER[%s]: %s\n", h->name().c_str(), h->value().c_str());
    }
    request->send(200, "text/plain", "Post route");
  });
  server.addHandler(&ws);
  server.addHandler(&events);
  DefaultHeaders::Instance().addHeader("Access-Control-Allow-Origin", "*");
  DefaultHeaders::Instance().addHeader("Access-Control-Allow-Headers", "*");
  DefaultHeaders::Instance().addHeader("Access-Control-Allow-Method", "*");
  DefaultHeaders::Instance().addHeader("Access-Control-Allow-Private-Network", "true");

  server.begin();
  Serial.println("HTTP websocket server started");

}

void onWifiConnect(WiFiEvent_t event, WiFiEventInfo_t info){
  Serial.println("Connected to Wi-Fi.");
  set_error_led_blink(5);
}

void onWifiDisconnect(WiFiEvent_t event, WiFiEventInfo_t info){
  Serial.println("Disconnected from Wi-Fi. ");
  Serial.println(info.wifi_sta_disconnected.reason);
  set_error_led_blink(1000);
  //reconnectTimer.detach(); // ensure we don't reconnect to MQTT while reconnecting to Wi-Fi
  //reconnectTimer.once(5, connectToWifi);
}


void init_web()
{
  // Connect to WiFi
  Serial.println("Setting up WiFi");
  WiFi.setHostname(HOSTNAME);
  WiFi.mode(WIFI_MODE_STA);
  
  WiFi.onEvent(onWifiConnect, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_GOT_IP);

  //WiFiEventId_t eventID = WiFi.onEvent(onWifiDisconnect, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_DISCONNECTED);
  WiFi.onEvent(onWifiDisconnect, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_DISCONNECTED);  

  connectToWifi();

  updateStateDoc();
  init_ota();
  EEPROM.begin(512);
  EepromStream eepromStream(0, 512);
  deserializeJson(nvConfigDoc, eepromStream);
  Serial.println("NV Config? ");
  Serial.println((int)nvConfigDoc["i"]);
  if (!nvConfigDoc["i"] || KILLNV == 1)
  {
    Serial.println("creating default nvConfig");
    initNvConfigDoc();
  }
  else
  {
    loadNvConfigDoc();
  }
}

void init_ota()
{
  ArduinoOTA
      .onStart([]()
               {
      String type;
      if (ArduinoOTA.getCommand() == U_FLASH)
        type = "sketch";
      else // U_SPIFFS
        type = "filesystem";

      // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
      Serial.println("Start updating " + type); })
      .onEnd([]()
             { Serial.println("\nEnd"); })
      .onProgress([](unsigned int progress, unsigned int total)
                  { Serial.printf("Progress: %u%%\r", (progress / (total / 100))); })
      .onError([](ota_error_t error)
               {
      Serial.printf("Error[%u]: ", error);
      if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
      else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
      else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
      else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
      else if (error == OTA_END_ERROR) Serial.println("End Failed"); });

  ArduinoOTA.begin();
}

void sendUpdates()
{
  // called in main loop
  if (update_timer.repeat())
  {
    updateDebugStatusDoc();
    //updateStatusDoc();

    ws.cleanupClients();
  }
  if(sse_timer.repeat() && (events.avgPacketsWaiting() < 3)){
    // TODO:  make waiting packet number above configurable
    // TODO:  make SSE configurable and detect connection so you can fall back to WS if needed

    eventDoc["t"] = "status";

    eventDoc["m"] = (int)run_mode;
    // the position in steps
    eventDoc["p"] = gs.position;

    // encoder postion in cpr pulses
    eventDoc["encoderPos"] = encoder.getCount();

    eventDoc["targetSteps"] = mc.moveDistanceSteps;
    eventDoc["jogging"] = jogging;
    // bool for rapid sync movement TODO: bad name
    eventDoc["rap"] = rapiding;
    // flag for bouncing mode
    eventDoc["bf"] = bouncing;
    // main bool to turn movement on/off
    eventDoc["pos_feed"] = pos_feeding;

    // Wait for the spindle/ncoder "0" position
    // this acts like a thread dial
    eventDoc["sw"] = syncWaiting;
    // generated in the encoder and displayed in the UI
    // TODO: calc rpm in browser
    eventDoc["rpm"] = rpm;

    // the virtual stop in the Z + direction, used to calculate "distance to go" in the UI
    eventDoc["sp"] = mc.stopPos;
    // the stop in the Z - direction
    eventDoc["sn"] = mc.stopNeg;
    eventDoc["r"] = WiFi.RSSI();

    // this is used by "Distance to Go" in the UI to figure out the direction 
    eventDoc["fd"] = mc.moveDirection;

    // this reverses everything
    eventDoc["fccw"] = mc.feeding_ccw;
    eventDoc["es"] = encoder.start;

    // the delta in steps to go for moveAsync
    eventDoc["asd"] = stepsDelta;
    eventDoc["av"] = alarm_value;
    eventDoc["sr"] = stepTimerIsRunning;
    eventDoc["as"] = (uint8_t)accelState;

    // unclear how to send binary data via events
    eventLen = serializeJson(eventDoc, eventBuf);

    events.send(eventBuf);
    // TODO:  why not just send only the stuff that changed?
  }
}
void do_web()
{
  if (web)
  {
    if(ota_timer.repeat()){
      ArduinoOTA.handle();
    }
  }
}
