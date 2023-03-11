## ESPels

A simple electronic lead screw for the esp32

* DANGER *
There is no estop right now, not 100% safe.  Ensure you can quickly stop the spindle and disengage the half nut.  It shouldn't but bugs....

[depricated: demo](https://www.youtube.com/watch?v=uXhqEe8Kw6M&list=PLvpLfzys-jPumkXZj8ZZn11zyY3UYtSkn&index=6)

## Basics

The basic architecture is the firmware does the motion control and the frontend does user interaction and configuration.  The frontend is a react SPA (single page application) which uses websockets to connect to the firmware.  The Frontend can live where ever you like, you can use the S3 link or you can host it in your own network.  Once the files are loaded to your browser it only uses local networking.  No silly firwall transitions or anything like that.  You might ask why I don't serve the web content directly from the esp32 and the answer is simply development speed.  This could be done with littlefs and an esp32 with enough storage.

This repo contains the firmware.  The frontend can be found here: [https://github.com/jschoch/espELSfrontend]

You can see the frontend built here: [http://espels.s3-us-west-2.amazonaws.com/dev0_0_3/index.html]


## Configuration


Wifi:  ssid and password need to be defined, this is default a file ../../wifisecret.h

```c
#pragma once
#ifndef _WEBCONFIG_H
#define _WEBCONFIG_H

static const char* ssid = "";
static const char* password = "";

#endif
```

Copy the "config.ini.default" to "config.ini" and update as needed.  For example `'-D HOSTNAME="mx210latheELS"'` sets the hostname,  EA, EB for your encoder signals.  

See (Install.md) for more info on installation




## Frontend

![sample](https://user-images.githubusercontent.com/20271/104212553-33b6c200-53ea-11eb-899b-3ec2c22e56c2.png)

the webfrontend can be found here [https://github.com/jschoch/espELSfrontend]







Plan:

1. [x] update DRO with toolRelMM 
1. [x] Slave Jog MM stepper mode
2. [x] get stops working, slave jog to stop mode
3. figure out how to sync thread start
4. implement full threading cycle

Cleanup:

1. rethink "modes", right now there are display_modes and state transitions.  The GUI's mode selection should select the state machine mode needed.  
  a. get rid of the menu thing.



** TODO

- [ ] fix rapid stepper rtos crashes
- [ ] lock UI on rapid move
- [ ] add rapid cancel
- [. ] Add rapid jog when Mod btn is pressed
- [ ] Add optional encoder wheel input
- [ ] Add virtual stops
- [ ] Add feed to stop mode
- [ ] Add feed to distance (mm) mode
- [ ] Display distance to go
- [ ] Add physical stops (adjustable hall sensors for example)
- [ ] Add backlash compensation
- [ ] turn on task monitoring and display via web ui
- [ ] smooth filter EMA RPM for display
- [ ] store position after disengaging slave so you can re-index if you re-engage
- [ ] Add angle measure mode like a manual turned indexing plate.
- [] bug with startup when spindle spinning it may have a hard time booting
- [x] Add mm position to display
- [x ] Add some js for frontend
- [x ] Add HTML parameter updates
- [x ] Turn web off when in feed modes
- [x ] Add EEPROM saving of params


Misc:

