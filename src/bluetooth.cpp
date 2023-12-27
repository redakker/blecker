#include "bluetooth.h"

BlueTooth::BlueTooth(Log& rlog, Led& led) : logger(rlog, "[BLUE]") {

    lastRun = 0;
    lastClear = 0;
    lastSendDeviceData = 0;        
    sendAutoDiscovery = false;
    lastSendAutoDiscovery = 0;
    autoDiscoveryPrefix = "";
    beaconPresenceRetain = MQTT_BEACON_PRESENCE_DEFAULT_RETAIN;
    // This is not the best place here. This object should not know this, but autodiscover must use it.
    // You mut not use any other place in the object
    mqttBaseTopic = "";
    detailedReport = false;
    monitorObservedOnly = false; // Monitor devices only which are configured in the database
    networkConnected = false; // Connected to the network (Wifi STA)
    devices = LinkedList<Device>();
    devicesToRemove = LinkedList<int>();

    boolean mqttConnected = false; // Connected to MQTT server
    this -> led = &led;
}

void BlueTooth::setup(Database &database, Signal<MQTTMessage> &mqttMessageSend, Signal<Device> &deviceChanged) {

    this -> mqttMessageSend = &mqttMessageSend;
    this -> deviceChanged = &deviceChanged;
    this -> database = &database;
    
    BLEDevice::init(BOARD_NAME);
    pBLEScan = BLEDevice::getScan(); //create new scan            
    pBLEScan->setAdvertisedDeviceCallbacks(this);
    //pBLEScan->setActiveScan(true); //active scan uses more power, but get results faster
    pBLEScan->setActiveScan(false); //set to passive mode because of bug #58
    pBLEScan->setInterval(100);
    pBLEScan->setWindow(99);  // less or equal setInterval value

    this -> beaconPresenceRetain = this -> database -> getValueAsBoolean(String(DB_BEACON_PRESENCE_RETAIN), false, MQTT_BEACON_PRESENCE_DEFAULT_RETAIN);
    logger << "Beacon presence retain is set to " << (String) this -> beaconPresenceRetain;

    logger << "Bluetooth is initialized with name:" << BOARD_NAME;

    // Prefill the device list with the user's devices
    // In case of accidently reboot it will send a "not_home" message if the device is gone meanwhile
    // Example:
    // Device is available -> system sends a "home" MQTT message as retained
    // Device is phisically gone (owner go to work) and before the microcontorller sends the "not_home" message microcontroller reboots (power outage)
    // MQTT retained message will be never changed and device stays at home in the MQTT system.
    // Solution:
    // Owner upload the device names which must be observed, so we prefill the list and on the next scan will set them as gone if the story happens above

    // Devices in this format: 317234b9d2d0;15172f81accc;d0e003795c50            
    fillDevices(database.getValueAsString(DB_DEVICES));
    // Parse the sting, split by ;

    // Auto discovery for Home Assistant is available.
    // Set it tre if the user enabled it
    sendAutoDiscovery = (database.getValueAsInt(DB_HA_AUTODISCOVERY) > 0) ? true : false;
    if (sendAutoDiscovery) {
        autoDiscoveryPrefix = database.getValueAsString(DB_HA_AUTODISCOVERY_PREFIX);
    }

    // This is not the best place here. This object should not know this, but autodiscover must use it.
    // You mut not use any other place in the object
    this -> mqttBaseTopic = this -> database -> getValueAsString(String(DB_MQTT_TOPIC_PREFIX), false) + MQTT_TOPIC;

    detailedReport = (database.getValueAsInt(DB_DETAILED_REPORT) > 0) ? true : false;

    // Run the BLE scanner on another core as a separated task
    xTaskCreatePinnedToCore(bluetoothScanner,       // Method name
                            "BLE Scan Task",        // Only for humans for debug
                            1024*2,                 // How many bytes should be alloted.
                            pBLEScan,               // Pass in variable reference here (or NULL)
                            8,                      // Priority of task
                            &scan_handle,           // Reference to Task handle.  Ex: to delete the scan task, it would look like: "vTaskDelete(scan_handle);"
                            0);
    
}

