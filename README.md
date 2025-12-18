[日本語はこちら](https://github.com/cpei2025/ESPNowAdhoc/blob/main/README-ja.md)

![MIT](https://img.shields.io/badge/License-MIT-yellow.svg)
![MIT](https://img.shields.io/badge/License-MIT-yellow.svg)
![Arduino](https://img.shields.io/badge/Arduino-Compatible-blue.svg)
![ESP32](https://img.shields.io/badge/ESP32-Compatible-green.svg)
![PlatformIO](https://img.shields.io/badge/PlatformIO-Compatible-orange.svg)

# ESPNowAdhoc Library
An advanced ESP-NOW ad-hoc networking library for ESP32. It enables direct communication among multiple ESP32 devices without a Wi‑Fi access point.  
A packaged library to easily and securely build an ad-hoc network environment using ESP-NOW. It packages the necessary features for ad-hoc use: group settings, role settings, security settings, automatic peer discovery via broadcast, and connected-peer status monitoring via heartbeat. Create an ad-hoc network using only ESP32 devices in three steps. This makes radio communication in ESP32 environments—previously expensive and complex—easy and low-cost.

## Features
- Build an ad-hoc network easily without developing with complex ESP_NOW library classes
- UUID-based group management (separates the advertising-group UUID used for connection from the unicast-group UUID to strongly prevent interference with other ESP_NOW networks)
- Configurable roles (server/client set by a single parameter; allows easy construction of various ad-hoc network topologies)
- Bidirectional communication without complex coding
- Automatic multi-device peering without complex coding (up to 20 peers when encryption is disabled, per ESP_NOW specification)
- One-source multi-use: switch between client and server by changing parameters
- Simple encrypted communication configuration
- Function to display connected peer status
- Configurable data payload region (optimize required memory)
- High security (group UUID for unicast is independent, so group UUID is not exposed under encrypted communication)
- Heartbeat-based status monitoring (monitors connected peers via heartbeat; if a connection is lost it will automatically reconnect when the other party recovers)
- Uses the ESP32 built-in Wi‑Fi, so no additional communication module is necessary
- If using an ESP_NOW V2 environment, up to 1470 bytes can be sent (confirm via serial output at startup whether 1470 bytes are supported)

## Use Cases
- Remote control systems
- Low-cost remote control systems requiring bidirectional communication
- Remote control systems controllable from multiple locations that require bidirectional communication
- Autonomous swarm drone/robot management leveraging auto-connection and no Wi‑Fi AP
- Outdoor autonomous sensor networks leveraging auto-connection and no Wi‑Fi AP
- Autonomous disaster relief systems leveraging auto-connection and no Wi‑Fi AP

## ESPNowAdhoc Payload (transmission packet) Specification
- Group UUID (char 37 Bytes)
- Role (Server, Client) (bool 1 byte)
- Wi‑Fi channel (reserved for future automatic channel switching) (int)
- Security mode enabled flag (bool 1 byte)
- Command (register, heartbeat, data) (int)
- Actual data (variable via ESPNOW_DATA_SIZE; must not exceed ESP‑NOW maximum transmission bytes)

## Configurable Parameters (optional)
- Broadcast interval setting
- Heartbeat send interval setting
- Heartbeat timeout setting
- Payload data size setting
- Debug mode to inspect peer registration and heartbeat state

## Supported Devices / Requirements
- Environment where ESP_NOW is available:
  - ESP32-S series: ESP32-S2, ESP32-S3, etc. supported
  - ESP32-C series: ESP32-C3, etc. supported
  - M5Stack series: ESP-NOW available because they include ESP32 microcontrollers
- Because clients do not peer with each other, at least one device must be set to the server role
- If ESP_NOW v2 is available in your environment, larger payload sizes may be supported
- For inquiries about using this library, please use GitHub Issues

## Constants & Default Values
> These can be overridden by preprocessor definitions in the header.

- **ESPNOW_WIFI_CHANNEL**: Default `4`  
- **ADV_GROUP_ID**: Default `"906b868f-7e9b-4c21-b587-70c8d5fadfee"`  
<br>**Note:** For security reasons, change this in your main program
- **GROUP_ID**: Default `"73f8e3bb-aab2-4808-8efe-c061c88e48c2"`  
<br>**Note:** For security reasons, change this in your main program
- **HEARTBEAT_TIMEOUT**: Default `5000` ms  
- **HEARTBEAT_INTERVAL**: Default `1000` ms  
- **BROADCAST_INTERVAL**: Default `1000` ms  
- **ESPNOW_DATA_SIZE**: Default `1000` bytes ※  
<br>**Note:** This is the internal allocation size in the library. Make sure it does not exceed the actual ESP-NOW transmission limit (device-dependent, typically ~250 bytes). Large data should be split and sent in parts.

## Installation

## STEP1 Install the Library

### Arduino IDE
1. Sketch → Include Library → Manage Libraries...
2. Search for "ESPNowAdhoc"
3. Click Install

### Manual Install
1. Download the ZIP from [Releases](https://github.com/cpei2025/ESPNowAdhoc/releases)
2. Arduino IDE: Sketch → Include Library → Add .ZIP Library...
3. Ensure the **ESP32 board package** is already installed

### PlatformIO
Add the following to your platformio.ini:
```
[env:esp32dev]
platform = espressif32
board = esp32dev
framework = arduino
lib_deps = 
    https://github.com/cpei2025/ESPNowAdhoc.git
```

## STEP2 Initial Configuration (Important)
1. Set group UUIDs for broadcast and unicast (required)
```
#define ADV_GROUP_ID "906b868f-7e9b-4c21-b587-70c8d5fadfee"
#define GROUP_ID "73f8e3bb-aab2-4808-8efe-c061c88e48c2"
```
2. Configure whether to use encrypted communication (required). If using security, set the public and private keys:
```
#define SECURITY true // (required)
#define PMK_STRING "hogehoge54321" // Public key, up to 16 chars (required in security mode)
#define LMK_STRING "hogehoge12345" // Private key, up to 16 chars (required in security mode)
```
3. Set the role:
```
#define ROLE true // Role setting (true: Server, false: Client) (required)
```

## STEP3 Coding & Release

1. Create an `ESPNowAdhoc` instance  
2. Initialize with `begin()` (specify Server/Client and whether security is enabled)  
3. Set `setDataCallback()` / `setPeerEventCallback()`  
4. Call `update()` on every iteration inside `loop()`  
5. Call `sendToAll()` / `sendToServer()` / `sendToClients()` as needed

## Troubleshooting & Solutions

### Problem 1: Peers do not connect
1. Devices are using different channels
2. Group IDs do not match
3. Security settings differ

### Problem 2: Data transmission fails
1. Payload data field size (bytes) exceeds ESPNOW_DATA_SIZE
2. Physical distance is too great
3. If you define `#define ESPNOW_DATA_SIZE`, make sure it is defined before `#include "ESPNowAdhoc.h"`
4. Check in debug mode (inside `setup()`):
```
espnow.setDebug(false);
```
Use this to verify whether peers are peered and whether heartbeat signals are being received from the other side.

## Common Usage Patterns (Checklist)
- Pass the correct **role (Server/Client)** to `begin()`  
- When security is enabled, ensure proper **PMK/LMK length and management**  
- Call **`update()` regularly inside `loop()`**  
- Enable debug to inspect connection logs (`setDebug(true)`)  
- Implement **split transmission** for large data  
- If you need group separation, set a different UUID with **`setGroupID()`**

## FAQ
### Q: How many devices can be connected at maximum?
#### A: Depends on security mode:
- Security disabled: up to 19 peers
- Security enabled: up to 5 peers

### Q: What is the communication range?
#### A: Varies by environment:
- Indoor (line of sight): 10–30 m
- Outdoor (open area): 100–200 m  
Obstacles such as walls significantly reduce range.

### Q: What is the data transfer speed?
#### A: Theoretical maximum:
- Point-to-point: about 1 Mbps
- Actual throughput: 500–800 kbps
- Latency: typically 10–50 ms  
Obstacles such as walls significantly reduce performance.

### Q: Is it compatible across different ESP32 models?
#### A: Yes:
Different models that support ESP-NOW can send and receive to each other.

## License & Support
- See the `LICENSE` file in the repository for license details.  
- For bug reports or feature requests, please open an issue on GitHub.

---

## For the API, see the WIKI
[[API Reference]](https://github.com/cpei2025/ESPNowAdhoc/wiki/API%E3%83%AA%E3%83%95%E3%82%A1%E3%83%AC%E3%83%B3%E3%82%B9)