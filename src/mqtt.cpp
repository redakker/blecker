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
    String status_on;
    String status_off;
    int port;
    String baseTopic;

    String deviceIPAddress = "undefined";

    // MQTT connect try
    int lasttry = 10000;
    int maxtry = 10;

    boolean networkConnected = false; // Connected to the network (Wifi STA)
    boolean subscribed = false;

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
            // this -> status_on = this -> database -> getValueAsString(String(DB_DEVICE_STATUS_ON), false);
            // this -> status_off = this -> database -> getValueAsString(String(DB_DEVICE_STATUS_OFF), false);

            String status_on = this -> database -> getValueAsString(String(DB_DEVICE_STATUS_ON), false);
            String status_off = this -> database -> getValueAsString(String(DB_DEVICE_STATUS_OFF), false);

            if (status_on == "") {
                status_on = MQTT_STATUS_ON_DEFAULT_VALUE;
            }

            if (status_off == "") {
                status_off = MQTT_STATUS_OFF_DEFAULT_VALUE;
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
                    reconnect();
                }
            } else {
                client->stop();
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
            client -> beginWill(baseTopic, String("{\"status\": \"" + status_off + "\"}").length(), false, 1);
            client -> print(String("{\"status\": \"" + status_off + "\", ip:\"" + this -> deviceIPAddress + "\"}"));            
            client -> endWill();
            this -> rlog -> log(log_prefix, (String) "Last will is set.");
            
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
            // subscribe to a topic and send an 'I'm alive' message
            String subscription = baseTopic + MQTT_IN_POSTFIX + "/#";
            client -> subscribe(subscription);
            sendMqttMessage(baseTopic, "{\"status\": \"" + status_on + "\", ip:\"" + this -> deviceIPAddress + "\"}");
            rlog -> log(log_prefix, "Subscribed to topic " + subscription);
            
            setLastWill();

            this -> errorCodeChanged->fire(ERROR_NO_ERROR);
            subscribed = true;
        }
        
        
};

#endif
