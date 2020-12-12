#include <Arduino.h>
#include "config.h"
#include "util.h"
#include "display.h"

#include <Wire.h>
#include "SSD1306Wire.h"
#include "neotimer.h"

SSD1306Wire  display(0x3c, 5, 4);
Neotimer display_timer = Neotimer(100);
Neotimer print_timer = Neotimer(1000);

int display_mode = STARTUP;


void clear(){
  display.clear();
  //display.flipScreenVertically();
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.setFont(ArialMT_Plain_10);
}
// TODO: figure out best way to deal with different displays or using Serial.

void init_display(){
//#ifdef USESSD1306
  // setup the display
  
  display.init();
  delay(200);
  clear();

  
//#endif

}

static String sp = String(" ");




// this is the feed mode
void do_startup_display(){
 
  display.drawString(0,0,"Mode: up/down ") ;
  display.drawString(0,11,"Left to start");  
  display.drawString(0,21, "Feed Mode: " + String(feed_menu_items[feed_mode_select]));
  
  display.display();
}

void do_ready_display(){
  display.drawString(0,0," Menu to go back") ;
  display.drawString(0,11,"Mod to start");
  display.drawString(0,22, "pitch: " + String(pitch) + " Mode: "  + String(feed_menu_items[feed_mode_select]));
  
  display.display();
}
// this is the config mode
void do_configure_display(){
 
  display.drawString(0,0, "config: ");
  display.display();
}


void do_status_display2(){
  
  display.drawString(0,0, "enc: " + String((int32_t)encoder.getCount()) + " T: " + String((int32_t)toolPos));
  display.drawString(0,11,"f: " +String(factor,2) + " D: " + String((int32_t)delta));

  display.drawString(0,21,"P:" + String(pitch,3) + "RPM: " + String(rpm));

  if(feeding){
    display.drawString(110,51,"F:"+ String(feeding_dir));
  }
    
  if(getDir()){
    display.drawString(100,51,"L");
  }else{
    display.drawString(100,51,"R");
  }

  display.display();
}

void do_debug_ready_display(){
  display.drawString(0,0,"Debug Jog" + String(vel));
  display.drawString(0,11," Left to jog 10mm");
  display.drawString(0,21," back: middle " + String(step_delta) );
  display.drawString(0,31,"J:"+ String(jogging) + " JS: " + String(jog_done));
  display.drawString(0,41,"s: " +String(jog_steps));
  display.display();
}

void do_status_display(){
  // Slave logging/feeding
  // this mode only moves slaved to spindle and with a direction button pressed
  display.drawString(0,0,"RPM:" + String(rpm) +  " Rapid: " + String(rapids) );
  display.drawString(0,11,  "Feed Pitch: " + String(pitch,2));
  display.drawString(0,21,"Pos: " + String(relativePosition))  ;
  display.drawString(0,31," Delta: " + String( (int32_t)delta ));
  display.drawString(0,41, "factor: " + String(factor,6) );
  
  display.display();
}

void do_display(){

  if(display_timer.repeat()){

    
    clear();
    switch (display_mode) {
      case STARTUP:
        do_startup_display();
        break;
      case CONFIGURE:
        do_configure_display();
        break; 
      case DSTATUS: 
        do_status_display();
        break;
      case READY:
        do_ready_display();
        break;
      case FEEDING:
        do_status_display();
        break;
      case DEBUG_READY:
        do_debug_ready_display();
        break;

    }
    
  }
  
  if (print_timer.repeat()){
    Serial.print("display mode ");
    Serial.print(DISPLAY_MODE[display_mode]);
    Serial.print(" Feed Mode: ");
    Serial.print(feed_menu_items[feed_mode_select]);
    Serial.println();
  }
}