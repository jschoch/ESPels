#pragma once

#include "config.h"

#include "web.h"

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
    class Msg{
        public:
        T t = T::WS;
        char buf[500] = "";
        Level level = Level::ERROR;
        void error();
        void error(const char*);
    };

}