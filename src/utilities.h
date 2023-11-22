#ifndef UTILITIES_H
#define UTILITIES_H

#include "Arduino.h"
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"

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

extern esp_chip_info_t chip_info;
void setChipInfo();
const char* getChipModelString(esp_chip_model_t model);
void bluetoothScanner(void *parameters);

#endif