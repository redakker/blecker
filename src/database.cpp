#ifndef DB
#define DB

// Json based library
// data

#include "definitions.h"
#include <EEPROM.h>
#include <ArduinoJson.h> // version 6
#include "log.cpp"

class Database {

    Log* rlog;
    String log_prefix = "[STORE] ";
    StaticJsonDocument<1000> jsonData;    

    public: 
        Database(Log &log) {
            this -> rlog = &log;                       
        }

        void setup() {
            EEPROM.begin(EEPROM_SIZE);
            this->init();
        }

        void loop() {

        }

        // Init EEPROM to check/set the identification
        void init() {
            String name = this -> getValueAsString("name", true);

            if (name == BOARD_NAME) {
                rlog -> log(log_prefix, "Init ready.");
            } else {
                rlog -> log(log_prefix, "Board name was not found, reinit the database.");
                jsonData.clear();
                this->updateProperty("name", BOARD_NAME, true);
            }
        }

        // Read all from the store
        void load() {
            String data = EEPROM.readString(0);
            rlog -> log(log_prefix, "data loaded: " + data);
            DeserializationError error = deserializeJson(jsonData, data);

            if (error) {
                rlog -> log(log_prefix + "DeserializationError: " + error.c_str());
                jsonData.clear();          
            } else {
                rlog -> log(log_prefix, "Data successfully parsed");
            }
        }

        // Always rewrite all data
        // Use it carefully
        void save() {

            String data = "";
            serializeJson(jsonData, data);
            
            // Clean the store first
            reset();

            EEPROM.writeString(0, data);
            EEPROM.commit();
            rlog -> log(log_prefix, "data saved: " + data);
        }

        void updateProperty(String property, String value) {            
            updateProperty(property, value, false);
        }

        // Update a property in a json data strucure.
        // Save to the store if saveValues is true
        void updateProperty(String property, String value, boolean saveValues) {            
            int str_len = property.length() + 1;
            char prop[str_len];
            property.toCharArray(prop, str_len);

            this->jsonData[prop] = value;
            if (saveValues) {
                save();             
            }
        }

        String getValueAsString(String name) {
            return getValueAsString(name, false);
        }

        String getValueAsString(String name, bool loadbefore) {
            if (loadbefore){
                load();
            }            

            if (jsonData.containsKey(name)){
                return jsonData[name.c_str()].as<String>();
            } else {
                return (String) "";
            }
        }

        int getValueAsInt(String name) {
            return getValueAsInt(name, false);
        }

        int getValueAsInt(String name, bool loadbefore) {
            if (loadbefore){
                load();
            }
            
            int ret = -1;
            if (jsonData.containsKey(name)) {
                String value = jsonData[name.c_str()].as<String>();            
                
                if (isNumeric(value)) {
                    ret = value.toInt();
                }
            }

            return ret;
        }

        boolean isPropertyExists(String property) {
            return jsonData.containsKey(property);
        }

        String getSerialized() {
            String output;
            serializeJson(jsonData, output);
            return output;
        }

        void jsonToDatabase(String json) {
            
            StaticJsonDocument<1000> tempJson; 
            DeserializationError error = deserializeJson(tempJson, json);

            if (error) {
                rlog -> log(log_prefix + (String) "DeserializationError: " + error.c_str() + " (jsonToDatabase) " + json);
            } else {
                rlog -> log(log_prefix, "Data successfully parsed during jsonToDatabase process. Data: " + json);
                // Save mechanism from hackers
                // Data alaways have a name property, because the system initialize the EEPROM if the format is not correct.
                // See the init() function
                // Check the if name is available and it is the same az a board name. In case of matching, save the data
                String value = tempJson["name"].as<String>();
                
                if (String(BOARD_NAME).equals(value)) {
                    // update/add properties individually, overwrite the wole database remove some other properties from other settings source (MQTT ledstrip)
                    JsonObject documentRoot = tempJson.as<JsonObject>();
                    for (JsonPair keyValue : documentRoot) {
                        if (strcmp(keyValue.key().c_str(),"command") != 0)
                        this -> updateProperty(keyValue.key().c_str(), keyValue.value().as<String>());
                    }
                    save();
                } else {
                    rlog -> log(log_prefix, "Json data is not valid, database was not overwritten.");
                }
                
            }
        }

        void receiveCommand(String message) {
            StaticJsonDocument<1000> tempJson; 
            DeserializationError error = deserializeJson(tempJson, message);

            if (error) {
                rlog -> log(log_prefix + (String) "DeserializationError: " + error.c_str() + " (receiveCommand) " + message);
            } else {               
                String value = tempJson["command"].as<String>();                
                if (String(COMMAND_CONFIG).equals(value)) {
                    rlog -> log(log_prefix, (String) "Command received: " + COMMAND_CONFIG);                    
                    this -> jsonToDatabase(message);
                }
                
            }
        }

        void reset(){
            // Reset settings
            rlog -> log(log_prefix + "Clear EEPROM");
            for (int i = 0; i < EEPROM_SIZE; ++i) { EEPROM.write(i, 0); }
            EEPROM.commit();
            rlog -> log(log_prefix + "EEPROM is clean.");
        }

    private:
        boolean isNumeric(String str) {
            unsigned int stringLength = str.length();

            if (stringLength == 0) {
                return false;
            }

            boolean seenDecimal = false;

            for(unsigned int i = 0; i < stringLength; ++i) {
                if (isDigit(str.charAt(i))) {
                    continue;
                }

                if (str.charAt(i) == '.') {
                    if (seenDecimal) {
                        return false;
                    }
                    seenDecimal = true;
                    continue;
                }
                return false;
            }
            return true;
        }

};

#endif
