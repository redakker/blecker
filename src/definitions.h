// VERSION
#define MAJOR_VERSION 1.02

// Board specific setings
#define BOARD_NAME "blecker"
#define LED_BUILTIN 2
#define EEPROM_SIZE 1024

// Software settings
#define SERVER_PORT 80
#define MQTT_MAX_TRY 10 // give the connect up after this amount of tries
#define MQTT_TOPIC "/blecker"
#define MQTT_IN_POSTFIX "/in"
#define MQTT_STATUS_ON "{\"status\": \"on\"}"
#define MQTT_STATUS_OFF "{\"status\": \"off\"}"

// Presence
#define DEFAULT_PRESENT "present"
#define DEFAULT_NOT_PRESENT "not_present"
#define BT_DEVICE_TIMEOUT 1000*60 // 120 seconds in milliseconds (it is doubled because of the mark mechanism) // After this time we sent a "not home" mqtt message
#define BT_LIST_REBUILD_INTERVAL 1000*60*60 // Just clear the list after every hour and rebuild again, send "refresh" state time to time even if the device is not gone
#define BT_DEVICE_DATA_INTERVAL 1000*60 // Send the BLE device data time to time
#define DEVICE_DROP_OUT_COUNT 2 // We won't drop out in the first "not found" state, just decrease this value. Drop out when this is 0
#define PARSE_CHAR ";"

// Network
#define WIFI_MAX_TRY 10

// ERRORS
#define ERROR_NO_ERROR 0
#define ERROR_UNKNOWN 1
#define ERROR_WIFI 2
#define ERROR_MQTT 8

#define COMMAND_CONFIG "config"

// DATABASE PROPERTIES
#define DB_WIFI_NAME "ssid"
#define DB_WIFI_PASSWORD "pw"
#define DB_MQTT_SERVER "mqttserver"
#define DB_MQTT_PORT "mqttport"
#define DB_MQTT_USER "mqttuser"
#define DB_MQTT_PW "mqttpw"
#define DB_MQTT_TOPIC_PREFIX "mqttprefix"
#define DB_PRECENCE "present"
#define DB_NO_PRECENCE "notpresent"
#define DB_VERSION "version"
#define DB_DEVICES "devices"
#define DB_DETAILED_REPORT "detailed"
#define DB_REBOOT_TIMEOUT "reboot"
