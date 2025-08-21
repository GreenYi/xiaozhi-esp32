// green_mqtt.cc
#include "green_mqtt.h"
#include "mqtt_protocol.h"
#include "board.h"
#include "application.h"
#include "settings.h"
#include <esp_log.h>
#include <cstring>
#include <arpa/inet.h>
#include "assets/lang_config.h"

// 只保留实现部分，不要重新定义类
GreenMqtt& GreenMqtt::Instance() {
    static GreenMqtt instance;
    return instance;
}

bool GreenMqtt::Publish(const char* message) {
    return Publish(GreenConfig::MQTT_TOPIC, message);
}

bool GreenMqtt::Publish(const char* topic, const char* message) {
    if (!EnsureConnected()) {
        ESP_LOGE("GreenMqtt", "Publish failed: no connection");
        return false;
    }

    if (!mqtt_->Publish(topic, message)) {
        ESP_LOGE("GreenMqtt", "Publish failed to %s", topic);
        return false;
    }

    ESP_LOGI("GreenMqtt", "Published to %s: %s", topic, message);
    return true;
}

GreenMqtt::~GreenMqtt() {
    if (mqtt_) {
        mqtt_->Disconnect();
    }
}

bool GreenMqtt::EnsureConnected() {
    if (mqtt_ && mqtt_->IsConnected()) {
        return true;
    }

    auto now = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();

    // 防止频繁重连
    if (last_connect_time_ + GreenConfig::MQTT_RECONNECT_DELAY_MS > now) {
        return false;
    }

    last_connect_time_ = now;
    
    mqtt_ = Board::GetInstance().GetNetwork()->CreateMqtt(1);

    if (!mqtt_ || !mqtt_->Connect(GreenConfig::MQTT_SERVER_IP, 
                                GreenConfig::MQTT_SERVER_PORT,
                                GreenConfig::MQTT_CLIENT_ID, "", "")) {
        ESP_LOGE("GreenMqtt", "Connect failed to %s:%d", 
                GreenConfig::MQTT_SERVER_IP, 
                GreenConfig::MQTT_SERVER_PORT);
        mqtt_.reset();
        return false;
    }

    ESP_LOGI("GreenMqtt", "Connected to MQTT server");
    return true;
}
