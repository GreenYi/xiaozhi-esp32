// green_mqtt.h
#ifndef GREEN_MQTT_H
#define GREEN_MQTT_H

#include <memory>
#include <string>

class Mqtt;

class GreenMqtt {
public:
    static GreenMqtt& Instance();

    // 发布主题
    bool Publish(const char* message);
    // 发布主题
    bool Publish(const char* topic, const char* message);
    // 订阅主题
    bool Subscribe(const char* topic);
    // 消息回调方法
    void MessageReceived(const char* topic, const char* message);
    // 异步延迟初始化连接
    void ScheduleConnectAfterDelay(uint64_t delay_ms);

private:
    GreenMqtt() = default;
    ~GreenMqtt();

    // 确认MQTT已连接
    bool EnsureConnected();
    // 在已连接状态下重复订阅主题（重连后复用）
    bool SubscribeInternal(const char* topic);
    // 初始化 MQTT 客户端的回调，确保消息回调与重连订阅被统一注册
    void InitializeMqttCallbacks();

    // 禁用拷贝
    GreenMqtt(const GreenMqtt&) = delete;
    GreenMqtt& operator=(const GreenMqtt&) = delete;

    std::unique_ptr<Mqtt> mqtt_;
    uint64_t last_connect_time_ = 0;
};

#endif // GREEN_MQTT_H
