
#include "c:\Users\jesse\Documents\Arduino\config.h"
// TODO: Configure your WiFi here
#include <WiFi.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include "config.h"

/* Put IP Address details */
IPAddress local_ip(192,168,1,1);
IPAddress gateway(192,168,1,1);
IPAddress subnet(255,255,255,0);

WebServer server(80);
bool web = true;

void handle_OnConnect() {
  
  Serial.println("Web connection to /");
    String response = String("Settings:<HR>Pitch: " + String(lead_screw_pitch) + "<BR> resolution: " + \
    spindle_encoder_resolution + "<BR> Micro Steps: " +  microsteps + "<BR> Motor: " + motor_type + \
    "<BR> Rapids: " + rapids + "<BR> Backlash: "+ backlash + "<BR> motor steps: "+ motor_steps +\
    "<BR> Factor: " + String(factor,10) \
    );
  server.send(200, "text/html", "Lead: " + String(response)); 
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

  if (!MDNS.begin("els")) {
        Serial.println("Error setting up MDNS responder!");
        while(1) {
          Serial.print("*");
            delay(100);
        }
    }
  MDNS.addService("http", "tcp", 80);
  



  server.on("/", handle_OnConnect);
  
  server.on("/leadscrew/{}", []() {
    float leadscrew = server.pathArg(0).toFloat();
    server.send(200, "text/plain", "L: '" + String(leadscrew) + "'");
    lead_screw_pitch = leadscrew;
    init_machine();
  });

  server.on("/encoder/{}", []() {
    int enc = server.pathArg(0).toInt();
    server.send(200, "text/plain", "Got: '" + String(enc) + "'");
    spindle_encoder_resolution = enc;
  });

  server.on("/microsteps/{}", []() {
    microsteps = server.pathArg(0).toInt();
    server.send(200, "text/plain", "Got: '" + String(microsteps) + "'");
    init_machine();
  });

  server.on("/motor_type/{}", []() {
    int mt = server.pathArg(0).toInt();
    if(mt == 1 || mt == 2){
      motor_type = mt;
      server.send(200, "text/plain", "Got: '" + String(microsteps) + "'");
      init_machine();
    }else{
      server.send(200, "text/plain", "Error: motor type should be 1(200 1.8) or 2 (400 0.0");
    }
    
  });

  server.on("/rapids/{}", []() {
    float rapids = server.pathArg(0).toFloat();
    server.send(200, "text/plain", "Got: '" + String(rapids) + "'");
  });

  server.on("/backlash/{}", []() {
    float backlash = server.pathArg(0).toFloat();
    server.send(200, "text/plain", "Got: '" + String(backlash) + "'");
  });
  
  // TODO:  add motor_steps, microsteps, native_steps, encoder_resolution 
  // TODO:  consider only run server in setup mode

  server.begin();
  Serial.println("HTTP server started");
}

void do_web(){
  if(web){
    server.handleClient();
  }
  
}


