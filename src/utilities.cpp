#include "Arduino.h"

#ifndef STRUCTS
#define STRUCTS
struct MQTTMessage {
  String topic;
  String payload;
  boolean retain;
};

struct Device {
    String name;
    String rssi;
    String mac;
    boolean available;
    // internal data
    unsigned long lastSeen;
    int mark;
    boolean observed;

};

#endif