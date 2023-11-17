#ifndef DATABASE_H
#define DATABASE_H

// Json based library
// data

#include "definitions.h"
#include <EEPROM.h>
#include <ArduinoJson.h> // version 6
#include "log.h"

class Database {

    Logger logger;
    StaticJsonDocument<1000> jsonData;    

    public: 
        Database(Log& rlog);
        void setup();
        void loop();
        // Init EEPROM to check/set the identification
        void init();
        // Read all from the store
        void load();
        // Always rewrite all data
        // Use it carefully
        void save();
        void updateProperty(String property, String value);
        // Update a property in a json data strucure.
        // Save to the store if saveValues is true
        void updateProperty(String property, String value, boolean saveValues);
        String getValueAsString(String name);
        String getValueAsString(String name, bool loadbefore);
        int getValueAsInt(String name);
        int getValueAsInt(String name, bool loadbefore);
        boolean getValueAsBoolean(String name, bool loadbefore, bool defaultReturn);
        boolean isPropertyExists(String property);
        boolean isPropertyExistsAndNonEmpty(String property);
        String getSerialized();
        void jsonToDatabase(String json);
        void receiveCommand(String message);
        void reset();
        String getPresentString (boolean presenceState);

    private:
        boolean isNumeric(String str);
};

#endif
