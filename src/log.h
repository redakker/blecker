#ifndef LOG_H
#define LOG_H

#include "definitions.h"
#include <HardwareSerial.h>
#include <BluetoothSerial.h>

class Log {

    BluetoothSerial* blueToothSerial;

    public:
        Log();
        void setup ();
        void loop ();
        void addBlueToothSerial(BluetoothSerial* bts);
        bool checkBlueToothSerial();
        void logBlueToothSerial(char message);
        void logBlueToothSerial(const char* message);
        void logBlueToothSerial(const String& message);
        void logPrefix(const char* prefix);
        void logMessage(const String& message);
        void logEnd();
};

class LogEntry {

    public:
        LogEntry(Log& log_, const char* prefix_);
        ~LogEntry();
        LogEntry& operator << (const char* message);
        LogEntry& operator << (const String& message);

    private:
        Log& l;
};

class Logger {
    
    public:
        Logger(Log& log_, const char* prefix_);
        LogEntry operator << (const char* message);

    private:
        Log& l;
        const char* prefix;
};

#endif