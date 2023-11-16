#ifndef HTTPCLIENT
#define HTTPCLIENT

#include "definitions.h"
#include "utilities.cpp"
#include <Arduino.h>
#include <HTTPClient.h>
#include "log.hpp"

class Webhook {

    Logger logger;
    Database* database;
    HTTPClient http;    
    
    boolean webhookConfigured = false;
    unsigned long lastrun = 0;
    
    public:
        Webhook(Log& rlog) : logger(rlog, "[HTTPCLIENT]") {
        }

        void setup (Database &database) {
            this -> database = &database;

            if (this->database->getValueAsString(DB_WEBHOOK) != "") {
                this-> webhookConfigured = true;
                logger << "Webhook is configured with the following URL:" << this->database->getValueAsString(DB_WEBHOOK);
            } else {
                this-> webhookConfigured = false;
                logger << "Webhook is not configured, will be not used.";
            }
        }

        void loop() {
        }

        void callWebhook(Device device) {

            if (webhookConfigured) {
                String baseURL = this->database->getValueAsString(DB_WEBHOOK);
                baseURL.toLowerCase();
                
                baseURL.replace(DEVICE_WILDCARD, device.mac);
                if (device.available) {
                    baseURL.replace(PRESENCE_WILDCARD, getPresentString(*database, true));
                } else {
                    baseURL.replace(PRESENCE_WILDCARD, getPresentString(*database, false));
                }
               
                http.begin(baseURL); //Specify the URL               
                
                int httpCode = http.GET(); //Make the request
            
                if (httpCode > 0) { //Check for the returning code
            
                    String payload = http.getString();
                    
                    logger << "URL is called with GET method: " << baseURL;
                    logger << "Status code: " << (String)httpCode;
                    logger << "Payload: " << payload;

                } else {
                    Serial.println("Error on HTTP request");
                    logger << "Error on http request";
                }
            
                http.end(); //Free the resources
            }

            
        }

    private:
     
};

#endif