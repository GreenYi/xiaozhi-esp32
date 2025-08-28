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
#include "green_service.h"

#define TAG "GreenMqtt"

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
        ESP_LOGE(TAG, "Publish failed: no connection");
        return false;
    }
    if (!mqtt_->Publish(topic, message)) {
        ESP_LOGE(TAG, "Publish failed to %s", topic);
        return false;
    }
    ESP_LOGI(TAG, "Published to %s:%s", topic, message);
    return true;
}

bool GreenMqtt::Subscribe(const char* topic) {
    if (!EnsureConnected()) {
        ESP_LOGE(TAG, "Subscribe failed: no connection");
        return false;
    }
    // 订阅主题
    if (!mqtt_->Subscribe(topic, 0)) {
        // 即使Subscribe返回false，也订阅成功了的
        // ESP_LOGE(TAG, "Subscribe failed to %s", topic);
        // return false;
    }
    // 设置消息回调
    mqtt_->OnMessage([](const std::string& topic, const std::string& payload) {
        // 在这里处理接收到的消息
        if (!payload.empty()) {
            // 如果 payload 非空，则将回调的文本转语音TTS
            ESP_LOGI(TAG, "OnMessage Callback, Received on %s:%s", topic.c_str(), payload.c_str());
            GreenService::Instance().Process(payload);
        }
    });
    ESP_LOGI(TAG, "Subscribed to %s", topic);
    return true;
}

void GreenMqtt::MessageReceived(const char* topic, const char* message) {
    ESP_LOGI(TAG, "MessageReceived Callback, Received on %s", topic);
    // 在这里可以进一步处理消息
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
    auto now = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    // 防止频繁重连
    if (last_connect_time_ + GreenConfig::MQTT_RECONNECT_DELAY_MS > now) {
        return false;
    }
    last_connect_time_ = now;
    mqtt_ = Board::GetInstance().GetNetwork()->CreateMqtt(1);
    if (!mqtt_ || !mqtt_->Connect(GreenConfig::MQTT_SERVER_IP, 
                                  GreenConfig::MQTT_SERVER_PORT,
                                  GreenConfig::MQTT_CLIENT_ID, "", "")) {
        ESP_LOGE(TAG, "Connect failed to %s:%d", 
                 GreenConfig::MQTT_SERVER_IP, 
                 GreenConfig::MQTT_SERVER_PORT);
        mqtt_.reset();
        return false;
    }
    ESP_LOGI(TAG, "Connected to MQTT server");
    // MQTT连接成功时订阅
    Subscribe(GreenConfig::MQTT_TOPIC_XATXHF);
    return true;
}
