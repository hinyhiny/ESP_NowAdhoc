#include "ESP_NowAdhoc.h"
#include <WiFi.h>

// ==================== ESP_NowAdhocPeer クラス ====================

ESP_NowAdhocPeer::ESP_NowAdhocPeer(const uint8_t *mac_addr, uint8_t channel, wifi_interface_t iface, 
                               ESP_NowAdhoc* parent, const uint8_t *lmk)
    : ESP_NOW_Peer(mac_addr, channel, iface, lmk), _parent(parent) {
    lastGetMs = millis();
    isServer = false;
    isSecure = (lmk != nullptr);
}

ESP_NowAdhocPeer::~ESP_NowAdhocPeer() {
    remove();
}

bool ESP_NowAdhocPeer::begin() {
    return add();
}

bool ESP_NowAdhocPeer::removePeer() {
    return remove();
}

bool ESP_NowAdhocPeer::sendData(const uint8_t *data, size_t len) {
    return send(data, len);
}

void ESP_NowAdhocPeer::onSent(bool success) {
    if (_parent && _parent->debugEnabled()) {
        Serial.print("[ESP_NowAdhocPeer] Send ");
        Serial.println(success ? "Success" : "Failed");
    }
}

void ESP_NowAdhocPeer::onReceive(const uint8_t *data, size_t len, bool broadcast) {
    processReceivedMessage(data, len, broadcast);
}

