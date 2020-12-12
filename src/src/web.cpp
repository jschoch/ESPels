
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include "config.h"

// json buffer
StaticJsonDocument<300> doc;
StaticJsonDocument<300> inDoc;

/* Put IP Address details */
IPAddress local_ip(192,168,1,1);
IPAddress gateway(192,168,1,1);
IPAddress subnet(255,255,255,0);

AsyncWebServer server(80);
AsyncWebSocket ws("/test");
AsyncWebSocketClient * globalClient = NULL;
bool web = true;


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
    char outBuffer[200]; 
    size_t len2 = serializeJson(doc, outBuffer);  
    // send it! 
    ws.textAll(outBuffer,len2);

  }else if(strcmp(cmd,"send") == 0){
    JsonObject config = inDoc["config"];
    Serial.println("getting config");
    Serial.print("got pitch: ");
    float p = config["pitch"];
    Serial.println(p);
    if(p != pitch){
      Serial.println("new pitch");
      doc["pitch"] = p;
      pitch = p;
    }

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

  if (!MDNS.begin("elsWS")) {
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

  doc["pitch"] = 0.1;
  doc["rapid"] = 1.0;
  doc["stopA"] = 1.0;
  doc["stopB"] = 2.0;
  doc["zero"] = 3.0;
  doc["jogD"] = 0.1;
  doc["lead"] = 2.0;
  doc["enc"] = 2400;
  doc["micro"] = 32;
  doc["e"] = 0;
  doc["u"] = 0;

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


