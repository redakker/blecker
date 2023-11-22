#include "wifinetwork.h"

WifiNetwork::WifiNetwork(Log& rlog) : logger(rlog, "[WIFI]") {
    wifi_connected = false;
    tries = 0;
    APstart = 0;
}

void WifiNetwork::setup(Database &database, Signal<boolean> &wifiStatusChanged, Signal<int> &errorCodeChanged, Signal<String> &ipAddressChanged){

    this -> database = &database;

    this -> ssid = this -> database -> getValueAsString(String(DB_WIFI_NAME), false);
    this -> password = this -> database -> getValueAsString(String(DB_WIFI_PASSWORD), false);

    this -> wifiStatusChanged = &wifiStatusChanged;
    this -> errorCodeChanged = &errorCodeChanged;
    this -> ipAddressChanged = &ipAddressChanged;

    uniqueBoardname = BOARD_NAME "_" + WiFi.macAddress();
    uniqueBoardname.replace(":","");
    logger << "Unique board name (hostname) is: " << uniqueBoardname;

    WiFi.onEvent(
    [this](WiFiEvent_t event, WiFiEventInfo_t info) {
        this->WiFiEvent(event);
    });

    configAP();        
    WiFi.mode(WIFI_AP_STA);

    setupMDNS();
}

void WifiNetwork::connectWifi() {            
    if (ssid.length() > 0) {
        this -> connectToAP();
    } else {
        logger << "Cannot connect to wifi, because no SSID was defined. Create an AP.";
        createAP();
    }
    
}

void WifiNetwork::disconnectWifi() {
    WiFi.disconnect();
    logger << "Wifi is disconnected from a function.";
}

void WifiNetwork::connectToAP() {
    WiFi.mode(WIFI_STA);
    WiFi.config(INADDR_NONE, INADDR_NONE, INADDR_NONE, INADDR_NONE);
    WiFi.begin(const_cast<char*>(ssid.c_str()), const_cast<char*>(password.c_str()));            
    WiFi.setHostname(uniqueBoardname.c_str());
}

void WifiNetwork::createAP() {
    configAP();
    WiFi.mode(WIFI_AP);
    APstart = millis();
    logger << "AP is created from a function. Name: " << BOARD_NAME;
}

void WifiNetwork::stopAP() {            
    WiFi.softAPdisconnect();
    WiFi.enableAP(false);
    logger << "AP disconnected from a function.";
}

boolean WifiNetwork::isConnected() {
    return wifi_connected;
}     

void WifiNetwork::loop() {
    if(wifi_connected) {
        wifiConnectedLoop();
    } else {
        wifiDisconnectedLoop();
        dnsServer.processNextRequest();
    }
}

void WifiNetwork::WiFiEvent(WiFiEvent_t event) {
    switch(event) {

        case SYSTEM_EVENT_AP_START:
            //can set ap hostname here
            WiFi.softAPsetHostname(uniqueBoardname.c_str());
            //enable ap ipv6 here
            // WiFi.softAPenableIpV6();

            logger << "AP started. SSID: " << BOARD_NAME << " AP IPv4: " << WiFi.softAPIP().toString();
            break;

        case SYSTEM_EVENT_STA_START:
            //set sta hostname here
            WiFi.setHostname(uniqueBoardname.c_str());
            logger << "Wifi hostname set to " << uniqueBoardname;
            break;
        case SYSTEM_EVENT_STA_CONNECTED:
            // enable sta ipv6 here
            // WiFi.enableIpV6();
            break;
        case SYSTEM_EVENT_AP_STA_GOT_IP6:
            //both interfaces get the same event
            // log -> log("STA IPv6: ");
            // log -> log(WiFi.localIPv6());
            // log -> log("AP IPv6: ");
            // log -> log(WiFi.softAPIPv6());
            break;
        case SYSTEM_EVENT_STA_GOT_IP:
            wifiOnConnect();
            break;
        case SYSTEM_EVENT_STA_DISCONNECTED:                                       
            wifiOnDisconnect();
            break;
        default:
            break;
    }
}

// when wifi connects
void WifiNetwork::wifiOnConnect() {
    tries = 0;            
    stopAP();
    wifi_connected = true;
    
    // Emit a NO_ERROR event about the Wifi status
    errorCodeChanged->fire(ERROR_NO_ERROR);
    
    // Emit the IP address. This first, to have it in the last will message in the MQTT. If the wifi status emitted first, the ip address change signal will be late to set
    ipAddressChanged->fire(WiFi.localIP().toString());
    // Emit an event about the Wifi status
    wifiStatusChanged->fire(wifi_connected);            
    logger << "STA Connected. STA SSID: " << WiFi.SSID() << " STA IPv4: " << WiFi.localIP().toString() << ", GW: " << WiFi.gatewayIP().toString() << ", Mask: " << WiFi.subnetMask().toString() << ", DNS: " << WiFi.dnsIP().toString();
    
}

// when wifi disconnects
void WifiNetwork::wifiOnDisconnect() {
    logger << "Disconnected.";
    wifi_connected = false;
    
    // Emit an event about the Wifi status
    wifiStatusChanged->fire(wifi_connected);
    
    // Try agan till WIFI_MAX_TRY
    this->tries++;
    if (this->tries > WIFI_MAX_TRY) {
        WiFi.disconnect();
        logger << "Final disconnect.";
        errorCodeChanged->fire(ERROR_WIFI);
        this -> createAP();
    } else {
        this -> connectToAP();
    }
    //this -> connectWifi();
}

// while wifi is connected
void WifiNetwork::wifiConnectedLoop() {            
}

// while wifi is not connected
void WifiNetwork::wifiDisconnectedLoop() {
    // Try to reconnect time to time    
    if ((millis() - APstart) > WIFI_OFF_REBOOT_TIME) {
        if (ssid.length() > 0) {           
           ESP.restart();
        }
    }
}

void WifiNetwork::setupMDNS() {
    if(!MDNS.begin(BOARD_NAME)) {
        logger << "Error starting mDNS";
        //return;
    } else {
        MDNS.addService("http", "tcp", 80);
    }
}

void WifiNetwork::configAP() {
    WiFi.softAP(BOARD_NAME);

    // For captive portal
    dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
    dnsServer.start(53, "*", AP_IP);

    WiFi.softAPConfig(AP_IP, AP_IP, AP_NETMASK);
}
