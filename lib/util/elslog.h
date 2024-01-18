#pragma once
//#include <Arduino.h>

//#include "config.h"

#ifndef PIO_UNIT_TESTING
//#include "../../src/src/config.h"
//#include "../../src/src/web.h"
#endif

namespace Log{
    enum class T {
            WEB,
            SER,
            WS
        };
    enum class Level {
        ERROR,
        WARN,
        INFO,
        HALT
    };
    class Msg{
        public:
        T t = T::WS;
        bool hasError = false;
        char buf[1000] = "";
        Level level = Level::ERROR;
        void maybeSend();
        void error(void);
        void error(const char*);
        void halt(const char*);
        void addMsg(const char*);
        void errorTask();
        static void errorTaskImpl(void* _this);
    };

}

extern Log::Msg el;