void ESP_NowAdhocPeer::processReceivedMessage(const uint8_t *data, size_t len, bool broadcast) {
    if (!_parent || len < sizeof(espnow_message_t)) {
        return;
    }
    
    espnow_message_t *msg = (espnow_message_t *)data;
    
    // グループIDチェック
    if (strcmp(msg->group_id, _parent->getGroupID()) != 0) {
        return;
    }
    
    // セキュリティモードチェック
    if (msg->security != isSecure) {
        return;
    }
    
    // サーバーロールチェック（クライアントはサーバーからのみ受信）
   // if (!_parent->isServerMode() && !msg->role) {
   //     return;
   // }

    
    lastGetMs = millis();
    
    switch (msg->cmd) {
        case CMD_HEARTBEAT:
            if (_parent && _parent->debugEnabled()) {
                const uint8_t *mac = addr();
                Serial.printf("[GET] [%s] (%02X:%02X:%02X:%02X:%02X:%02X) CMD_HEARTBEAT\n",
                    broadcast ? "Broadcast" : "Unicast",
                    mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
            }
            break;
            
        case CMD_DATA:
            if (_parent && _parent->getDataCallback()) {
                _parent->getDataCallback()(addr(), msg, broadcast);
            }
            break;
            
        default:
            // その他のコマンド
            if (_parent && _parent->getDataCallback()) {
                _parent->getDataCallback()(addr(), msg, broadcast);
            }
            break;
    }
}

// ==================== ESP_NowAdhoc クラス ====================

ESP_NowAdhoc::ESP_NowAdhoc() {
    _isServer = false;
    _useSecurity = false;
    _debugEnabled = true;
    _wifiChannel = ESPNOW_WIFI_CHANNEL;
    
    strncpy(_advGroupID, ADV_GROUP_ID, sizeof(_advGroupID));
    strncpy(_groupID, GROUP_ID, sizeof(_groupID));
    
    _pmk = nullptr;
    _lmk = nullptr;
    
    _lastBroadcastTime = 0;
    _lastHeartbeatTime = 0;
    _lastStatusDisplayTime = 0;
    
    _broadcastInterval = BROADCAST_INTERVAL;
    _heartbeatInterval = HEARTBEAT_INTERVAL;
    _heartbeatTimeout = HEARTBEAT_TIMEOUT;
    _statusDisplayInterval = STATUS_DISPLAY_INTERVAL;
    
    _broadcastPeer = nullptr;
    _dataCallback = nullptr;
    _peerEventCallback = nullptr;
    
    memset(_pmkString, 0, sizeof(_pmkString));
    memset(_lmkString, 0, sizeof(_lmkString));
}

ESP_NowAdhoc::~ESP_NowAdhoc() {
    if (_broadcastPeer) {
        delete _broadcastPeer;
    }
    
    for (auto peer : _peers) {
        if (peer) {
            delete peer;
        }
    }
    _peers.clear();
    
    ESP_NOW.end();
}

bool ESP_NowAdhoc::begin(bool isServerRole, bool useSecurity, const char* pmk, const char* lmk) {
    _isServer = isServerRole;
    _useSecurity = useSecurity;
    
    if (useSecurity) {
        if (pmk && lmk) {
            strncpy(_pmkString, pmk, sizeof(_pmkString) - 1);
            strncpy(_lmkString, lmk, sizeof(_lmkString) - 1);
            _pmk = (const uint8_t*)_pmkString;
            _lmk = (const uint8_t*)_lmkString;
        } else {
            Serial.println("[ESP_NowAdhoc] Error: Security mode requires PMK and LMK");
            return false;
        }
    }
    
    setupWiFi();
    
    if (!setupESPNow()) {
        return false;
    }
    
    // ブロードキャストピアの設定
    _broadcastPeer = new ESP_NowAdhocPeer(ESP_NOW.BROADCAST_ADDR, _wifiChannel, WIFI_IF_STA, this, nullptr);
    if (!_broadcastPeer->begin()) {
        Serial.println("[ESP_NowAdhoc] Failed to create broadcast peer");
        return false;
    }
    
    // 登録コールバックの設定
    ESP_NOW.onNewPeer(registrationCallback, this);
    
    if (_debugEnabled) {
        Serial.println("[ESP_NowAdhoc] Initialization complete");
        Serial.printf("  Role: %s\n", _isServer ? "SERVER" : "CLIENT");
        Serial.printf("  Security: %s\n", _useSecurity ? "ENABLED" : "DISABLED");
        Serial.printf("  Channel: %d\n", _wifiChannel);
        Serial.printf("  Adv Group: %s\n", _advGroupID);
        Serial.printf("  Group: %s\n", _groupID);
        Serial.printf("  Heartbeat Interval: %lu ms\n", _heartbeatInterval);
        Serial.printf("  Heartbeat Timeout: %lu ms\n", _heartbeatTimeout);
    }
    
    return true;
}

void ESP_NowAdhoc::setupWiFi() {
    WiFi.mode(WIFI_STA);
    WiFi.setChannel(_wifiChannel);
    WiFi.setTxPower(WIFI_POWER_17dBm);
    
    while (!WiFi.STA.started()) {
        delay(100);
    }
    
    if (_debugEnabled) {
        Serial.println("[ESP_NowAdhoc] WiFi initialized");
        Serial.printf("  MAC: %s\n", WiFi.macAddress().c_str());
        Serial.printf("  Channel: %d\n", _wifiChannel);
    }
}

bool ESP_NowAdhoc::setupESPNow() {
    bool success;
    
    if (_useSecurity) {
        success = ESP_NOW.begin(_pmk);
    } else {
        success = ESP_NOW.begin();
    }
    
    if (!success) {
        Serial.println("[ESP_NowAdhoc] Failed to initialize ESP-NOW");
        return false;
    }
    
    if (_debugEnabled) {
        Serial.println("[ESP_NowAdhoc] ESP-NOW initialized");
        Serial.printf("  Version: %d\n", ESP_NOW.getVersion());
        Serial.printf("  Max Data Length: %d\n", ESP_NOW.getMaxDataLen());
    }
    
    return true;
}

void ESP_NowAdhoc::update() {
    unsigned long currentTime = millis();
    
    // ブロードキャスト広告の送信
    if (currentTime - _lastBroadcastTime >= _broadcastInterval) {
        sendBroadcastAdvertisement();
        _lastBroadcastTime = currentTime;
    }
    
    // ハートビートの送信
    if (currentTime - _lastHeartbeatTime >= _heartbeatInterval) {
        sendHeartbeats();
        _lastHeartbeatTime = currentTime;
    }
    
    // ピアタイムアウトチェック
    checkPeerTimeouts();
    
    // ステータス表示
    if (_debugEnabled && currentTime - _lastStatusDisplayTime >= _statusDisplayInterval) {
        displayStatus();
        _lastStatusDisplayTime = currentTime;
    }
}

void ESP_NowAdhoc::sendBroadcastAdvertisement() {
    if (!_broadcastPeer) {
        return;
    }
    
    espnow_message_t msg;
    memset(&msg, 0, sizeof(msg));
    
    strncpy(msg.group_id, _advGroupID, sizeof(msg.group_id));
    msg.role = _isServer;
    msg.channel = _wifiChannel;
    msg.security = _useSecurity;
    msg.cmd = CMD_REGISTER;
    
    String dataStr = String("Register request from ") + 
                    (_isServer ? "SERVER" : "CLIENT") + 
                    " MAC: " + WiFi.macAddress();
    strncpy(msg.data, dataStr.c_str(), sizeof(msg.data));
    
    if (_broadcastPeer->sendData((uint8_t*)&msg, sizeof(msg))) {
        if (_debugEnabled) {
            Serial.println("[ESP_NowAdhoc] Broadcast advertisement sent");
        }
    }
}

void ESP_NowAdhoc::sendHeartbeats() {
    for (size_t i = 0; i < _peers.size(); i++) {
        if (_peers[i]) {
            espnow_message_t msg;
            memset(&msg, 0, sizeof(msg));
            
            strncpy(msg.group_id, _groupID, sizeof(msg.group_id));
            msg.role = _isServer;
            msg.channel = _wifiChannel;
            msg.security = _useSecurity;
            msg.cmd = CMD_HEARTBEAT;
            
            String dataStr = String("Heartbeat from ") + 
                            (_isServer ? "SERVER" : "CLIENT") + 
                            " to " + 
                            (_peers[i]->isServer ? "SERVER" : "CLIENT");
            strncpy(msg.data, dataStr.c_str(), sizeof(msg.data));
            
            _peers[i]->sendData((uint8_t*)&msg, sizeof(msg));
            
            if (_debugEnabled) {
                const uint8_t* mac = _peers[i]->addr();
                Serial.printf("[ESP_NowAdhoc] Heartbeat sent to %02X:%02X:%02X:%02X:%02X:%02X\n",
                    mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
            }
        }
    }
}

void ESP_NowAdhoc::checkPeerTimeouts() {
    for (auto it = _peers.begin(); it != _peers.end();) {
        ESP_NowAdhocPeer* peer = *it;
        
        if (millis() - peer->lastGetMs > _heartbeatTimeout) {
            if (_debugEnabled) {
                const uint8_t* mac = peer->addr();
                Serial.printf("[ESP_NowAdhoc] Peer timeout: %02X:%02X:%02X:%02X:%02X:%02X\n",
                    mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
            }
            
            if (_peerEventCallback) {
                _peerEventCallback(peer->addr(), peer->isServer, false);
            }
            
            peer->removePeer();
            delete peer;
            it = _peers.erase(it);
        } else {
            ++it;
        }
    }
}

void ESP_NowAdhoc::displayStatus() {
    Serial.println("\n[ESP_NowAdhoc] ===== Status =====");
    Serial.printf("  Role: %s\n", _isServer ? "SERVER" : "CLIENT");
    Serial.printf("  Security: %s\n", _useSecurity ? "ENABLED" : "DISABLED");
    Serial.printf("  Channel: %d\n", _wifiChannel);
    Serial.printf("  Heartbeat Interval: %lu ms\n", _heartbeatInterval);
    Serial.printf("  Heartbeat Timeout: %lu ms\n", _heartbeatTimeout);
    Serial.printf("  Total Peers: %d\n", getTotalPeerCount());
    Serial.printf("  Server Peers: %d\n", getServerPeerCount());
    Serial.printf("  Client Peers: %d\n", getClientPeerCount());
    
    // 各ピアの状態を表示
    for (size_t i = 0; i < _peers.size(); i++) {
        const uint8_t* mac = _peers[i]->addr();
        unsigned long lastSeen = millis() - _peers[i]->lastGetMs;
        Serial.printf("  Peer %d: %02X:%02X:%02X:%02X:%02X:%02X [%s] Last seen: %lu ms ago\n",
            i, mac[0], mac[1], mac[2], mac[3], mac[4], mac[5],
            _peers[i]->isServer ? "SERVER" : "CLIENT", lastSeen);
    }
    Serial.println("=============================\n");
}

void ESP_NowAdhoc::registrationCallback(const esp_now_recv_info_t *info, const uint8_t *data, int len, void *arg) {
    ESP_NowAdhoc* instance = static_cast<ESP_NowAdhoc*>(arg);
    if (instance && len >= sizeof(espnow_message_t)) {
        instance->processRegistration(info, data);
    }
}

void ESP_NowAdhoc::processRegistration(const esp_now_recv_info_t *info, const uint8_t *data) {
    espnow_message_t *msg = (espnow_message_t *)data;
    
    // 広告グループIDチェック
    if (strcmp(msg->group_id, _advGroupID) != 0) {
        return;
    }
    
    // セキュリティモードチェック（異なるモードとは接続しない）
    if (msg->security != _useSecurity) {
        if (_debugEnabled) {
            Serial.println("[ESP_NowAdhoc] Security mode mismatch, ignoring registration");
        }
        return;
    }
    
    // 既存ピアチェック
    for (auto peer : _peers) {
        const uint8_t* peerMac = peer->addr();
        if (memcmp(peerMac, info->src_addr, 6) == 0) {
            // 既に登録済み
            return;
        }
    }
    
    // ロールによる登録条件チェック
    if (_isServer) {
        // サーバーはすべてのロールを受け入れる
        addPeer(info->src_addr, msg->role, msg->security);
    } else {
        // クライアントはサーバーのみ受け入れる
        if (msg->role) { // 相手がサーバー
            addPeer(info->src_addr, msg->role, msg->security);
        }
    }
}

void ESP_NowAdhoc::addPeer(const uint8_t* mac, bool peerIsServer, bool peerIsSecure) {
    ESP_NowAdhocPeer* newPeer;
    
    if (_useSecurity) {
        newPeer = new ESP_NowAdhocPeer(mac, _wifiChannel, WIFI_IF_STA, this, _lmk);
    } else {
        newPeer = new ESP_NowAdhocPeer(mac, _wifiChannel, WIFI_IF_STA, this, nullptr);
    }
    
    if (newPeer->begin()) {
        newPeer->isServer = peerIsServer;
        newPeer->isSecure = peerIsSecure;
        newPeer->lastGetMs = millis();
        
        _peers.push_back(newPeer);
        
        if (_debugEnabled) {
            Serial.printf("[ESP_NowAdhoc] New peer registered: %02X:%02X:%02X:%02X:%02X:%02X (%s)\n",
                mac[0], mac[1], mac[2], mac[3], mac[4], mac[5],
                peerIsServer ? "SERVER" : "CLIENT");
        }
        
        if (_peerEventCallback) {
            _peerEventCallback(mac, peerIsServer, true);
        }
    } else {
        delete newPeer;
        Serial.println("[ESP_NowAdhoc] Failed to add peer");
    }
}

// ==================== 公開メソッド ====================

void ESP_NowAdhoc::setDebug(bool enable) {
    _debugEnabled = enable;
}

void ESP_NowAdhoc::setBroadcastInterval(unsigned long interval) {
    _broadcastInterval = interval;
}

void ESP_NowAdhoc::setHeartbeatInterval(unsigned long interval) {
    _heartbeatInterval = interval;
}

void ESP_NowAdhoc::setHeartbeatTimeout(unsigned long timeout) {
    _heartbeatTimeout = timeout;
}

void ESP_NowAdhoc::setStatusDisplayInterval(unsigned long interval) {
    _statusDisplayInterval = interval;
}

int ESP_NowAdhoc::getServerPeerCount() const {
    int count = 0;
    for (auto peer : _peers) {
        if (peer->isServer) {
            count++;
        }
    }
    return count;
}

int ESP_NowAdhoc::getClientPeerCount() const {
    int count = 0;
    for (auto peer : _peers) {
        if (!peer->isServer) {
            count++;
        }
    }
    return count;
}

int ESP_NowAdhoc::getTotalPeerCount() const {
    return _peers.size();
}

bool ESP_NowAdhoc::sendToAll(const uint8_t *data, size_t len) {
    bool allSuccess = true;
    for (auto peer : _peers) {
        if (!peer->sendData(data, len)) {
            allSuccess = false;
        }
    }
    return allSuccess;
}

bool ESP_NowAdhoc::sendToServer(const uint8_t *data, size_t len) {
    bool allSuccess = true;
    for (auto peer : _peers) {
        if (peer->isServer && !peer->sendData(data, len)) {
            allSuccess = false;
        }
    }
    return allSuccess;
}

bool ESP_NowAdhoc::sendToClients(const uint8_t *data, size_t len) {
    bool allSuccess = true;
    for (auto peer : _peers) {
        if (!peer->isServer && !peer->sendData(data, len)) {
            allSuccess = false;
        }
    }
    return allSuccess;
}

void ESP_NowAdhoc::setGroupID(const char* advGroupID, const char* groupID) {
    if (advGroupID) {
        strncpy(_advGroupID, advGroupID, sizeof(_advGroupID));
    }
    if (groupID) {
        strncpy(_groupID, groupID, sizeof(_groupID));
    }
}

void ESP_NowAdhoc::setChannel(uint8_t channel) {
    _wifiChannel = channel;
    WiFi.setChannel(channel);
}

void ESP_NowAdhoc::setDataCallback(DataCallback callback) {
    _dataCallback = callback;
}

void ESP_NowAdhoc::setPeerEventCallback(PeerEventCallback callback) {
    _peerEventCallback = callback;
}