# BLEcker
**Bluetooth low energy (BLE) tracker for ESP32**

This software is written for ESP32 boards to track BLE devices. It can be used for your smart home, scan BLE devices and send their presence to your smart home hub over MQTT.
This is a ready-to-use program, you don't need to modify the code (add your wifi, mqtt credentials whatever). Settings can be done on a nice web interface.

## What does it exactly
This is a very simple tracker software which creates an MQTT topic for each scanned device mac address (without ":" ) under the base topic and sends the availability as payload.
I personally use it for presence detection of family members. Every family member has a BLE device on her/his keyring and smart home can do automations depends on the members' availability. For example: turn on the alarm system if nobody at home.

The default topic is **/blecker**\
The default payload for available device: **present**\
The default payload for not available device: **not_present**
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

The scan is running in every 2 seconds for 5 seconds and collects the devices into a list. In case of an available device, it sends an MQTT message as available. If the device is gone or can't be scanned after 120 seconds then a "not available" message will be sent out.

The administrator can define an observable device list in a web frontend. These devices will be uploaded to the inside device list. With this mechanism, the "not available" message can be sent out even if the device was not available right after the reboot.

### Explanation
If the device is not available after the system start, "not available" message will be never sent out just in case the BLE device becomes available and will be gone again.

### Status messages
System sends a detailed status message about the BLE device in every minute: **/blecker/[device-mac]/status**\
The payload is a JSON object structure which contains detailed data like **name**, **rssi**, **observed**, etc. for more possibilities.
This function is off by default. It can be changed on a web administration UI.


## Upload to ESP32
1. Using VSCode and PlatformIO
  * download the source and put it into a folder
  * install VSCode (https://code.visualstudio.com/download) and PlatformIO inside (https://platformio.org/install/ide?install=vscode)
  * open the folder of the source code in VSCode
  * connect your ESP32 to your computer using the USB cable
  * on the bottom of the window there should be a PlatformIO tool: Platform IO upload
  * wait while the code is being built and uploaded

2. Use the ESPtool to upload the prebuilt binary
  * download the binary here
  * read how to install and use esptool: https://github.com/espressif/esptool
  * upload the binary: esptool.py --chip esp32 blecker.bin

## First steps
Upload and start the code on ESP32. If there is no configuration yet then it offers an access point. The name of the accesspoint can be found in this file: definitions.h
* connect to this access point with your smartphone and call the IP address **192.168.4.1** for web administration.
You can set your WiFi and MQTT credentials on that administration page. See the details below.

Later you can find the web administration tool on the IP address which was set to the ESP32. Check it in your own router or WiFi manager tool.

## Web configuration
Web configuration UI is available to change some parameters in the system. It can be reached in a browser. Call the IP address of the board. (See the network settings in your router or WiFi manager)
The following settings are available:
* WiFi name: your WiFi network name where the board should connect
* Password: password of your WiFi network
* MQTT server: your MQTT server address
* MQTT port: port of the MQTT connection
* Base topic: you can define a prefix for your messages. Example: /home/presence -> /home/presence/blecker/[device-mac] topic will be used
* Username: MQTT server username
* Password: MQTT server password
* Observed devices: you can define your own devices for more accuracy, see the reason above (explanation). Use the mac address without ":" and separate them with ";" Please do not use space characters
* Detailed report: default off. See the details in section **Status messages**

If you click to the advanced text, you can find more options
* Presence string (available): a custom payload to send an available state
* Presence string (not available) a custom payload to send a not available state


## Update
There are two ways to update your board:
* build and upload a new code like the first time (Upload to ESP32)
* use web OTA. Web administration interface offers you an update mechanism. You can update your board with a new .bin update file. Browse the update file from your PC and press the upload button. Some minutes later the new firmware will run on your ESP32.

## For developers
HTML code in /html folder is built to the source code. It is done by PlatformIO build mechanism. (pre_build.py, pre_build_web.py)
Python removes the trailing spaces and compile into a PROGMEM variable.
To live edit the web UI make a symlink from /html to your webserver folder. If you modify the code then refresh your browser by F5. You should not change the HTML code in a webcontent.h file.

## Example for Home Assistant
Let's say you have a BLE beacon with this device id (mac address): `12:34:56:ab:cd:ef`
### Settings on ESP32
* Upload the code to your ESP32 and let it run.
* Call the ESP32 web interface its IP address (Web frontend should appear)
* Set the credentials of your WiFi and MQTT, let the base topic field empty for now
* Click to the 'advenced' text and set the presence strings to the following -> *home* | *not_home*
* put your BLE device address into the Observed devices input field without ":". In tis case: 123456abcdef
* Press the submit button (device will reboot)

### Settings in Home Assistant
* open your configuration file of  Home Assistant instance to configure the device tracker module. It is usually in the configuration.yaml file
* complete your device tracker configuration with the MQTT presence option. Details: https://www.home-assistant.io/integrations/device_tracker.mqtt/
* reboot/reload the HA
* At the end of the day you should have something like this

```
device_tracker:
     - platform: mqtt
       devices:
         redakker: '/blecker/123456abcdef'

```
* after restar HA you will find among states the presence of your BLE device with this name:  **device_tracker.redakker**

