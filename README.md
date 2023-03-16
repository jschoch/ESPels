## ESPels:  A simple electronic lead screw for the esp32.

__DANGER__
There is no estop right now, not safe.  Use at your own risk and hover over the lathe e-stop.  The thing could go crazy and crash the carriage into the spindle at any moment.   It shouldn't but bugs happen....

This is currently in a "beta" state. There may be unfinished features, UI glitches, race conditions, crashes etc.  Feedback is needed to imporove the project.  Please use github issues with any questions, problems, or requests.

[ demo](https://www.youtube.com/watch?v=uXhqEe8Kw6M&list=PLvpLfzys-jPumkXZj8ZZn11zyY3UYtSkn&index=6)
[latest frontend release](http://espels.s3-us-west-2.amazonaws.com/release_0_0_3/index.html)

## Features

* Imperial or Metric 
* Revolution counter
* Feed Mode:
   * Acts like a full time gear system for the leadscrew (like clough42's ELS)  
* MoveSync Mode:
   * Allows you to feed a fixed distance and stop 
* Bounce Mode:
   * Feeds at a cutting speed and then rapids back to the starting position  

### Planned Features


* Spindle angle DRO indicator
* Broaching mode
  * Utilize bounce mode with spindle fixed for broaching keyways.
* Virtual compound threading mode
   * This mode will offset threading passes to simulate a compound's angle infeed
* TPI presets
* General Presets ( ala touchDRO)
* Hobbing Mode
  * Reuse codebase for configuring and running my hobbing controller 

## Configuration

Copy config.ini.d to config.ini

In config.ini update the WIFI_SSID, WIFI_PASSWORD Z_STEP_PIN, Z_DIR_PIN, EA, and EB for your encoder signals etc.

The webUI simply is a react.js application designed to be responsive, it can live anywhere and should run on your phone, tablet, laptop or PC.  It connects to the firmware via websockets.  No network out to the internets is needed.  The s3 links are simply for convienence, you can serve the frontend yourself by copying the index.html or running `HOST=0.0.0.0 npm start`.  I may eventually offer an option to serve it from esp32's directly (assuming there is flash available).

Once the firware is flashed you simply need to configure the frontend with the IP or dns of the firmware.

![image](https://user-images.githubusercontent.com/20271/225640770-0720b314-eee3-4650-9613-717a55c63898.png)


After that the "wifi" icon will turn from red to green once connected.  You may want to continue to monitor the firmware's serial output the first time you connect.  You will see something like this:

![image](https://user-images.githubusercontent.com/20271/225632806-9f107f18-0247-4377-a290-8d14f369b59e.png)

Next you can click the red "venc On" button.  This will turn on a virtual quadrature encoder to simulate the spindle moving. You will see the RPM: DRO field increase and the rev indicator will show the number of revolutions.

Finally click the "Selected Mode" dropdown to change the mode, select Feed Mode, or MoveSync Mode, change the parameters (pitch etc) and test away!

![image](https://user-images.githubusercontent.com/20271/225641162-c13e5c4c-d1a8-48b6-bdbc-2bdc5bf3bffc.png)


## Lathe Setup

When installing on a lathe you will need an encoder with enough resolution.  Testing has been done with a 600PPI -- 2400CPI encoder. While more encoder pulses will create more accuracy the algorithm used is limited to a max of 1 step per 1 encoder position.  This limits the maximum pitch you can synchronize.  Consider this and the maximum thread pitch your lathe can handle when selecting an encoder and stepper microstepping configuration.


A closed loop stepper is highly recommended.  The stepper online 1-CL57Y-24HE40 is a good kit for a small hobby lathe.


You may need to swap your encoder pins and or change your stepper driver's direction.  The expectation is that when the spindle turns counter clockwise the "left" buttons should move the carriage Z-.


## Frontend

the webfrontend can be found here [https://github.com/jschoch/espELSfrontend]

## HOW

Initially I was planning on making a pcb with buttons to control everything.  This is messy and hard to update.  Now I'm using websockets and a react SPA to control everything.  The websockets transmit json and msgPack for configuration and status updates.  This is working very nicely.

This is currently using a version of the "didge" algorythm here (https://github.com/prototypicall/Didge/blob/master/doc/How.md_).  The slope can't be > 1 so you may need to reduce your micro stepping to facilitate large thread pitches.



