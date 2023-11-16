#ifndef MQTT
#define MQTT

#include "definitions.h"
#include "utilities.cpp"
#include <WiFi.h>
#include <MqttClient.h>
#include "log.hpp"
#include "utilities.cpp"
#include "database.cpp"
#include <Callback.h>

class Mqtt {

    Logger logger;
    WiFiClient wifiClient;
    MqttClient* client;
    Database* database;
    Signal<int>* errorCodeChanged;
    Signal<String>* mqttMessageArrived;
    String server;
    String user;
    String password;
    String statusOn;
    String statusOff;
    String deviceID;
    int port;
    String baseTopic;

    String deviceIPAddress = "undefined";

    // MQTT connect try
    int lasttry = 10000;
    int maxtry = 10;

    // Keepalive timeout
    int lastKeepAliveTime = 0;

    boolean networkConnected = false; // Connected to the network (Wifi STA)
    boolean subscribed = false;
    boolean lastWillRetain = false;
    boolean lastiWillSet = false;

    public:
        Mqtt(Log& rlog) : logger(rlog, "[MQTT]") {
            this -> client = new MqttClient(wifiClient);
        }

        void setup(Database &database, Signal<int> &errorCodeChanged, Signal<String> &mqttMessageArrived) {

            this -> database = &database;
            this -> errorCodeChanged = &errorCodeChanged;
            this -> mqttMessageArrived = &mqttMessageArrived;
            
            this -> user = this -> database -> getValueAsString(String(DB_MQTT_USER), false);
            this -> password = this -> database -> getValueAsString(String(DB_MQTT_PW), false);
            this -> port =  this -> database -> getValueAsInt(String(DB_MQTT_PORT), false);
            this -> server = this -> database -> getValueAsString(String(DB_MQTT_SERVER), false);
            this -> baseTopic = this -> database -> getValueAsString(String(DB_MQTT_TOPIC_PREFIX), false) + MQTT_TOPIC;

            this -> lastWillRetain = this -> database -> getValueAsBoolean(String(DB_DEVICE_STATUS_RETAIN), false, MQTT_STATUS_OFF_DEFAULT_RETAIN);
            

            if (this -> database -> isPropertyExists(DB_DEVICE_STATUS_ON)) {
                this -> statusOn = this -> database -> getValueAsString(String(DB_DEVICE_STATUS_ON), false);
            } else {
                this -> statusOn = MQTT_STATUS_ON_DEFAULT_VALUE;               
            }

            if (this -> database -> isPropertyExists(DB_DEVICE_STATUS_OFF)) {
                this -> statusOff = this -> database -> getValueAsString(String(DB_DEVICE_STATUS_OFF), false);
            } else {
                this -> statusOff = MQTT_STATUS_OFF_DEFAULT_VALUE;
            }

            this -> deviceID = this -> database -> getValueAsString(String(DB_DEVICE_ID), false);
            
            if (deviceID) {
                client->setId(deviceID);
            }
            this -> client -> setUsernamePassword(user, password);            
        }

        void loop() {
            if (networkConnected) {

                // Last will should be set after network is connected, but before MQTT is connected
                if (!lastiWillSet) {
                    setLastWill();
                    lastiWillSet = true;
                }

                // check for incoming messages
                if (client->connected()) {
                    
                    if (!subscribed) {
                        subscribeForBaseTopic();
                    }

                    int messageSize = client -> parseMessage();
                    if (messageSize) {
                       processMessage();
                    }

                    int now = millis();
                    if (now - lastKeepAliveTime > MQTT_KEEPALILIVE_TIME ) {
                        lastKeepAliveTime = now;
                        sendStatus();
                    }
                } else {
                    client->stop();
                    this -> subscribed = false;
                    reconnect();
                }
            } else {
                client->stop();                
                lastiWillSet = false;
                this -> subscribed = false;
                
            }
        }

        void setConnected (boolean networkConnected) {
            this->logger << "Wifi connection is " << (String)networkConnected;
            this->networkConnected = networkConnected;            
        }

        void sendMqttMessage(MQTTMessage message) {
            if (client->connected()) {
                String topic = baseTopic + "/";
                if (message.individualTopic) {
                    topic = "";
                }
                sendMqttMessage(topic + message.topic, message.payload, message.retain);
            }
        }

        void ipAddressChanged (String ipAddress) {
            this -> deviceIPAddress = ipAddress;
        }

    private:
    
        void setLastWill() {

            String message = String("{\"status\": \"" + statusOff + "\", \"ip\":\"" + this -> deviceIPAddress + "\"}");
            
            client -> beginWill(baseTopic, message.length(), lastWillRetain, 1);
            client -> print(message);
            client -> endWill();
            logger << "Last will is set. Retain: " << (String)lastWillRetain;
            
        }

        void sendMqttMessage(String topic, String message, boolean retain = false) {
            client -> beginMessage(topic, retain);            
            client -> print(message);            
            client -> endMessage();
        }

        void reconnect() {
            if (!String("").equals(server)) {
                const char* mqtt_s = const_cast<char*>(server.c_str());
                if (!client -> connect(mqtt_s, port)) {                    
                    logger << "MQTT connection failed! Error code = " << (String)client -> connectError();
                    this -> errorCodeChanged->fire(ERROR_MQTT);
                } else {
                    logger << "Connection started.";
                }
            } else {
                // logger << "MQTT connection info is missing.");
            }
        }

        void processMessage() {
             // we received a message, print out the topic and contents
            logger << "Message received on topic: " << client -> messageTopic();
            
            String message = "";
            while (client -> available()) {
                message += (char) client -> read();
            }
            // Broadcast MQTT message
            this -> mqttMessageArrived->fire(message);
            logger << "Message: " << message;
        }

        void subscribeForBaseTopic () {

            //setLastWill();

            // subscribe to a topic and send an 'I'm alive' message
            String subscription = baseTopic + MQTT_IN_POSTFIX + "/#";
            client -> subscribe(subscription);            
            logger << "Subscribed to topic " << subscription;

            this -> errorCodeChanged->fire(ERROR_NO_ERROR);
            subscribed = true;
        }

        void sendStatus () {
            sendMqttMessage(baseTopic, "{\"status\": \"" + statusOn + "\", \"ip\":\"" + this -> deviceIPAddress + "\"}", true);
        }
};

#endif
