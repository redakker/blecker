#include "log.h"

// Log
Log::Log() : blueToothSerial(NULL) {}

void Log::setup () {
    // Init serial
    Serial.begin(115200);
}

void Log::loop () {}

void Log::addBlueToothSerial(BluetoothSerial* bts){
    this->blueToothSerial = bts;
}

bool Log::checkBlueToothSerial() {

    if (blueToothSerial == NULL) 
        return false;

    if (!this->blueToothSerial->hasClient())
        return false;

    return true;
}

void Log::logBlueToothSerial(char message) {

    if (!checkBlueToothSerial()) 
        return;

    this->blueToothSerial->print(message);
}

void Log::logBlueToothSerial(const char* message) {

    if (!checkBlueToothSerial()) 
        return;

    this->blueToothSerial->print(message);
}

void Log::logBlueToothSerial(const String& message) {

    if (!checkBlueToothSerial()) 
        return;

    this->blueToothSerial->print(message);
}

void Log::logPrefix(const char* prefix) {
    Serial.print(prefix);
    logBlueToothSerial(prefix);

    Serial.print(' ');
    logBlueToothSerial(' ');
}

void Log::logMessage(const String& message) {
    Serial.print(message);
    logBlueToothSerial(message);
}

void Log::logEnd() {
    Serial.println();
    logBlueToothSerial("\r\n");
}

// Logentry

LogEntry::LogEntry(Log& log_, const char* prefix_) : l(log_) {
    l.logPrefix(prefix_);
}

LogEntry::~LogEntry() {
    l.logEnd();
}

LogEntry& LogEntry::operator << (const char* message) {
    l.logMessage(message);
    return *this;
}

LogEntry& LogEntry::operator << (const String& message) {
    l.logMessage(message);
    return *this;
}

// Logger

Logger::Logger(Log& log_, const char* prefix_) : l(log_), prefix(prefix_) {
}

LogEntry Logger::operator << (const char* message) {
    LogEntry logEntry(this->l, this->prefix);
    logEntry << message;
    return logEntry;
}
