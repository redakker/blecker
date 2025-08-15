#include "mqtt.h"

Mqtt::Mqtt(Log& rlog) : logger(rlog, "[MQTT]") {

    deviceIPAddress = "undefined";

    // MQTT connect try
    lasttry = 10000;
    maxtry = 10;
    MQTTconnectTime = 0;

    // Keepalive timeout
    lastKeepAliveTime = 0;

    networkConnected = false; // Connected to the network (Wifi STA)
    subscribed = false;
    deviceStatusRetain = MQTT_STATUS_DEFAULT_RETAIN;
    lastiWillSet = false;
    mqtt_connected = false;

    this -> client = new MqttClient(wifiClient);
}

void Mqtt::setup(Database &database, Signal<boolean> &mqttStatusChanged, Signal<int> &errorCodeChanged, Signal<String> &mqttMessageArrived) {

    this -> database = &database;
    this -> mqttStatusChanged = &mqttStatusChanged;
    this -> errorCodeChanged = &errorCodeChanged;
    this -> mqttMessageArrived = &mqttMessageArrived;

    this -> user = this -> database -> getValueAsString(String(DB_MQTT_USER), false);
    this -> password = this -> database -> getValueAsString(String(DB_MQTT_PW), false);
    this -> port =  this -> database -> getValueAsInt(String(DB_MQTT_PORT), false);
    this -> server = this -> database -> getValueAsString(String(DB_MQTT_SERVER), false);
    this -> baseTopic = this -> database -> getValueAsString(String(DB_MQTT_TOPIC_PREFIX), false) + MQTT_TOPIC;

    this -> deviceStatusRetain = this -> database -> getValueAsBoolean(String(DB_DEVICE_STATUS_RETAIN), false, MQTT_STATUS_DEFAULT_RETAIN);


    if (this -> database -> isPropertyExistsAndNonEmpty(DB_DEVICE_STATUS_ON)) {
        this -> statusOn = this -> database -> getValueAsString(String(DB_DEVICE_STATUS_ON), false);
    } else {
        this -> statusOn = MQTT_STATUS_ON_DEFAULT_VALUE;
    }

    if (this -> database -> isPropertyExistsAndNonEmpty(DB_DEVICE_STATUS_OFF)) {
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

void Mqtt::loop() {
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
        this -> subscribed = false;
        mqtt_connected = false;
        // Emit an event about the MQTT status
        mqttStatusChanged->fire(mqtt_connected);
    }

}

void Mqtt::setConnected (boolean networkConnected) {
    this->logger << "Wifi connection is " << (String)networkConnected;
    this->networkConnected = networkConnected;
}

void Mqtt::sendMqttMessage(MQTTMessage message) {
    if (client->connected()) {
        String topic = baseTopic + "/";
        if (message.individualTopic) {
            topic = "";
        }
        sendMqttMessage(topic + message.topic, message.payload, message.retain);
    }
}

void Mqtt::ipAddressChanged (String ipAddress) {
    this -> deviceIPAddress = ipAddress;
}

void Mqtt::setLastWill() {

    String message = String("{\"status\": \"" + statusOff + "\", \"ip\":\"" + this -> deviceIPAddress + "\"}");

    client -> beginWill(baseTopic, message.length(), deviceStatusRetain, 1);
    client -> print(message);
    client -> endWill();
    logger << "Last will is set. Retain: " << (String)deviceStatusRetain;

}

void Mqtt::sendMqttMessage(String topic, String message, boolean retain = false) {
    if (networkConnected) {
        client -> beginMessage(topic, retain);
        client -> print(message);
        client -> endMessage();
    }
}

void Mqtt::reconnect() {
    if (!String("").equals(server)) {
        const char* mqtt_s = const_cast<char*>(server.c_str());

        if (millis() - MQTTconnectTime > 15 * 1000) // 15 sec
        {
            logger << "Connecting to MQTT server...";
            setLastWill();
            if (!client->connect(mqtt_s, port))
            {
               logger << "MQTT connection failed! Error code = " << (String)client -> connectError();
               this->errorCodeChanged->fire(ERROR_MQTT);
               mqtt_connected = false;
               // Emit an event about the MQTT status
               mqttStatusChanged->fire(mqtt_connected);
            }
            else
            {
               sendChipInfo();
               logger << "MQTT Connection started/checked.";
               mqtt_connected = true;
               // Emit an event about the MQTT status
               mqttStatusChanged->fire(mqtt_connected);
            }

            MQTTconnectTime = millis();
        }
    } else {
        // logger << "MQTT connection info is missing.");
    }
}

void Mqtt::processMessage() {
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

void Mqtt::subscribeForBaseTopic () {

    //setLastWill();

    // subscribe to a topic and send an 'I'm alive' message
    String subscription = baseTopic + MQTT_IN_POSTFIX + "/#";
    client -> subscribe(subscription);
    logger << "Subscribed to topic " << subscription;

    this -> errorCodeChanged->fire(ERROR_NO_ERROR);
    subscribed = true;
}

void Mqtt::sendStatus () {
    sendMqttMessage(baseTopic, "{\"status\": \"" + statusOn + "\", \"ip\":\"" + this -> deviceIPAddress + "\"}", deviceStatusRetain);

}

void Mqtt::sendChipInfo() {
    sendMqttMessage(baseTopic + "/chipinfo", "{\"model\": \"" + (String) getChipModelString(chip_info.model) + "\", \"cores\":\"" + chip_info.cores + "\", \"revision\":\"" + chip_info.revision + "\"}", true);
}
