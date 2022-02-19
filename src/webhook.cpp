#ifndef HTTPCLIENT
#define HTTPCLIENT

#include "definitions.h"
#include "utilities.cpp"
#include <Arduino.h>
#include <HTTPClient.h>
#include "log.cpp"

class Webhook {

    Log* rlog;
    String log_prefix = "[HTTPCLIENT] ";
    Database* database;
    HTTPClient http;
    
    boolean webhookConfigured = false;
    unsigned long lastrun = 0;
    
    public:
        Webhook(Log &rlog) {
            this -> rlog = &rlog;
            
        }

        void setup (Database &database) {
            this -> database = &database;

            if (
                this->database->getValueAsString(DB_WEBHOOK)
            ) {
                this-> webhookConfigured = true;
                rlog -> log(log_prefix, (String) "Webhook is configured with the following URL:" + this->database->getValueAsString(DB_WEBHOOK));
            } else {
                this-> webhookConfigured = true;
                rlog -> log(log_prefix, (String) "Webhook is not configured, will be not used.");
            }
        }

        void loop() {
  
        }

        void callWebhook (Device device) {

            if (webhookConfigured) {
                String baseURL = this->database->getValueAsString(DB_WEBHOOK);
                
                baseURL.replace(DEVICE_WILDCARD, device.mac);
                if (device.available) {
                    baseURL.replace(PRESENCE_WILDCARD, getPresentString(*database, true));
                } else {
                    baseURL.replace(PRESENCE_WILDCARD, getPresentString(*database, false));
                }

                http.begin(baseURL); //Specify the URL
                int httpCode = http.GET();                                        //Make the request
            
                if (httpCode > 0) { //Check for the returning code
            
                    String payload = http.getString();
                    
                    rlog -> log(log_prefix, (String) "URL is called with GET method: " + baseURL);
                    rlog -> log(log_prefix, (String) "Status code: " + (String) httpCode);
                    rlog -> log(log_prefix, (String) "Payload: " + payload);

                } else {
                    Serial.println("Error on HTTP request");
                    rlog -> log(log_prefix, (String) "Error on http request");
                }
            
                http.end(); //Free the resources

            }

            
        }

    private:
     
};

#endif