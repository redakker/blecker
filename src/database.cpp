#include "database.h"

Database::Database(Log& rlog) : logger(rlog, "[STORE]") {
}

void Database::setup() {
    EEPROM.begin(EEPROM_SIZE);
    this->init();
}

void Database::loop() {
}

// Init EEPROM to check/set the identification
void Database::init() {
    String name = this -> getValueAsString("name", true);

    if (name == BOARD_NAME) {
        logger << "Init ready.";
    } else {
        logger << "Board name was not found, reinit the database.";
        jsonData.clear();
        this->updateProperty("name", BOARD_NAME, true);
    }
}

// Read all from the store
void Database::load() {
    String data = EEPROM.readString(0);
    logger << "data loaded: " << data;
    DeserializationError error = deserializeJson(jsonData, data);

    if (error) {
        logger << "DeserializationError: " << error.c_str();
        jsonData.clear();          
    } else {
        logger << "Data successfully parsed";
    }
}

// Always rewrite all data
// Use it carefully
void Database::save() {

    String data = "";
    serializeJson(jsonData, data);
    
    // Clean the store first
    reset();

    EEPROM.writeString(0, data);
    EEPROM.commit();
    logger << "data saved: " << data;
}

void Database::updateProperty(String property, String value) {            
    updateProperty(property, value, false);
}

// Update a property in a json data strucure.
// Save to the store if saveValues is true
void Database::updateProperty(String property, String value, boolean saveValues) {            
    int str_len = property.length() + 1;
    char prop[str_len];
    property.toCharArray(prop, str_len);

    this->jsonData[prop] = value;
    if (saveValues) {
        save();             
    }
}

String Database::getValueAsString(String name) {
    return getValueAsString(name, false);
}

String Database::getValueAsString(String name, bool loadbefore) {
    if (loadbefore){
        load();
    }            

    if (jsonData.containsKey(name)){
        return jsonData[name.c_str()].as<String>();
    } else {
        return (String) "";
    }
}

int Database::getValueAsInt(String name) {
    return getValueAsInt(name, false);
}

int Database::getValueAsInt(String name, bool loadbefore) {
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

boolean Database::getValueAsBoolean(String name, bool loadbefore, bool defaultReturn) {
    if (loadbefore){
        load();
    }

    if (isPropertyExists(name)) {
        String value = jsonData[name.c_str()].as<String>();
        return !value.isEmpty() && (strcasecmp (value.c_str (), "true") == 0 || atoi (value.c_str ()) != 0);
    } else {
        return defaultReturn;
    }
    return false;
}

boolean Database::isPropertyExists(String property) {
    return jsonData.containsKey(property);
}

boolean Database::isPropertyExistsAndNonEmpty(String property) {
    if (isPropertyExists(property)) {
        return !getValueAsString(property).isEmpty();
    }
    return false;
}

String Database::getSerialized() {
    String output;
    serializeJson(jsonData, output);
    return output;
}

void Database::jsonToDatabase(String json) {
    
    StaticJsonDocument<1000> tempJson; 
    DeserializationError error = deserializeJson(tempJson, json);

    if (error) {
        logger << "DeserializationError: " << error.c_str() << " (jsonToDatabase) " << json;
    } else {
        logger << "Data successfully parsed during jsonToDatabase process. Data: " << json;
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
            logger << "Json data is not valid, database was not overwritten.";
        }
        
    }
}

void Database::receiveCommand(String message) {
    StaticJsonDocument<1000> tempJson; 
    DeserializationError error = deserializeJson(tempJson, message);

    if (error) {
        logger << "DeserializationError: " << error.c_str() << " (receiveCommand) " << message;
    } else {               
        String value = tempJson["command"].as<String>();                
        if (String(COMMAND_CONFIG).equals(value)) {
            logger << "Command received: " << COMMAND_CONFIG;
            this -> jsonToDatabase(message);
        }
        
    }
}

void Database::reset(){
    // Reset settings
    logger << "Clear EEPROM";
    for (int i = 0; i < EEPROM_SIZE; ++i) { EEPROM.write(i, 0); }
    EEPROM.commit();
    logger << "EEPROM is clean.";
}

// Get the name of the presence string. This is not the state, just the name which will be sent over!!!
// Maybe this is not the best place, if you know better, then send me a message :)
String Database::getPresentString (boolean presenceState) {
    String presence = DEFAULT_NOT_PRESENT;

    if (presenceState) {
        presence = getValueAsString(DB_PRECENCE);
        if (presence == "") {                    
            presence = DEFAULT_PRESENT;
        }
    } else {
        presence = getValueAsString(DB_NO_PRECENCE);
        if (presence == "") {                    
            presence = DEFAULT_NOT_PRESENT;
        }                
    }

    return presence;
}

boolean Database::isNumeric(String str) {
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

