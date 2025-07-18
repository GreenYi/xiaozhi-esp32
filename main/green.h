// green.h
#ifndef _GREEN_H_
#define _GREEN_H_

namespace GreenConfig {
    // 当SPEAK_COUNT大于等于SPEAK_COUNT_STOP时设置设备状态为空闲
    constexpr int SPEAK_COUNT = 0;
    constexpr int SPEAK_COUNT_STOP = 3;
}

#endif // _GREEN_H_