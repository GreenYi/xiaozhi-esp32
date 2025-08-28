#include "green_service.h"
#include <sstream>

#define TAG "GreenService"

// 只保留实现部分，不要重新定义类
GreenService& GreenService::Instance() {
    static GreenService instance;
    return instance;
}

bool GreenService::SetChatMessage(const std::string& message) {
    try {
        auto display = Board::GetInstance().GetDisplay();
        Application::GetInstance().Schedule([display, message]() {
            display->SetChatMessage("assistant", message.c_str());
        });
        return true;
    } catch (...) {
        return false;
    }
}

bool GreenService::SendTTSRequest(const std::string& text, std::string& response) {
    // 获取 Board 实例并创建 HTTP 客户端
    auto& board = Board::GetInstance();
    auto http = board.GetNetwork()->CreateHttp(0);
    // 构造 URL 和请求头部
    std::string url = "https://openspeech.bytedance.com/api/v3/tts/unidirectional";
    // 准备请求体
    std::string payload = "{\"req_params\": {\"text\": \"" + text + "\", "
                          "\"speaker\": \"zh_female_kefunvsheng_mars_bigtts\", "
                          "\"additions\": \"{\\\"disable_markdown_filter\\\":true,\\\"silence_duration\\\":500,\\\"enable_language_detector\\\":true,\\\"enable_latex_tn\\\":true,\\\"disable_default_bit_rate\\\":true,\\\"max_length_to_filter_parenthesis\\\":0,\\\"cache_config\\\":{\\\"text_type\\\":1,\\\"use_cache\\\":true}}\", "
                          "\"audio_params\": {\"format\": \"ogg_opus\", \"sample_rate\": 24000, \"loudness_rate\": 50}}}";
    // 设置请求头
    http->SetHeader("x-api-key", GreenConfig::API_KEY);
    http->SetHeader("X-Api-Resource-Id", "volc.service_type.10029");
    http->SetHeader("Connection", "keep-alive");
    http->SetHeader("Content-Type", "application/json");
    // 设置请求体
    http->SetContent(payload.c_str());
    // 打开连接并发送请求
    if (!http->Open("POST", url)) {
        ESP_LOGE("HTTP", "Failed to open HTTP connection");
        return false;
    }
    // 获取响应状态码
    auto status_code = http->GetStatusCode();
    if (status_code != 200) {
        ESP_LOGE("HTTP", "Failed to send request, status code: %d", status_code);
        return false;
    }
    // 读取响应数据
    response = http->ReadAll();
    http->Close();
    // 调试输出响应数据
    ESP_LOGI("HTTP", "Response OK");
    return true;
}

std::vector<uint8_t> GreenService::DecodeBase64Audio(const std::string& base64Data) {
    // 计算解码后数据长度
    size_t output_len = 0;
    int ret = mbedtls_base64_decode(nullptr, 0, &output_len, 
                                  (const unsigned char*)base64Data.data(), base64Data.length());
    if (ret != MBEDTLS_ERR_BASE64_BUFFER_TOO_SMALL) {
        // 处理错误
        return {};
    }
    std::vector<uint8_t> decodedData(output_len);
    // 执行base64解码
    ret = mbedtls_base64_decode(decodedData.data(), decodedData.size(), &output_len,
                               (const unsigned char*)base64Data.data(), base64Data.length());
    if (ret != 0) {
        // 处理解码错误
        return {};
    }
    decodedData.resize(output_len);
    return decodedData;
}


