#ifndef DEFINITIONS_H
#define DEFINITIONS_H

// VERSION
#define MAJOR_VERSION 1.11

// Board specific setings
#define BOARD_NAME "blecker"
#define LED_BUILTIN 2
#define EEPROM_SIZE 1024
#define SERIAL_SPEED 115200

// Software settings
#define SERVER_PORT 80
#define MQTT_MAX_TRY 10 // give the connect up after this amount of tries
#define MQTT_TOPIC "/" BOARD_NAME
#define MQTT_IN_POSTFIX "/in"

#define MQTT_STATUS_ON_DEFAULT_VALUE "on"
#define MQTT_STATUS_OFF_DEFAULT_VALUE "off"
#define MQTT_STATUS_DEFAULT_RETAIN false

#define MQTT_BEACON_PRESENCE_ON_DEFAULT_VALUE "on"
#define MQTT_BEACON_PRESENCE_OFF_DEFAULT_VALUE "off"
#define MQTT_BEACON_PRESENCE_DEFAULT_RETAIN false

#define MQTT_KEEPALILIVE_TIME 30000 // in milliseconds

// Presence
#define DEFAULT_PRESENT "present"
#define DEFAULT_NOT_PRESENT "not_present"
#define BT_DEVICE_TIMEOUT 1000*60 // 60 seconds in milliseconds (is is marked as gone, drop out when DEVICE_DROP_OUT_COUNT is decreased to 0)
#define BT_LIST_REBUILD_INTERVAL 1000*60*60 // Just clear the list after every hour and rebuild again, send "refresh" state time to time even if the device is not gone
#define DEVICE_DROP_OUT_COUNT 2 // We won't drop out in the first "not found" state, just decrease this value. Drop out when this is 0
#define PARSE_CHAR ";"
#define BT_DEFAULT_SCAN_DURATION_IN_SECONDS 10 // Scan is running for this time and repeats

// Webhook
#define PRESENCE_WILDCARD "{presence}"
#define DEVICE_WILDCARD "{device}"

// Auto Discovery
#define HA_AUTODISCOVERY_INTERVAL 1000*60

// Network
#define WIFI_MAX_TRY 10
#define WIFI_OFF_REBOOT_TIME 5*60*1000 // If SSID is configured, but still AP is created restart after this time. Ex: Wifi router is gone -> device goes to AP mode. Without this it never tries to connect again. Just after a reboot
#define AP_IP {192, 168, 4, 1} // Change together with the string version
#define AP_IP_STRING "192.168.4.1" // Change together with the object version
#define AP_NETMASK {255, 255, 255, 0}

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
#define DB_HA_AUTODISCOVERY "hadisc"
#define DB_HA_AUTODISCOVERY_PREFIX "hadiscpref"
#define DB_REBOOT_TIMEOUT "reboot"
#define DB_WEBHOOK "webhook"
#define DB_DEVICE_STATUS_ON "status_on"
#define DB_DEVICE_STATUS_OFF "status_off"
#define DB_DEVICE_STATUS_RETAIN "status_retain"
#define DB_DEVICE_ID "deviceid"
#define DB_BEACON_PRESENCE_RETAIN "presence_retain"

#endif
