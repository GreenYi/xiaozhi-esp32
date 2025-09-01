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

private:
    GreenMqtt() = default;
    ~GreenMqtt();

    // 禁用拷贝
    GreenMqtt(const GreenMqtt&) = delete;
    GreenMqtt& operator=(const GreenMqtt&) = delete;

    // 确认MQTT已连接
    bool EnsureConnected();

    std::unique_ptr<Mqtt> mqtt_;
    uint64_t last_connect_time_ = 0;
};

#endif // GREEN_MQTT_H
