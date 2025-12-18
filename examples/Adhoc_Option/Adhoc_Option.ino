// WIFI channel setting
#define ESPNOW_WIFI_CHANNEL 4

// Broadcast and unicast group UUID (Required)
#define ADV_GROUP_ID "906b868f-7e9b-4c21-b587-70c8d5fadfee"//For security reasons, please change the UUID to a new (random) one.
#define GROUP_ID "73f8e3bb-aab2-4808-8efe-c061c88e48c2"//For security reasons, please change the UUID to a new (random) one.

// Interval settings
#define BROADCAST_INTERVAL 1000       // Broadcast interval (ms)
#define HEARTBEAT_INTERVAL 1000       // Heartbeat send interval (ms)
#define HEARTBEAT_TIMEOUT 5000        // Heartbeat timeout (ms)
#define ESPNOW_DATA_SIZE 1000        // This is the internal allocation size in the library. Make sure it does not exceed the actual ESP-NOW transmission limit (device-dependent, typically ~250 or ~1470 bytes(ESP_NOW V2)). Large data should be split and sent in parts.
//Important!! Please add the include libraries after ESPNOW_DATA_SIZE statement.

// Role setting (true: Server, false: Client) (Required)
#define ROLE true

// Security mode setting
#define SECURITY true // (Required)
#define PMK_STRING "hogehoge54321" // Public key (16 characters or less) (Required if security mode is enabled)
#define LMK_STRING "hogehoge12345" // Private key (16 characters or less) (Required if security mode is enabled)

// Customize transmission data size
#define ESPNOW_DATA_SIZE 1000  // Set to 1000 bytes

// Debug enable/disable
#define ESPNOW_DEBUG false

// Library includes (Required)
#include <Arduino.h>
#include "ESP_NowAdhoc.h"


// ==================== Global Variables ====================
ESP_NowAdhoc espnow; // Start ESP_NowAdhoc (Required)
unsigned long lastDataSendTime = 0;
unsigned long lastStatusSendTime = 0;
int sendCounter = 0;

// ==================== Callback Functions ====================

