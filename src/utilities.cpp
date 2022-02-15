#include "Arduino.h"
#include "database.cpp"

#ifndef STRUCTS
#define STRUCTS
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

// Get the name of the presence string. This is not the state, just the name which will be sent over!!!
static String getPresentString (Database &database, boolean presenceState) {
    String presence = DEFAULT_NOT_PRESENT;

    if (presenceState) {
        presence = database.getValueAsString(DB_PRECENCE);
        if (presence == "") {                    
            presence = DEFAULT_PRESENT;
        }
    } else {
        presence = database.getValueAsString(DB_NO_PRECENCE);
        if (presence == "") {                    
            presence = DEFAULT_NOT_PRESENT;
        }                
    }

    return presence;
}

#endif