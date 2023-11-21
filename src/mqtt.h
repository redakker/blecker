#ifndef MQTT_H
#define MQTT_H

#include "definitions.h"
#include "utilities.h"
#include <WiFi.h>
#include <MqttClient.h>
#include "log.h"
#include "database.h"
#include <Callback.h>

class Mqtt {

    Logger logger;
    WiFiClient wifiClient;
    MqttClient* client;
    Database* database;
    Signal<int>* errorCodeChanged;
    Signal<String>* mqttMessageArrived;
    Signal<boolean>* mqttStatusChanged;
    String server;
    String user;
    String password;
    String statusOn;
    String statusOff;
    String deviceID;
    int port;
    String baseTopic;

    String deviceIPAddress;

    // MQTT connect try
    int lasttry;
    int maxtry;
    int MQTTconnectTime;

    // Keepalive timeout
    int lastKeepAliveTime;

    boolean networkConnected; // Connected to the network (Wifi STA)
    boolean subscribed;
    boolean deviceStatusRetain;
    boolean lastiWillSet;
    boolean mqtt_connected;

    public:
        Mqtt(Log& rlog);
        void setup(Database &database, Signal<boolean> &mqttStatusChanged, Signal<int> &errorCodeChanged, Signal<String> &mqttMessageArrived);
        void loop();
        void setConnected (boolean networkConnected);
        void sendMqttMessage(MQTTMessage message);
        void ipAddressChanged (String ipAddress);

    private:    
        void setLastWill();
        void sendMqttMessage(String topic, String message, boolean retain);
        void reconnect();
        void processMessage();
        void subscribeForBaseTopic();
        void sendStatus();
        void sendChipInfo();
};

#endif
