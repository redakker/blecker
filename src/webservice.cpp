#include "webservice.h"




    
        Webservice::Webservice(Log& rlog) : logger(rlog, "[WEB]"), server(80) {
            networkConnected = false;
        }

        void Webservice::setup(Database &database) {

            this->database = &database;

            // -- Set up required URL handlers on the web server.
            // We should bind the member function in this way to able to pass to the request function.
            // https://stackoverflow.com/questions/43479328/how-to-pass-class-member-function-as-handler-function
            server.on("/", std::bind(&Webservice::handleRoot, this));

            server.on(data_functions_js_path, HTTP_GET, std::bind(&Webservice::handleJavaScript, this));
            server.on(data_style_css_path, HTTP_GET, std::bind(&Webservice::handleStyle, this));
            server.on(data_normalize_css_path, HTTP_GET, std::bind(&Webservice::handleNormalize, this));
            server.on(data_skeleton_css_path, HTTP_GET, std::bind(&Webservice::handleSkeleton, this));
            //server.on("/logo.jpg", HTTP_GET, std::bind(&Webservice::handleLogo, this));

            server.on("/data", std::bind(&Webservice::handleData, this));

            server.on("/chipinfo", std::bind(&Webservice::handleChipInfo, this));

            // POST
            server.on("/savedata", std::bind(&Webservice::handleSaveData, this));
            
            // update
            server.on("/update", std::bind(&Webservice::handleUpdate, this));
            // upgrade
            server.on("/upgrade", HTTP_POST, std::bind(&Webservice::handleUpgradeFn, this), std::bind(&Webservice::handleUpgradeUFn, this));

            // reset
            server.on("/reset", std::bind(&Webservice::handleReset, this));
            
            // Error handling
            server.onNotFound(std::bind(&Webservice::handleNotFound, this));

            // Favicon
            server.on("/favicon.ico", std::bind(&Webservice::handleFavicon, this));
            
            server.begin();
            logger << "Webserver is ready.";
        }

        void Webservice::loop() {
            server.handleClient();
        }

        String Webservice::getData() {
            return this -> database -> getSerialized();
        }

        void Webservice::setConnected(boolean connected) {
            this -> networkConnected = connected;
        }


        void Webservice::sendHeaders() {
            server.enableCORS();
            server.enableCrossOrigin();
            server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
            server.sendHeader("Pragma", "no-cache");
            server.sendHeader("Expires","-1");
        }

        void Webservice::handleRoot() {            
            logger << "/ is called";
            server.sendHeader("Access-Control-Allow-Origin", "*");
            sendHeaders();
            server.send(200, "text/html", data_index_html);
        }

        void Webservice::handleJavaScript() {
            logger << "/function.js is called";
            server.send(200, "text/javascript", data_functions_js);
        }

        void Webservice::handleStyle() {
            logger << "/style.css is called";
            server.send(200, "text/css", data_style_css);
        }

        void Webservice::handleNormalize() {
            logger << "/normalize.css is called";
            server.send(200, "text/css", data_normalize_css);
        }

        void Webservice::handleSkeleton() {
            logger << "/skeleton.css is called";
            server.send(200, "text/css", data_skeleton_css);
        }

        void Webservice::handleLogo() {
            logger << "/logo.jpg is called";
            //server.sendContent_P(data_logo_jpg);
        }

        void Webservice::handleData() {
            logger << "/data is called";
            sendHeaders();
            server.send(200, "application/json", getData());
        }

        void Webservice::handleChipInfo() {
            logger << "/chipinfo is called";
            sendHeaders();
            server.send(200, "application/json", "{\"model\": \"" + (String) getChipModelString(chip_info.model) + "\", \"cores\":\"" + chip_info.cores + "\", \"revision\":\"" + chip_info.revision + "\"}");
        }

        void Webservice::handleFavicon() {
            logger << "/favicon is called";
            sendHeaders();
            server.send(200, "image/webp", "0");
        }

         // POST handle methods
        void Webservice::handleSaveData() {
            logger << "/savedata is called. args: " << (String)server.args();
            String postBody = server.arg("data");
            database->jsonToDatabase(postBody);
            sendHeaders();
            server.send(200, "application/json", getData());
            
            delay(500);
            ESP.restart();
        }

        void Webservice::handleReset() {
            logger << "/reset is called";
            String resetData = "{\"name\":\"" + (String)BOARD_NAME + "\"}";
            database->jsonToDatabase(resetData);
            server.send(200, "text/html", "Board has been reset.");
        }

        void Webservice::handleUpdate() {
            logger << "/update is called";
            server.send(200, "text/html", data_update_html);
        }

        void Webservice::handleUpgradeFn() {
            logger << "/upgrade (fn) is called";
            server.sendHeader("Connection", "close");
            server.send(200, "text/plain", (Update.hasError()) ? "FAIL" : "OK");
            ESP.restart();
        }

        void Webservice::handleUpgradeUFn() {
            //logger << "/upgrade (ufn) is called");
            HTTPUpload& upload = server.upload();            
            if (upload.status == UPLOAD_FILE_START) {
                logger << "Upload started. Upload filename: " << upload.filename.c_str();
                if (!Update.begin()) { //start with max available size
                    logger << "Error in update: " << (String)Update.getError() << " -> " << Update.errorString() << " Upload status: " << (String)upload.status;
                }
            } else if (upload.status == UPLOAD_FILE_WRITE) {
                if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
                    logger << "Error in update: " << (String)Update.getError() << " -> " << Update.errorString() << " Upload status: " << (String)upload.status;
                }
            } else if (upload.status == UPLOAD_FILE_END) {
                if (Update.end(true)) { //true to set the size to the current progress                    
                    logger << "Update Success. Upload size: " << (String)upload.totalSize << " Rebooting... ";
                    // Save the uploaded filename
                    // It contains the version
                    this -> database -> updateProperty(DB_VERSION, upload.filename.c_str(), true);
                } else {
                    logger << "Error in update: " << Update.getError() + " -> " << Update.errorString() << " Upload status: " << (String)upload.status;
                }
                
            } else {                
                logger << "Update Failed Unexpectedly (likely broken connection): " << (String)upload.status;
            }
        }

        // 404
        void Webservice::handleNotFound() {
            logger << "Not found URL is called";
            if (this -> networkConnected) {
                sendHeaders();
                server.send(404, "text/plain", "404: Not found"); // Send HTTP status 404 (Not Found) when there's no handler for the URI in the request
            } else {
                captivePortal();
            }
        }

        void Webservice::captivePortal() {
            logger << "Request redirected to captive portal";
            server.sendHeader("Location", String("http://") + String(AP_IP_STRING), true);
            server.send(302, "text/plain", "");   // Empty content inhibits Content-length header so we have to close the socket ourselves.
            server.client().stop(); // Stop is needed because we sent no content length
        }
