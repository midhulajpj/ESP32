# ESP32_ESP-IDF_CPP
Contains C++ applications for ESP-32 using ESP-IDF Framework

-  Gives an example for a GATT Server with one service and two characteristics.
-  Contains the security feature implementations.
-  Used [gatt_server_service_table](https://github.com/espressif/esp-idf/tree/master/examples/bluetooth/bluedroid/ble/gatt_server_service_table) and [gatt_security_server](https://github.com/espressif/esp-idf/tree/master/examples/bluetooth/bluedroid/ble/gatt_security_server) as reference.

## SRC File Descriptions:
- includes.h : Contains all the required header files.
- Class BLEManager(BLEManager.h and BLEManager.cpp) contains the following:
  - Methods to initialize a BLE GATT Security server.
  - GATT Profile.
  - Methods to send notification after connecting and the notification are enabled.
  main.cpp : As an example, it just initializes the BLE and send the incrimented counter value as a notification to the client.

### Notes
* Used nRF Connect android application as the ble gatt client.

