#ifndef HUB_WIFI_MANAGER_H
#define HUB_WIFI_MANAGER_H

#include <Arduino.h>
#include <functional>

#if defined(ARDUINO_ARCH_ESP32)
#include <WiFi.h>
#else
#include <WiFiNINA.h>
#include <utility/wifi_drv.h>
#endif


enum class WifiStatus {
    IDLE,
    CONNECTING,
    CONNECTED,
    DISCONNECTING,
    DISCONNECTED,
    ERROR
};

class WifiManager {
    private:
        std::function<void(String)> onConnectedCallback;
        std::function<void()> onDisconnectedCallback;
        bool connected = false;
        bool autoReconnect = true;
        String ipAddress;
        String ssid;
        String pass;
        WifiStatus _status = WifiStatus::IDLE;
        Print* logger = nullptr;

        #ifdef ARDUINO_ARCH_ESP32
        void onEvent(WiFiEvent_t event);
        #endif
    public:
        WifiManager(String ssid, String pass);
        void begin();
        void disconnect();
        bool isConnected();
        void onConnected(std::function<void(String)> callback);
        void onDisconnected(std::function<void()> callback);
        void setAutoReconnect(bool autoReconnect);
        bool isAutoReconnect();
        void setLogger(Print& logger);
        WifiStatus status();
        String address();
        long strength();
};

#endif // HUB_WIFI_MANAGER_H