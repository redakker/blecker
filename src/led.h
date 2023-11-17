#ifndef LED_H
#define LED_H

#include "definitions.h"
#include <Arduino.h>
#include <jled.h>
#include "log.h"

class Led {

    Logger logger;
    int messageCode;    
    JLed* led;

    unsigned long lastrun = 0;
    
    public:
        Led(Log& rlog);
        void setup ();
        void loop();
        void setMessage(int messageCode);

    private:
     
};

#endif