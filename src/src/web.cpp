
#include "c:\Users\jesse\Documents\Arduino\config.h"
// TODO: Configure your WiFi here
#include <WiFi.h>
#include <WebServer.h>
#include "config.h"

/* Put IP Address details */
IPAddress local_ip(192,168,1,1);
IPAddress gateway(192,168,1,1);
IPAddress subnet(255,255,255,0);

WebServer server(80);

void handle_OnConnect() {
  
  Serial.println("GPIO4 Status: OFF | GPIO5 Status: OFF");
  server.send(200, "text/html", "Lead: " + String(lead_screw_pitch)); 
}


void init_web(){
  // Connect to WiFi
  Serial.println("Setting up WiFi");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.print("Connected. IP=");
  Serial.println(WiFi.localIP());



  server.on("/", handle_OnConnect);
  
  
  server.on("/users/{}", []() {
    String user = server.pathArg(0);
    server.send(200, "text/plain", "User: '" + user + "'");
  });

  server.on("/leadscrew/{}", []() {
    float leadscrew = server.pathArg(0).toFloat();
    server.send(200, "text/plain", "L: '" + String(leadscrew) + "'");
    lead_screw_pitch = leadscrew;
  });
  
  // TODO:  add motor_steps, microsteps, native_steps, encoder_resolution 
  // TODO:  consider only run server in setup mode

  server.begin();
  Serial.println("HTTP server started");
}

void do_web(){
  server.handleClient();
}