void BlueTooth::loop() {

    // Find the expired devices
    for (int i = 0; i < this -> devices.size(); i++) {
        
        Device dev = devices.get(i);
        
        if (millis() - dev.lastSeen > BT_DEVICE_TIMEOUT || (long) millis() - (long) dev.lastSeen < 0) {

            // Give another chance to the device to appear (Device has DEVICE_DROP_OUT_COUNT lives in the beginning)
            dev.mark--;
            dev.lastSeen = millis();

            if (dev.mark == 0) {
                logger << "Device is gone. MAC: " << dev.mac;
                
                // Virtually remove the device
                dev.available = false;
                dev.rssi = "0";
                devices.set(i, dev);

                // Send an MQTT message about this device is NOT at home
                handleDeviceChange(dev);

            } 
            if (dev.mark > 0) {
                logger << "Device marked as gone. MAC: " << dev.mac << " Current mark is: " << (String)dev.mark;
                devices.set(i, dev);
            }
        } 
    }
    
    if ((millis() - lastClear > BT_LIST_REBUILD_INTERVAL && devices.size() > 0) || (long) millis() - (long) lastClear < 0) {
        lastClear = millis();
        logger << "Clear the device list. (This is normal operation. :))";
        // Clear the list, it will be rebuilt again. Resend the (available) status should not be a problem.
        devices.clear();
        fillDevices(this-> database -> getValueAsString(DB_DEVICES));
        mqttMessageSend->fire(MQTTMessage{"selfclean", "true", false});
    }
 
    if (sendAutoDiscovery) {
        if ((millis() - lastSendAutoDiscovery > HA_AUTODISCOVERY_INTERVAL && devices.size() > 0) || (long) millis() - (long) lastSendAutoDiscovery < 0) {
            lastSendAutoDiscovery = millis();
            logger << "Send autodicovery data.";
            for (int i = 0; i < this -> devices.size(); i++) {
                Device dev = devices.get(i);
                // Example
                // mosquitto_pub -h 127.0.0.1 -t home-assistant/device_tracker/a4567d663eaf/config -m '{"state_topic": "a4567d663eaf/state", "name": "My Tracker", "payload_home": "home", "payload_not_home": "not_home"}'

                if (dev.mac != NULL) {
                    String payload = "{\"state_topic\": \"" + mqttBaseTopic + "/" + dev.mac + "\", \"name\": \"" + dev.mac + "\", \"payload_home\": \"" + 
                    this -> database->getValueAsString(DB_PRECENCE) + "\", \"payload_not_home\": \"" + 
                    this -> database->getValueAsString(DB_NO_PRECENCE) + "\", \"source_type\": \"bluetooth_le\"}";

                    MQTTMessage autoDiscMessage = MQTTMessage{autoDiscoveryPrefix + "/device_tracker/" + dev.mac + "/config", payload, false, true};
                    mqttMessageSend->fire(autoDiscMessage);
                }
            }
        }
    }

}

void BlueTooth::setConnected(boolean connected) {
    this -> networkConnected = connected;
}

void BlueTooth::setMqttConnected(boolean connected) {
    this -> mqttConnected = connected;
}
        
void BlueTooth::onResult(BLEAdvertisedDevice advertisedDevice) {

    boolean newFound = true;
    String deviceMac = advertisedDevice.getAddress().toString().c_str();
    deviceMac.toLowerCase();
    String deviceName = advertisedDevice.getName().c_str();
    String deviceRSSI = (String) advertisedDevice.getRSSI();
    deviceMac.replace(":","");

    for (int i = 0; i < this -> devices.size(); i++) {
        Device dev = devices.get(i);
        if (deviceMac == dev.mac) {

            dev.available = true;
            handleDeviceChange(dev);
            dev.lastSeen = millis();
            dev.mark = DEVICE_DROP_OUT_COUNT;
            
            devices.set(i, dev);                   
            newFound = false;
            
        }
    }

    if (!monitorObservedOnly) {
        if (newFound) {
            Device dev = {deviceName, deviceRSSI, deviceMac, true, millis(), DEVICE_DROP_OUT_COUNT, false };
            devices.add(dev);
            logger << "New device found. MAC: " << deviceMac;
            // Send an MQTT message about this device is at home
            handleDeviceChange(dev);
        }
    }
}

void BlueTooth::fillDevices(String devicesString) {

    if (devicesString.length() == 0) { return; }

    this->monitorObservedOnly = true;
    char *devicesChar = new char[devicesString.length() + 1];
    strcpy(devicesChar, devicesString.c_str());
    String devMac = "";
    while ((devMac = strtok_r(devicesChar, PARSE_CHAR, &devicesChar)) != NULL) { // delimiter is the semicolon
        if (devMac.length() > 0) {
            devMac.toLowerCase();
            Device device = {
                "", // name
                "", // rssi
                devMac, // mac
                false, // available
                millis(), // lastSeen
                DEVICE_DROP_OUT_COUNT, // mark
                true //observed
            };
            this -> devices.add(device);
            logger << "Device added as observed device. MAC: " << devMac;
        }
    }

    delete [] devicesChar;
}

void BlueTooth::handleDeviceChange(Device dev) {
    mqttMessageSend->fire(MQTTMessage{dev.mac, database -> getPresentString(dev.available), beaconPresenceRetain});
    // TODO: need to refactor, send only one message for the consumers
    // This will call the webhook
    deviceChanged->fire(dev);

    if (detailedReport) {
        String payload = "{\"name\":\"" + ((dev.name == NULL) ? "" : dev.name) + "\", \"rssi\":\"" + ((dev.rssi == NULL) ? "" : dev.rssi) + "\", \"mac\":\"" + ((dev.mac == NULL) ? "" : dev.mac) + "\", \"presence\":\"" + database -> getPresentString(dev.available) + "\", \"observed\":\"" + ((dev.observed) ? "true" : "false") + "\", \"lastSeenMs\":\"" + (millis() - dev.lastSeen) + "\"}";
        MQTTMessage message = MQTTMessage{ String ("status/" + dev.mac), payload, beaconPresenceRetain };
        mqttMessageSend->fire(message);
    }
}



