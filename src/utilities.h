#ifndef UTILITIES_H
#define UTILITIES_H

#include "Arduino.h"

struct MQTTMessage {
  String topic;
  String payload;
  boolean retain;
  boolean individualTopic; // Control if we want to send an individual topic, or the device add the default prefix automatically (send device topic by default)
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