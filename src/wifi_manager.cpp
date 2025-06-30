
#include <functional>
#include "wifi_manager.h"

WifiManager::WifiManager(String ssid, String pass) {
    #ifdef ARDUINO_ARCH_ESP32
    WiFi.onEvent(std::bind(&WifiManager::onEvent, this, std::placeholders::_1));
    #endif
    this->ssid = ssid;
    this->pass = pass;
}

#ifdef ARDUINO_ARCH_ESP32
void WifiManager::onEvent(WiFiEvent_t event) {
    // Serial.printf("[WiFi-event] event: %d\n", event);
    switch (event) {
        case ARDUINO_EVENT_WIFI_STA_GOT_IP:
            this->_status = WifiStatus::CONNECTED;
            this->connected = true;
            this->ipAddress = WiFi.localIP().toString();
            if (this->logger) {
                this->logger->println("WIFI CONNECTED; IP: " + this->ipAddress + "; RSSI: " + String(WiFi.RSSI()));
            }
            if (this->onConnectedCallback) {
                this->onConnectedCallback(this->ipAddress);
            }
            break;
        case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
            this->_status = WifiStatus::DISCONNECTED;
            this->connected = false;
            this->ipAddress = "";
            if (this->logger) {
                this->logger->println("WIFI DISCONNECTED; Disconnected from WiFi network");
            }
            if (this->onDisconnectedCallback) {
                this->onDisconnectedCallback();
            }

            if (this->autoReconnect) {
                this->begin();
            }
            break;
        case ARDUINO_EVENT_WIFI_STA_CONNECTED:
            this->_status = WifiStatus::CONNECTING;
            break;
        case ARDUINO_EVENT_WIFI_STA_LOST_IP: 
            this->connected = false;
            this->ipAddress = "";
            break;
        default:
            break;
    }
}
#endif

void WifiManager::begin() {
    if (this->connected) {
        return;
    }

    if (this->logger) {
        this->logger->println("WIFI CONNECTING; To network: " + this->ssid);
    }

    #ifndef ARDUINO_ARCH_ESP32
    if (WiFi.status() == WL_NO_MODULE) {
        if (this->logger) {
            this->logger->println("WiFi module not avaialble - cannot connect to WiFi!");
        }
        this->_status = WifiStatus::ERROR;
        return;
    }

    int connect_status = WiFi.begin(this->ssid.c_str(), this->pass.c_str());
    unsigned long start_time = millis();
    while (connect_status != WL_CONNECTED && millis() - start_time < 10000) {   // Max 10s to connect
        delay(500);
        connect_status = WiFi.status();
        if (connect_status == WL_CONNECT_FAILED) {
            if (this->logger) {
                this->logger->println("Failed to connect to WiFi network: " + this->ssid);
            }
            this->_status = WifiStatus::ERROR;
            return;
        }
    }

    if (connect_status != WL_CONNECTED) {
        if (this->logger) {
            this->logger->println("Failed to connect to WiFi network: " + this->ssid);
        }
        this->_status = WifiStatus::ERROR;
        return;
    }

    this->_status = WifiStatus::CONNECTED;
    this->connected = true;
    this->ipAddress = WiFi.localIP().toString();
    if (this->logger) {
        this->logger->println("WIFI CONNECTED; IP: " + this->ipAddress + "; RSSI: " + String(WiFi.RSSI()));
    }
    if (this->onConnectedCallback) {
        this->onConnectedCallback(this->ipAddress);
    }
    #else
        WiFi.begin(this->ssid, this->pass);
        this->_status = WifiStatus::CONNECTING;
    #endif
}

void WifiManager::disconnect() {
    WiFi.disconnect();
    this->_status = WifiStatus::DISCONNECTING;
}

bool WifiManager::isConnected() {
    return this->connected;
}

void WifiManager::onConnected(std::function<void(String)> callback) {
    this->onConnectedCallback = callback;
}

void WifiManager::onDisconnected(std::function<void()> callback) {
    this->onDisconnectedCallback = callback;
}

void WifiManager::setAutoReconnect(bool autoReconnect) {
    this->autoReconnect = autoReconnect;
}

bool WifiManager::isAutoReconnect() {
    return this->autoReconnect;
}

WifiStatus WifiManager::status() {
    return this->_status;
}

String WifiManager::address() {
    return this->ipAddress;
}

long WifiManager::strength() {
    return WiFi.RSSI();
}

void WifiManager::setLogger(Print& logger) {
    this->logger = &logger;
}