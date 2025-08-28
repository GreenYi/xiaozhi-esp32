#ifndef GreenService_H
#define GreenService_H

#include <mbedtls/base64.h>
#include <vector>
#include <string>
#include "cJSON.h"
#include "esp_log.h"
#include "application.h"
#include "board.h"
#include "display.h"

class GreenService {
public:
    // 单例模式访问
    static GreenService& Instance();
    
    // 新增总方法，总方法里面调用其他的各个方法
    bool Process(const std::string& text);

    // 处理TTS
    bool ProcessTTS(const std::string& text);

    // 设置显示消息
    bool SetChatMessage(const std::string& message);

    // 发送HTTP POST请求TTS
    bool SendTTSRequest(const std::string& text, std::string& response);
    
    // 解码base64音频数据
    std::vector<uint8_t> DecodeBase64Audio(const std::string& base64Data);
    
    // 解析并返回音频数据
    std::vector<uint8_t> ParseAudioData(const std::string& response);
    
    // 播放音频数据
    bool PlaySound(const std::vector<uint8_t>& audioData);

    // 设置设备状态
    bool SetDeviceState();

private:
    // 私有构造函数确保单例
    GreenService() = default;
    ~GreenService() = default;
    
    // 禁用拷贝和赋值
    GreenService(const GreenService&) = delete;
    GreenService& operator=(const GreenService&) = delete;
};

#endif // GreenService_H
