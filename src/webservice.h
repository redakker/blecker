#ifndef WEBSERVERVICE_H
#define WEBSERVERVICE_H

#include <WebServer.h>
#include <ArduinoOTA.h>
#include <Update.h>
#include "utilities.h"
#include "database.h"
#include "log.h"
#include "webcontent.h"

class Webservice {
    
    Logger logger;
    Database* database;
    WebServer server;

    boolean networkConnected = false;

    public:
        Webservice(Log& rlog);
        void setup(Database &database);
        void loop();
        String getData();
        void setConnected(boolean connected);

    private:
        void sendHeaders();
        void handleRoot();
        void handleJavaScript();
        void handleStyle();
        void handleNormalize();
        void handleSkeleton();
        void handleLogo();
        void handleData();
        void handleChipInfo();
        void handleFavicon();         
        void handleSaveData();
        void handleReset();
        void handleUpdate();
        void handleUpgradeFn();
        void handleUpgradeUFn();
        void handleNotFound();
        void captivePortal();
};

#endif
