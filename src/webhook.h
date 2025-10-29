#ifndef WEBHOOK_H
#define WEBHOOK_H

#include "definitions.h"
#include "utilities.h"
#include <Arduino.h>
#include <HTTPClient.h>
#include <WiFiClient.h>
#include <WiFiClientSecure.h>
#include "log.h"
#include "database.h"

class Webhook {

    Logger logger;
    Database* database;
    HTTPClient http;

    boolean webhookConfigured;
    unsigned long lastrun;

    public:
        Webhook(Log& rlog);
        void setup (Database &database);
        void loop();
        void callWebhook(Device device);

    private:

};

#endif