#ifndef LOG
#define LOG

#include "definitions.h"
#include <HardwareSerial.h>
#include "BluetoothSerial.h" // Header File for Serial Bluetooth

class Log {

    BluetoothSerial* blueToothSerial; // Object for Bluetooth

    public:
        Log() {}

        void setup () {
            // Init serial
            Serial.begin(115200);
        }

        void loop () {}

        void addBlueToothSerial(BluetoothSerial &bts){
            this->blueToothSerial = &bts;
        }

        void logPrefix(const char* prefix) {
            Serial.print(prefix);
			Serial.print(' ');

            // Send the messages over Bluetooth too if available
            if (blueToothSerial != NULL) {
                if (this->blueToothSerial->hasClient()) {
            		this->blueToothSerial->print(prefix);
					this->blueToothSerial->print(' ');
                }
            }
        }

        void logMessage(String message) {
			Serial.print(message);

            // Send the messages over Bluetooth too if available
            if (blueToothSerial != NULL) {
                if (this->blueToothSerial->hasClient()) {
                    this->blueToothSerial->print(message);
                }
            }
        }

        void logEnd() {
			Serial.println();

            // Send the messages over Bluetooth too if available
            if (blueToothSerial != NULL) {
                if (this->blueToothSerial->hasClient()) {
                    this->blueToothSerial->println();
                }
            }
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