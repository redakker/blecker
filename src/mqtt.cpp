#ifndef MQTT
#define MQTT

#include "definitions.h"
#include "utilities.cpp"
#include <WiFi.h>
#include <MqttClient.h>
#include "log.cpp"
#include "utilities.cpp"
#include "database.cpp"
#include <Callback.h>

class Mqtt {

    Log* rlog;
    String log_prefix = "[MQTT] ";
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

    boolean networkConnected = false; // Connected to the network (Wifi STA)
    boolean subscribed = false;
    boolean lastWillRetain = false;

    public:
        Mqtt(Log &log) {
            this -> rlog = &log;
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
                // check for incoming messages            
                if (client->connected()) {
                    
                    if (!subscribed) {
                        subscribeForBaseTopic();
                    }

                    int messageSize = client -> parseMessage();
                    if (messageSize) {
                       processMessage();
                    }
                } else {
                    client->stop();
                    this -> subscribed = false;
                    reconnect();
                }
            } else {
                client->stop();
                this -> subscribed = false;
            }
        }

        void setConnected (boolean networkConnected) {
            this -> rlog -> log(log_prefix, (String) " Wifi connection is " + networkConnected);
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
            this -> rlog -> log(log_prefix, (String) "Last will is set. Retain: " + lastWillRetain);
            
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
                    this -> rlog -> log(log_prefix, (String) "MQTT connection failed! Error code = " + client -> connectError());
                    this -> errorCodeChanged->fire(ERROR_MQTT);
                } else {
                    rlog -> log(log_prefix, "Connection started.");
                }
            } else {
                // rlog -> log(log_prefix, "MQTT connection info is missing.");
            }
        }

        void processMessage() {
             // we received a message, print out the topic and contents
            this -> rlog -> log(log_prefix, (String) "Message received on topic: " + client -> messageTopic());
            
            String message = "";
            while (client -> available()) {
                message += (char) client -> read();
            }
            // Broadcast MQTT message
            this -> mqttMessageArrived->fire(message);
            this -> rlog -> log(log_prefix, (String) "Message: " + message);
        }

        void subscribeForBaseTopic () {

            setLastWill();

            // subscribe to a topic and send an 'I'm alive' message
            String subscription = baseTopic + MQTT_IN_POSTFIX + "/#";
            client -> subscribe(subscription);
            sendMqttMessage(baseTopic, "{\"status\": \"" + statusOn + "\", \"ip\":\"" + this -> deviceIPAddress + "\"}");
            rlog -> log(log_prefix, "Subscribed to topic " + subscription);
            
            

            this -> errorCodeChanged->fire(ERROR_NO_ERROR);
            subscribed = true;
        }
        
        
};

#endif
