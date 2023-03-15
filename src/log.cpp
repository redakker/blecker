#ifndef LOG
#define LOG

#include "definitions.h"
#include <HardwareSerial.h>
#include <BluetoothSerial.h>

class Log {

    BluetoothSerial* blueToothSerial;

    public:
        Log() : blueToothSerial(NULL) {}

        void setup () {
            // Init serial
            Serial.begin(115200);
        }

        void loop () {}

        void addBlueToothSerial(BluetoothSerial* bts){
            this->blueToothSerial = bts;
        }

        bool checkBlueToothSerial() {

            if (blueToothSerial == NULL) 
                return false;

            if (!this->blueToothSerial->hasClient())
                return false;

            return true;
        }

        void logBlueToothSerial(char message) {

            if (!checkBlueToothSerial()) 
                return;

            this->blueToothSerial->print(message);
        }

        void logBlueToothSerial(const char* message) {

            if (!checkBlueToothSerial()) 
                return;

            this->blueToothSerial->print(message);
        }

        void logBlueToothSerial(const String& message) {

            if (!checkBlueToothSerial()) 
                return;

            this->blueToothSerial->print(message);
        }

        void logPrefix(const char* prefix) {
            Serial.print(prefix);
            logBlueToothSerial(prefix);

            Serial.print(' ');
            logBlueToothSerial(' ');
        }

        void logMessage(const String& message) {
            Serial.print(message);
            logBlueToothSerial(message);
        }

        void logEnd() {
            Serial.println();
            logBlueToothSerial("\r\n");
        }
};

class LogEntry {

    public:
        LogEntry(Log& log_, const char* prefix) : l(log_) {
            l.logPrefix(prefix);
        }

        ~LogEntry() {
            l.logEnd();
        }

        LogEntry& operator << (const char* message) {
            l.logMessage(message);
            return *this;
        }

        LogEntry& operator << (const String& message) {
            l.logMessage(message);
            return *this;
        }

    private:
        Log& l;
};

class Logger {
    
    public:
        Logger(Log& log_, const char* prefix_) : l(log_), prefix(prefix_) {
        }

        LogEntry operator << (const char* message) {
            LogEntry logEntry(this->l, this->prefix);
            logEntry << message;
            return logEntry;
        }

    private:
        Log& l;
        const char* prefix;
};

#endif