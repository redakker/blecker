#ifndef LOG
#define LOG

#include "definitions.h"
#include <HardwareSerial.h>
#include "BluetoothSerial.h" // Header File for Serial Bluetooth

class Log {

    BluetoothSerial* blueToothSerial; // Object for Bluetooth

    public:
        Log(){}

        void setup () {
            // Init serial
            Serial.begin(115200);
        }

        void loop (){
            
        }

        void addBlueToothSerial(BluetoothSerial &bts){
            this->blueToothSerial = &bts;
        }

        void log(String prefix, String message) {
            prefix.trim();
            this -> log( prefix + " " + message );
        }        

        void log(String message) {
            Serial.println(message);

            // Send the messages over Bluetooth too if available
            if (blueToothSerial != NULL) {
                if (this->blueToothSerial->hasClient()) {
                    this->blueToothSerial->println(message);
                }
            }
        }
};

#endif