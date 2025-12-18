#ifndef ESPNowAdhoc_H
#define ESPNowAdhoc_H

#include <Arduino.h>
#include <WiFi.h>
#include <vector>
#include <string>
#include "ESP32_NOW.h"

// デフォルト設定
#ifndef ESPNOW_WIFI_CHANNEL
#define ESPNOW_WIFI_CHANNEL 4
#endif

#ifndef ADV_GROUP_ID
#define ADV_GROUP_ID "906b868f-7e9b-4c21-b587-70c8d5fadfee"
#endif

#ifndef GROUP_ID
#define GROUP_ID "73f8e3bb-aab2-4808-8efe-c061c88e48c2"
#endif

#ifndef HEARTBEAT_TIMEOUT
#define HEARTBEAT_TIMEOUT 5000
#endif

#ifndef HEARTBEAT_INTERVAL
#define HEARTBEAT_INTERVAL 1000
#endif

#ifndef BROADCAST_INTERVAL
#define BROADCAST_INTERVAL 1000
#endif

#ifndef STATUS_DISPLAY_INTERVAL
#define STATUS_DISPLAY_INTERVAL 5000
#endif

#ifndef ESPNOW_DATA_SIZE
#define ESPNOW_DATA_SIZE 1000  // Set to 1000 bytes
#endif

// コマンド定義
#define CMD_REGISTER 1
#define CMD_HEARTBEAT 2
#define CMD_DATA 11

// メッセージ構造体
typedef struct __attribute__((packed)) {
    char group_id[37];
    bool role;
    uint8_t channel;
    bool security;  // セキュリティモードかどうか
    uint8_t cmd;
    char data[ESPNOW_DATA_SIZE];
} espnow_message_t;

// 前方宣言
class ESPNowAdhoc;

class ESPNowAdhocPeer : public ESP_NOW_Peer {
public:
    ESPNowAdhocPeer(const uint8_t *mac_addr, uint8_t channel, wifi_interface_t iface, 
                   ESPNowAdhoc* parent, const uint8_t *lmk = nullptr);
    ~ESPNowAdhocPeer();
    
    bool begin();
    bool removePeer();
    bool sendData(const uint8_t *data, size_t len);
    
    void onSent(bool success) override;
    void onReceive(const uint8_t *data, size_t len, bool broadcast) override;
    
    unsigned long lastGetMs;
    bool isServer;
    bool isSecure;
    
    void setParent(ESPNowAdhoc* parent) { _parent = parent; }
    
private:
    void processReceivedMessage(const uint8_t *data, size_t len, bool broadcast);
    ESPNowAdhoc* _parent;
};

class ESPNowAdhoc {
public:
    ESPNowAdhoc();
    ~ESPNowAdhoc();
    
    bool begin(bool isServerRole, bool useSecurity, const char* pmk = nullptr, const char* lmk = nullptr);
    void update();
    void setDebug(bool enable);
    
    void setBroadcastInterval(unsigned long interval);
    void setHeartbeatInterval(unsigned long interval);
    void setHeartbeatTimeout(unsigned long timeout);
    void setStatusDisplayInterval(unsigned long interval);
    
    int getServerPeerCount() const;
    int getClientPeerCount() const;
    int getTotalPeerCount() const;
    
    bool sendToAll(const uint8_t *data, size_t len);
    bool sendToServer(const uint8_t *data, size_t len);
    bool sendToClients(const uint8_t *data, size_t len);
    
    void setGroupID(const char* advGroupID, const char* groupID);
    void setChannel(uint8_t channel);
    
    // コールバック設定用
    typedef void (*DataCallback)(const uint8_t* mac, const espnow_message_t* msg, bool broadcast);
    void setDataCallback(DataCallback callback);
    
    typedef void (*PeerEventCallback)(const uint8_t* mac, bool isServer, bool connected);
    void setPeerEventCallback(PeerEventCallback callback);
    
    // ピアクラスからアクセスするためのゲッター
    bool isServerMode() const { return _isServer; }
    bool debugEnabled() const { return _debugEnabled; }
    DataCallback getDataCallback() const { return _dataCallback; }
    const char* getGroupID() const { return _groupID; }
    void displayStatus();
    
    // 新しいメソッド
    size_t getMaxDataSize() const { return ESPNOW_DATA_SIZE; }
    size_t getMessageSize() const { return sizeof(espnow_message_t); }
    size_t getESPNOWMaxPayload() const { return ESP_NOW.getMaxDataLen(); }

private:
    void setupWiFi();
    bool setupESPNow();
    void sendBroadcastAdvertisement();
    void sendHeartbeats();
    void checkPeerTimeouts();
    
    
    static void registrationCallback(const esp_now_recv_info_t *info, const uint8_t *data, int len, void *arg);
    void processRegistration(const esp_now_recv_info_t *info, const uint8_t *data);
    void addPeer(const uint8_t* mac, bool peerIsServer, bool peerIsSecure);
    
    bool _isServer;
    bool _useSecurity;
    bool _debugEnabled;
    
    uint8_t _wifiChannel;
    char _advGroupID[37];
    char _groupID[37];
    
    const uint8_t* _pmk;
    const uint8_t* _lmk;
    
    unsigned long _lastBroadcastTime;
    unsigned long _lastHeartbeatTime;
    unsigned long _lastStatusDisplayTime;
    
    unsigned long _broadcastInterval;
    unsigned long _heartbeatInterval;
    unsigned long _heartbeatTimeout;
    unsigned long _statusDisplayInterval;
    
    std::vector<ESPNowAdhocPeer*> _peers;
    ESPNowAdhocPeer* _broadcastPeer;
    
    DataCallback _dataCallback;
    PeerEventCallback _peerEventCallback;
    
    char _pmkString[33];
    char _lmkString[33];
    
    // フレンドクラスとしてESPNowAdhocPeerを宣言
    friend class ESPNowAdhocPeer;
};

#endif