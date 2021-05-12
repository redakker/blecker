# BLEcker
Bluetooth low enery (BLE) tracker for ESP32

This software is written for ESP32 boards to track BLE devices. It can be used for your smarthome, scan BLE deviceas and send their presence to your smarthome hub over MQTT.
This is a ready to use program, you don't need to modify the code (add your wifi, mqtt credentials whatever). Settings can be done on a nice web interface.

## What is does exactly
This is a very simple tracker software which creates an MQTT topic for each scanned device mac address (without ":" ) under the base topic and send the availability as payload.
The default topic is */blecker*
The default payload for available device: *present*
The default payload for not available device: *not_present*
### Example

Device MAC is: `12:34:56:ab:cd:ef`
Send an MQTT message if available: 
```
topic: /blecker/123456abcdef
payload: present
```

Send an MQTT message if NOT available: 
```
topic: /blecker/123456abcdef
payload: not_present
```



## Upload to ESP32
1. Using VSCode and PlatformIO
  * download the source and put into a folder
  * install VSCode (https://code.visualstudio.com/download) and PlatformIO inside (https://platformio.org/install/ide?install=vscode)
  * open the folder of the source code in VSCode
  * connect your ESP32 to your computer using the USB cable
  * on the bottom of the window there should be a PlatformIO tool: Platform IO upload
  * wait while the code is being built and uploaded

2. Use the ESPtool to upload the prebuild binary
  * download the binary here
  * read how to install and use esptool: https://github.com/espressif/esptool
  * upload the binary: esptool.py --chip esp32 blecker.bin

## First steps

## Web configuration
