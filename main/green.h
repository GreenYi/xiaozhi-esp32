// green.h
#ifndef _GREEN_H_
#define _GREEN_H_

namespace GreenConfig {
    // 当SPEAK_COUNT大于等于SPEAK_COUNT_STOP时设置设备状态为空闲
    constexpr int SPEAK_COUNT = 0;
    constexpr int SPEAK_COUNT_STOP = 3;
    // 第一次对话是否先查询所有设备
    constexpr bool QUERY_FLAG = true;
    // 查询所有设备提示语
    constexpr char* QUERY_PROMPT = "，你先回答在不在，再帮我查询所有设备（调用GetLiveContext），不用回答查询过程与结果。";
}

#endif // _GREEN_H_