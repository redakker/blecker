#ifndef BLUETOOTH_H
#define BLUETOOTH_H

#include "definitions.h"
#include "utilities.h"
#include "BluetoothSerial.h" // Header File for Serial Bluetooth
#include "LinkedList.h"
#include "log.h"
#include "led.h"
#include "database.h"
#include <Callback.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>


class BlueTooth: public BLEAdvertisedDeviceCallbacks {

    Logger logger;
    Led* led;
    Signal<MQTTMessage>* mqttMessageSend;
    Signal<Device>* deviceChanged;
    BLEScan* pBLEScan;
    Database* database;

    BluetoothSerial blueToothSerial; // Object for Bluetooth
    String command;
    long lastRun;
    long lastClear;
    long lastSendDeviceData;    
    
    boolean sendAutoDiscovery;
    long lastSendAutoDiscovery;
    String autoDiscoveryPrefix;
    // This is not the best place here. This object should not know this, but autodiscover must use it.
    // You mut not use any other place in the object
    String mqttBaseTopic;

    boolean detailedReport;
    boolean monitorObservedOnly; // Monitor devices only which are configured in the database
    boolean networkConnected; // Connected to the network (Wifi STA)
    boolean mqttConnected; // Connected to MQTT server

    boolean beaconPresenceRetain;


    LinkedList<Device> devices;
    LinkedList<int> devicesToRemove;

    // For Bluetooth scan task
    TaskHandle_t scan_handle;

    public:
        BlueTooth(Log& rlog, Led& led);
        void setup(Database &database, Signal<MQTTMessage> &mqttMessageSend, Signal<Device> &deviceChanged);
        void loop();
        void setConnected(boolean connected);
        void setMqttConnected(boolean connected);
    
    private: 
        void onResult(BLEAdvertisedDevice advertisedDevice);
        void fillDevices(String devicesString);
        void handleDeviceChange(Device dev);        
};

#endif