// Callback triggered when data is received from a registered peer (Required for receiving)
void dataCallback(const uint8_t* mac, const espnow_message_t* msg, bool broadcast) {
  Serial.printf("[GET] Data received from %02X:%02X:%02X:%02X:%02X:%02X",
                mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  Serial.printf("  Command: %d", msg->cmd);
  Serial.printf("  Data field size: %d bytes\n", ESPNOW_DATA_SIZE);
  Serial.printf("  Data: %s\n", msg->data);

  // Add msg->cmd, msg->data processing logic below
}

// Callback for notifications when peers join or leave
void peerEventCallback(const uint8_t* mac, bool isServer, bool connected) {
  if (connected) {
    Serial.printf("[STATUS] Peer connected: %02X:%02X:%02X:%02X:%02X:%02X (%s)\n",
                  mac[0], mac[1], mac[2], mac[3], mac[4], mac[5],
                  isServer ? "SERVER" : "CLIENT");
  } else {
    Serial.printf("[STATUS] Peer disconnected: %02X:%02X:%02X:%02X:%02X:%02X\n",
                  mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  }
}

void setup() {
  Serial.begin(115200);
  delay(2000);

  Serial.println("ESP-NOW Adhoc Network Sample");

  // Initialize ESP-NOW (Required)
  if (SECURITY) {
    espnow.begin(ROLE, true, PMK_STRING, LMK_STRING);
  } else {
    espnow.begin(ROLE, false);
  }

  // Interval settings
  espnow.setBroadcastInterval(BROADCAST_INTERVAL);
  espnow.setHeartbeatInterval(HEARTBEAT_INTERVAL);
  espnow.setHeartbeatTimeout(HEARTBEAT_TIMEOUT);
  espnow.setStatusDisplayInterval(STATUS_DISPLAY_INTERVAL);

  // Set callbacks
  espnow.setDataCallback(dataCallback); // (Required for receiving)
  espnow.setPeerEventCallback(peerEventCallback);

  // Set Group ID (Required)
  espnow.setGroupID(ADV_GROUP_ID, GROUP_ID);

  // Set channel
  espnow.setChannel(ESPNOW_WIFI_CHANNEL);

  // Debug setting
  espnow.setDebug(ESPNOW_DEBUG);

  Serial.println("[SETUP] Setup complete");
  Serial.printf("[SETUP] Heartbeat Interval: %d ms\n", HEARTBEAT_INTERVAL);
  Serial.printf("[SETUP] Heartbeat Timeout: %d ms\n", HEARTBEAT_TIMEOUT);

  // Display message size information
  Serial.printf("[SETUP] Total Payload Size: %d bytes\n", sizeof(espnow_message_t));
  Serial.printf("[SETUP] ESP-NOW Max Payload: %d bytes\n", espnow.getESPNOWMaxPayload());
}

// Example: Send data to specific role
void sendDataToRole(bool targetIsServer, const char* message) {
  espnow_message_t msg;
  memset(&msg, 0, sizeof(msg));

  strncpy(msg.group_id, GROUP_ID, sizeof(msg.group_id));
  msg.role = ROLE;
  msg.channel = ESPNOW_WIFI_CHANNEL;
  msg.security = SECURITY;
  msg.cmd = CMD_DATA;

  strncpy(msg.data, message, sizeof(msg.data));

  bool success;
  if (targetIsServer) {
    success = espnow.sendToServer((uint8_t*)&msg, sizeof(msg));
    Serial.printf("[SEND SERVER] To servers: %s\n", message);
  } else {
    success = espnow.sendToClients((uint8_t*)&msg, sizeof(msg));
    Serial.printf("[SEND CLIENT] To clients: %s\n", message);
  }

  if (!success) {
    Serial.println("  No peers of target role connected");
  }
}

// Example: Send data to all roles
void sendDataToAll(const char* message) {
  espnow_message_t msg;
  memset(&msg, 0, sizeof(msg));

  strncpy(msg.group_id, GROUP_ID, sizeof(msg.group_id));
  msg.role = ROLE;
  msg.channel = ESPNOW_WIFI_CHANNEL;
  msg.security = SECURITY;
  msg.cmd = CMD_DATA;

  strncpy(msg.data, message, sizeof(msg.data));

  bool success;
  success = espnow.sendToAll((uint8_t*)&msg, sizeof(msg));
  Serial.printf("[SEND ALL] To servers: %s\n", msg.data);

  if (!success) {
    Serial.println("  No peers of target role connected");
  }
}

void loop() {
  unsigned long currentTime = millis();
  // Update processing in the main loop
  espnow.update(); // (Required)

  // Example of periodic data transmission
  if (currentTime - lastDataSendTime >= 3000) {
    lastDataSendTime = currentTime;

    // Example: Send only to specific role
    if (ROLE) {  // If server
      sendDataToRole(false, "SERVER_MESSAGE: Hello clients!");
    }
    if (!ROLE) {  // If client
      sendDataToRole(true, "CLIENT_REQUEST: Hello Server!");
    }
    // Send data to all roles
    sendDataToAll("The Power of Small BeginningsGreat journeys often start with a single, hesitant step. A towering oak tree begins as a vulnerable acorn, and a vast, sweeping novel starts with one blank page. In our quest for monumental achievements, we frequently overlook the profound power of small beginnings. These initial moments, seemingly insignificant, contain the DNA of everything that follows. They are the quiet laboratories where courage is tested, foundations are laid, and the trajectory of a future is quietly determined.Consider the process of learning. Mastery of a complex skill—be it a language, an instrument, or a scientific discipline—does not erupt fully formed. It is painstakingly built from the basic alphabet of its domain: the first clumsy chords, the simple vocabulary words, the fundamental equations. Each small act of practice is a brick in an invisible wall. The beginner, focused only on the immediate struggle, may not see the emerging structure. Yet, without the the fundamental equations. Each small act of practice is a brick in an invisible wall. The beginner, focused only on the immediate struggle, may not see the emerging structure. Yet, without OK!");
  }


  // Display status
  if (currentTime - lastStatusSendTime >= 5000) {
    lastStatusSendTime = currentTime;
    espnow.displayStatus();
  }

  // Add any additional processing here if needed

  delay(10);  // Small delay to reduce CPU load
}