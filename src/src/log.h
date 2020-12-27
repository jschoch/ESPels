#pragma once

#include "config.h"

namespace Log{
    enum class T {
            WEB,
            SER,
            WS
        };
    enum class Level {
        ERROR,
        WARN,
        INFO
    };
    struct Msg{
        T t = T::WS;
        char buf[500] = "";
        Level level = Level::ERROR;
    };

    inline void send(Msg msg){
        Serial.print("\nWS Log: ");
        Serial.println(msg.buf);
        //sendLog(msg);

    }


}