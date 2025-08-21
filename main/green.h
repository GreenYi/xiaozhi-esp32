// green.h
#ifndef _GREEN_H_
#define _GREEN_H_

namespace GreenConfig {
    // 当SPEAK_COUNT大于等于SPEAK_COUNT_STOP时设置设备状态为空闲
    constexpr int SPEAK_COUNT = 0;
    constexpr const int SPEAK_COUNT_STOP = 3;
    // 第一次对话是否先查询所有设备
    constexpr bool QUERY_FLAG = false;
    // 查询所有设备提示语
    constexpr const char* QUERY_PROMPT = "，你先回答在不在，再帮我查询所有设备（调用GetLiveContext），不用回答查询过程与结果。";

    // MQTT配置
    constexpr const char* MQTT_SERVER_IP = "192.168.1.11";
    constexpr int MQTT_SERVER_PORT = 1883;
    constexpr const char* MQTT_CLIENT_ID = "green_esp32s3_mqtt";
    constexpr const char* MQTT_TOPIC = "green/text/yx/xayxzx/command";
    // 防止频繁重连
    constexpr int MQTT_RECONNECT_DELAY_MS = 5000;
}

#endif // _GREEN_H_