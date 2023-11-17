#ifndef WIFINETWORK_H
#define WIFINETWORK_H

#include <WiFi.h>
#include "WiFiUdp.h"
#include <Callback.h>
#include <ESPmDNS.h>
#include <DNSServer.h>
#include "log.h"
#include "database.h"

class WifiNetwork {

    public:

        Logger logger;
        Signal<boolean>* wifiStatusChanged;
        Signal<int>* errorCodeChanged;
        Signal<String>* ipAddressChanged;
        bool wifi_connected;
        DNSServer dnsServer; 
        Database* database;

        String ssid;
        String password;

        int tries;

        WifiNetwork(Log& rlog);
        void setup(Database &database, Signal<boolean> &wifiStatusChanged, Signal<int> &errorCodeChanged, Signal<String> &ipAddressChanged);
        void loop();
        void connectWifi();
        void disconnectWifi();
        void connectToAP();
        void createAP();
        void stopAP();
        boolean isConnected();
        

    private:

        void WiFiEvent(WiFiEvent_t event);
        // when wifi connects
        void wifiOnConnect();
        // when wifi disconnects
        void wifiOnDisconnect();
        // while wifi is connected
        void wifiConnectedLoop();
        // while wifi is not connected
        void wifiDisconnectedLoop();
        void setupMDNS();
        void configAP();
};

#endif
