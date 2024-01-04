/*

TODO:  This seems 100% not used
        get rid of it if so
*/

#include <Arduino.h>
#include "config.h"
#include <yasm.h>
#include "state.h"




void FeedModeReadyState(){
    if(main_yasm.isFirstRun()){
        updateMode(RunMode::FEED_READY);

    }
}

