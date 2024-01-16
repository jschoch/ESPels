## ESPels:  A simple electronic lead screw for the esp32.

Join us on discord: https://discord.gg/Z2CqK4Kk


__DANGER__
while there is a "soft estop" right now take care to test things out without the halfnuts engaged, please test all the functions you plan to use this way first.  Use at your own risk and hover over the lathe e-stop.  The thing could go crazy and crash the carriage into the spindle at any moment.   It shouldn't but bugs happen....

This is currently in a "beta" state. There may be unfinished features, UI glitches, race conditions, crashes etc.  Feedback is needed to imporove the project.  Please use github issues with any questions, problems, or requests.

[ demo](https://www.youtube.com/watch?v=uXhqEe8Kw6M&list=PLvpLfzys-jPumkXZj8ZZn11zyY3UYtSkn&index=6)
[latest frontend release](http://espels.s3.us-west-2.amazonaws.com/dev0_0_5/index-cors3.html)

## Features

* Imperial or Metric 
* DRO
* Revolution counter
* angle indicator
* Feed Mode:
   * Acts like a full time gear system for the leadscrew (like clough42's ELS)  
* MoveSync Mode:
   * Allows you to feed a fixed distance and stop 
* Bounce Mode:
   * Feeds at a cutting speed and then rapids back to the starting position  
   * configurable delay
* Jog Mode:
   * moves not slaved to the spindle.  Be aware the default acceleration and speed settings  may not be appropriate for you machine.
   * Bounce mode for things like broaching with configurable delay

### Planned Features


* Virtual compound threading mode
   * This mode will offset threading passes to simulate a compound's angle infeed
   * this was done but needs a big refactoring
* TPI/pitch presets
* General Presets ( ala touchDRO)
* Hobbing Mode
  * Reuse codebase for configuring and running my hobbing controller 

## Configuration

Copy config.ini.template.ini to config.ini


in config.ini update the WIFI_SSID, WIFI_PASSWORD Z_STEP_PIN, Z_DIR_PIN, EA, and EB for your encoder signals etc.

# lead screw pitch

If you are connecting your stepper or servo to the lead screw 1:1 then simply measure the lead/pitch of the screw and enter that value in mm (not inches) into the config.ini or via the webUI.

If you have a belt/pulley reduction and or gears you need to set the LEADSCREW_LEAD to the disance the carriage travels for 1 motor revolution.  For example if you have a 2:3 reduction on your belt/pulley and your lead screw's physical pitch is 2mm you need to set the LEADSCREW_LEAD to (0.6666666666666 * 2) = 1.33333333333333333... 
=======

For example `'-D HOSTNAME="mx210latheELS"'` sets the hostname,  EA, EB for your encoder signals.  

The webUI simply is a react.js application designed to be responsive, it can live anywhere and should run on your phone, tablet, laptop or PC.  It connects to the firmware via websockets.  No network out to the internets is needed.  The s3 links are simply for convienence, you can serve the frontend yourself by copying the index.html or running `HOST=0.0.0.0 npm start`.  I may eventually offer an option to serve it from esp32's directly (assuming there is flash available).

Once the firware is flashed you simply need to configure the frontend with the IP or dns of the firmware.
TODO: fix image

After that the "wifi" icon will turn from red to green once connected.  You may want to continue to monitor the firmware's serial output the first time you connect.  You will see something like this:
TODO: fix image

Next you can click the red "venc On" button.  This will turn on a virtual quadrature encoder to simulate the spindle moving. You will see the RPM: DRO field increase and the rev indicator will show the number of revolutions.

Finally click the "Selected Mode" dropdown to change the mode, select Feed Mode, or MoveSync Mode, change the parameters (pitch etc) and test away!

TODO: fix image

## Lathe Setup

When installing on a lathe you will need an encoder with enough resolution.  Testing has been done with a 600PPI -- 2400CPI encoder. While more encoder pulses will create more accuracy the algorithm used is limited to a max of 1 step per 1 encoder position.  This limits the maximum pitch you can synchronize.  Consider this and the maximum thread pitch your lathe can handle when selecting an encoder and stepper microstepping configuration.


A closed loop stepper is highly recommended.  The stepper online 1-CL57Y-24HE40 is a good kit for a small hobby lathe.


You may need to swap your encoder pins and or change your stepper driver's direction.  The expectation is that when the spindle turns counter clockwise the "left" buttons should move the carriage Z-.


## Frontend

the webfrontend can be found here [https://github.com/jschoch/espELSfrontend]

## HOW

Initially I was planning on making a pcb with buttons to control everything.  This is messy and hard to update.  Now I'm using websockets and a react SPA to control everything.  The websockets transmit json and msgPack for configuration and status updates.  This is working very nicely.

This is currently using a version of the "didge" algorythm here (https://github.com/prototypicall/Didge/blob/master/doc/How.md_).  The slope can't be > 1 so you may need to reduce your micro stepping to facilitate large thread pitches.

## Development

to run the tests, for example just a single test matching the glob "*fsm*"  run 

```
pio test -e ESPelsTest --filter "*fsm*"
```