std::vector<uint8_t> GreenService::ParseAudioData(const std::string& response) {
    // 按行分割输入（假设每个JSON对象占一行）
    std::istringstream stream(response);
    std::string line;
    std::vector<uint8_t> combinedAudioData; // 存储所有解码后的音频数据
    while (std::getline(stream, line)) {
        // 跳过空行
        if (line.empty()) continue;
        cJSON* root = cJSON_Parse(line.c_str());
        if (root == nullptr) {
            ESP_LOGE("Audio", "Failed to parse JSON line: %s", line.c_str());
             return {};
        }
        // 检查code
        cJSON* codeItem = cJSON_GetObjectItem(root, "code");
        if (!codeItem || !cJSON_IsNumber(codeItem)) {
            ESP_LOGE("Audio", "Invalid or missing code field");
            cJSON_Delete(root);
             return {};
        }
        int code = codeItem->valueint;
        if (code != 0) {
            ESP_LOGE(TAG, "parseAudioData code != 0: %d", code);
            cJSON_Delete(root);
            // 如果遇到非0的code，可以选择继续或中断
            return {};
        }
        // 检查data字段
        cJSON* data = cJSON_GetObjectItem(root, "data");
        if (data == nullptr) {
            cJSON_Delete(root);
            break; // 遇到data为null时停止
        }
        if (cJSON_IsString(data)) {
            std::string base64Data = data->valuestring;
            // 调用解码方法
            auto decodedAudio = DecodeBase64Audio(base64Data);
            if (decodedAudio.empty()) {
                ESP_LOGE("Audio", "Failed to decode audio data");
            } else {
                // 将解码后的数据拼接到总数据中
                combinedAudioData.insert(combinedAudioData.end(), 
                                        decodedAudio.begin(), decodedAudio.end());
            }
        } else if (cJSON_IsNull(data)) {
            cJSON_Delete(root);
            break; // 遇到data为null时停止
        } else {
            ESP_LOGE(TAG, "parseAudioData data is not string or null: %s", cJSON_Print(data));
        }
        cJSON_Delete(root);
    }
    return combinedAudioData;
}

bool GreenService::PlaySound(const std::vector<uint8_t>& audioData) {
    // 如果有解码的音频数据，则播放
    if (!audioData.empty()) {
        std::string_view audioView(
            reinterpret_cast<const char*>(audioData.data()),
            audioData.size()
        );
        auto codec = Board::GetInstance().GetAudioCodec();
        codec->EnableOutput(true);
        Application::GetInstance().PlaySound(audioView);
        return true;
    } else {
        ESP_LOGE(TAG, "No audio data to play");
        return false;
    }
}

bool GreenService::SetDeviceState() {
    try {
        auto& app = Application::GetInstance();
        if (app.GetDeviceState() == kDeviceStateSpeaking) {
            if (app.speak_count >= 1) {
                // 小爱音箱回复后更新状态see GreenService.SetDeviceState()
                app.speak_count++;
                ESP_LOGI(TAG, "Greenyi: speak_count: %d", app.speak_count);
                // 新增判断：当speak_count大于等于SPEAK_COUNT_STOP时设置设备状态为空闲
                if (app.speak_count >= GreenConfig::SPEAK_COUNT_STOP) {
                    app.SetDeviceState(kDeviceStateIdle);
                } else {
                    app.SetDeviceState(kDeviceStateListening);
                }
            }
        }
        return true;
    } catch (...) {
        return false;
    }
}

bool GreenService::ProcessTTS(const std::string& text) {
    std::string response;

    // 1. 设置显示消息
    if (!SetChatMessage(text)) {
        ESP_LOGW(TAG, "Failed to set chat message");
    }

    // 2. 发送TTS请求
    if (!SendTTSRequest(text, response)) {
        ESP_LOGE(TAG, "TTS request failed");
        return false;
    }

    // 3. 解析音频数据
    std::vector<uint8_t> audioData = ParseAudioData(response);
    if (audioData.empty()) {
        ESP_LOGE(TAG, "No audio data parsed");
        return false;
    }

    // 4. 播放音频
    if (!PlaySound(audioData)) {
        ESP_LOGE(TAG, "Audio playback failed");
        return false;
    }
    
    return true;
}


// 新增总方法，总方法里面调用其他的各个方法
bool GreenService::Process(const std::string& text) {

    // 处理TTS
    ProcessTTS(text);

    // 设置设备状态
    if (!SetDeviceState()) {
        ESP_LOGE(TAG, "Failed to set device state");
        return false;
    }

    return true;
}
