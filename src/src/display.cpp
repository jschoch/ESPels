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

  display.drawString(0, 0, "Hello world");
  display.display();
//#endif

}

static String sp = String(" ");




// this is the feed mode
void do_startup_display(){
 
  
  display.drawString(0,0,"M: "+ String(mode_select));
  display.drawString(30,0,String(FEED_MODE[mode_select]));
  display.drawString(0,11,"P:" + String(pitch));
  display.display();
}

void do_ready_display(){
  display.drawString(0,0,"System Ready!") ;
  display.drawString(0,11,"Press Left Button to start");
  display.drawString(0,22, "feed pitch: " + String(pitch));
  display.drawString(0,33, String(FEED_MODE[mode_select]));
  display.display();
}
// this is the config mode
void do_configure_display(){
 
  display.drawString(0,0, "config: ");
  display.display();
}


void do_status_display(){
  
  display.drawString(0,0, "enc: " + String((uint32_t)encoder.getCount()) + " T: " + String(toolPos));
  //display.drawString(0,0,  " T: " + String(toolPos));
  display.drawString(0,11,"f: " +String(factor,4) + " D: " + String(delta));

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
    }
    
  }
  
  if (print_timer.repeat()){
    Serial.print("display mode ");
    Serial.print(DISPLAY_MODE[display_mode]);
    Serial.print(" ");
    Serial.print(String(FEED_MODE[mode_select]));
    Serial.println();
  }
}