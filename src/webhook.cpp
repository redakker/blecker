#include "webhook.h"


Webhook::Webhook(Log& rlog) : logger(rlog, "[HTTPCLIENT]") {
    webhookConfigured = false;
    lastrun = 0;
}

void Webhook::setup (Database &database) {
    this -> database = &database;

    if (this->database->getValueAsString(DB_WEBHOOK) != "") {
        this-> webhookConfigured = true;
        logger << "Webhook is configured with the following URL:" << this->database->getValueAsString(DB_WEBHOOK);
    } else {
        this-> webhookConfigured = false;
        logger << "Webhook is not configured, will be not used.";
    }
}

void Webhook::loop() {
}



void Webhook::callWebhook(Device device) {
    if (webhookConfigured) {
        String baseURL = this->database->getValueAsString(DB_WEBHOOK);
        baseURL.toLowerCase();

        baseURL.replace(DEVICE_WILDCARD, device.mac);
        if (device.available) {
            baseURL.replace(PRESENCE_WILDCARD, database->getPresentString(true));
        } else {
            baseURL.replace(PRESENCE_WILDCARD, database->getPresentString(false));
        }

        HTTPClient http;
        if (baseURL.startsWith("https://")) {
            // HTTPS handling
            WiFiClientSecure client;
            client.setInsecure(); // Disable certificate verification (use only if you trust the server)
            client.setNoDelay(true);
            // Additional SSL/TLS configuration to fully disable certificate checking
            client.setCACert(NULL);
            client.setCertificate(NULL);
            client.setPrivateKey(NULL);

            if (http.begin(client, baseURL)) { // Initialize HTTPS connection
                int httpCode = http.GET(); // Make the request

                if (httpCode > 0) {
                    String payload = http.getString();
                    logger << "URL is called with GET method (HTTPS): " << baseURL;
                    logger << "Status code: " << (String)httpCode;
                    logger << "Payload: " << payload;
                } else {
                    Serial.println("Error on HTTPS request");
                    logger << "Error on HTTPS request: " << (String)http.errorToString(httpCode);
                }

                http.end(); // Free the resources
            } else {
                Serial.println("Unable to connect to the HTTPS server");
                logger << "Unable to connect to the HTTPS server: " << baseURL;
            }
        } else if (baseURL.startsWith("http://")) {
            // HTTP handling
            WiFiClient client;

            if (http.begin(client, baseURL)) { // Initialize HTTP connection
                int httpCode = http.GET(); // Make the request

                if (httpCode > 0) {
                    String payload = http.getString();
                    logger << "URL is called with GET method (HTTP): " << baseURL;
                    logger << "Status code: " << (String)httpCode;
                    logger << "Payload: " << payload;
                } else {
                    Serial.println("Error on HTTP request");
                    logger << "Error on HTTP request: " << (String)http.errorToString(httpCode);
                }

                http.end(); // Free the resources
            } else {
                Serial.println("Unable to connect to the HTTP server");
                logger << "Unable to connect to the HTTP server: " << baseURL;
            }
        } else {
            Serial.println("Invalid URL protocol");
            logger << "Invalid URL protocol: " << baseURL;
        }
    }
}
