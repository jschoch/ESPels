#pragma once


namespace vFSM {

    struct Timer {

    virtual void startTimer() = 0;
    virtual void stopTimer() = 0;
    virtual bool timerRunning() = 0;
    };
}
