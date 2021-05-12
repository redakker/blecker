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
    int port;
    String baseTopic;

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
            
            this -> client -> setUsernamePassword(user, password);
            
        }

        void loop() {
            if (networkConnected) {
                // check for incoming messages            
                if (client->connected()) {

                    if (!subscribed) {
                        subscribeFroBaseTopic();
                    }

                    int messageSize = client -> parseMessage();
                    if (messageSize) {
                       processMessage();
                    }
                } else {
                    reconnect();
                }
            }
        }

        void setConnected (boolean networkConnected) {
            this -> rlog -> log(log_prefix, (String) " Wifi connection is " + networkConnected);
            this->networkConnected = networkConnected;            
        }

        void sendMqttMessage(MQTTMessage message) {
            if (client->connected()) {
                sendMqttMessage(baseTopic + "/" + message.topic, message.payload, message.retain);
            }
        }

    private:
    
        void setLastWill() {
            client -> beginWill(baseTopic, String(MQTT_STATUS_OFF).length(), false, 1);
            client -> print(String(MQTT_STATUS_OFF));            
            client -> endWill();
            this -> rlog -> log(log_prefix, (String) "Last will is set.");
            
        }

        void sendMqttMessage(String topic, String message, boolean retain = false) {
            client -> beginMessage(topic);
            if (retain) {
                client->messageRetain();
            }
            client -> print(message);            
            client -> endMessage();
        }

        void reconnect() {
            if (!String("").equals(server) && !String("").equals(user)) {
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

        void subscribeFroBaseTopic () {
            // subscribe to a topic and send an 'I'm alive' message
            String subscription = baseTopic + MQTT_IN_POSTFIX + "/#";
            client -> subscribe(subscription);
            sendMqttMessage(baseTopic, MQTT_STATUS_ON);
            rlog -> log(log_prefix, "Subscribed to topic " + subscription);
            
            setLastWill();

            this -> errorCodeChanged->fire(ERROR_NO_ERROR);
            subscribed = true;
        }
        
        
};

#endif
