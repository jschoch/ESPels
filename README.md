## ESPels

A simple electronic lead screw for the esp32

this is currently in test mode and pre-alpha

[b] DANGER 
There is no estop right now, not safe.  Use at your own risk and hover over the lathe e-stop.  The thing could go crazy and crash the carriage into the spindle at any moment.  It shouldn't but bugs....

[ demo](https://www.youtube.com/watch?v=uXhqEe8Kw6M&list=PLvpLfzys-jPumkXZj8ZZn11zyY3UYtSkn&index=6)
[latest frontend release](http://espels.s3-us-west-2.amazonaws.com/release_0_0_3/index.html)


Disclaimer out of the way: i've been using it for a week and it has not freaked out yet.


## Configuration

Copy config.ini.d to config.ini

in config.ini update the WIFI_SSID, WIFI_PASSWORD Z_STEP_PIN, Z_DIR_PIN etc.
Update  EA, and EB for your encoder signals.  

See (Install.md) for more info on installation

## Frontend

the webfrontend can be found here [https://github.com/jschoch/espELSfrontend]



## HOW

Initially I was planning on making a pcb with buttons to control everything.  This is messy and hard to update.  Now I'm using websockets and a react SPA to control everything.  The websockets transmit json and msgPack for configuration and status updates.  This is working very nicely but there is some lag.

Because there is no feedback from a touch screen I plan on integrating an optional haptic input controller which will have bidirectional communication with the ELS controller.  


This is currently using a version of the "didge" algorythm here (https://github.com/prototypicall/Didge/blob/master/doc/How.md_).  The slope can't be > 1 so you may need to reduce your micro stepping.



