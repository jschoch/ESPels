
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include "config.h"

// json buffer
StaticJsonDocument<400> doc;
StaticJsonDocument<400> inDoc;

/* Put IP Address details */
IPAddress local_ip(192,168,1,1);
IPAddress gateway(192,168,1,1);
IPAddress subnet(255,255,255,0);

AsyncWebServer server(80);
AsyncWebSocket ws("/test");
AsyncWebSocketClient * globalClient = NULL;
bool web = true;
char outBuffer[250]; 
RunMode run_mode = RunMode::STARTUP;

void updateConfigDoc(){
  doc["absP"] = absolutePosition;
  doc["P"] = relativePosition;
  doc["pitch"] = pitch;
  doc["rapid"] = rapids;
  doc["stopA"] = 1.0;
  doc["stopB"] = 2.0;
  doc["zero"] = 3.0;
  doc["jogD"] = 0.1;
  doc["lead"] = lead_screw_pitch;
  doc["enc"] = spindle_encoder_resolution;
  doc["micro"] = microsteps;
  doc["e"] = 0;
  doc["u"] = 0;
  doc["m"] = (int)run_mode; 
  doc["d"] = (int)display_mode;

  sendConfig();
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

void sendConfig(){
  size_t len2 = serializeJson(doc, outBuffer);  
    // send it! 
  Serial.print("sending config: ");
  Serial.println(outBuffer);
  ws.textAll(outBuffer,len2);

}

void parseObj(String msg){
  //const size_t capacity = JSON_OBJECT_SIZE(2) + 10;
  //DynamicJsonDocument buf(capacity);
  DeserializationError error = deserializeJson(inDoc, msg);
  if (error) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.f_str());
    return;
  }
  
  const char * cmd = inDoc["cmd"];

  if(strcmp(cmd,"fetch") == 0){
    // regenerate config and send it along

    Serial.println("sending config");
    sendConfig();    
    
  }else if(strcmp(cmd,"send") == 0){
    JsonObject config = inDoc["config"];
    Serial.println("getting config");
    Serial.print("got pitch: ");
    float p = config["pitch"];
    Serial.println(p);
    if(p != pitch){
      Serial.println("new pitch");

      //doc["pitch"] = p;
      pitch = p;
      setFactor();
    }
    if(config["rapid"] != rapids){
      Serial.println("updating rapids");
      rapids = config["rapid"];
      //doc["rapid"] = rapids;
    }
    if(config["m"] != (int)run_mode){
      Serial.print("setting new mode from webUI: ");
      int got_run_mode = config["m"];
      run_mode = RunMode(got_run_mode);
      Serial.println((int)run_mode);
      setRunMode(got_run_mode);
    }
    updateConfigDoc();

  }
}

void onWsEvent(AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len){
  Serial.println("ws event");
  if(type == WS_EVT_CONNECT){
 
    Serial.println("Websocket client connection received");
    globalClient = client;
    
 
  } else if(type == WS_EVT_DISCONNECT){
    Serial.println("Client disconnected");
    Serial.println("-----------------------");
 
  } else if(type == WS_EVT_DATA){
    AwsFrameInfo * info = (AwsFrameInfo*)arg;
    Serial.print("Data received: ");
 
    for(int i=0; i < len; i++) {
          Serial.print((char) data[i]);
    }
    if(info->final && info->index == 0 && info->len == len){

      if(info->opcode == WS_TEXT){
        data[len] = 0;
        parseObj(String((char*) data));
      }
    }
 
    Serial.println();
  }
}
void init_web(){
  // Connect to WiFi
  Serial.println("Setting up WiFi");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(100);
  }
  Serial.print("Connected. IP=");
  Serial.println(WiFi.localIP());

  if (!MDNS.begin("elsWSdesk")) {
        Serial.println("Error setting up MDNS responder!");
        while(1) {
          Serial.print("*");
            delay(100);
        }
    }
  MDNS.addService("http", "tcp", 80);
  ws.onEvent(onWsEvent);
  server.addHandler(&ws);



  

  server.begin();
  Serial.println("HTTP websocket server started");

  updateConfigDoc();

  init_ota();
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


