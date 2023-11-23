#include "utilities.h"
#include "definitions.h"
#include <BLEScan.h>


esp_chip_info_t chip_info;

void setChipInfo() {
    // Get ESP32 chip information    
    esp_chip_info(&chip_info);
}

// Function to convert chip model enum to string
const char* getChipModelString(esp_chip_model_t model) {
    switch (model) {
        case CHIP_ESP32:
            return "ESP32";
        case CHIP_ESP32S2:
            return "ESP32-S2";
        case CHIP_ESP32S3:
            return "ESP32-S3";
        case CHIP_ESP32C3:
            return "ESP32-C3";
        default:
            return "Unknown";
    }
}

void bluetoothScanner(void *parameters) {
  
  BLEScan *pBLEScan = static_cast<BLEScan*>(parameters);

  for( ;; ) {
    pBLEScan -> start(BT_DEFAULT_SCAN_DURATION_IN_SECONDS, false);
    
    // Tell the task how long to delay for:
    vTaskDelay(2000 / portTICK_PERIOD_MS );    
    
    pBLEScan -> clearResults();   // delete results fromBLEScan buffer to release memory
    vTaskDelay(20 / portTICK_PERIOD_MS ); 
  }
}