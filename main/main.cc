#include <esp_log.h>
#include <esp_err.h>
#include <nvs.h>
#include <nvs_flash.h>
#include <driver/gpio.h>
#include <esp_event.h>

#include "application.h"
#include "system_info.h"

#define TAG "main"

extern "C" void app_main(void)
{
    // Initialize the default event loop
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    // Initialize NVS flash for WiFi configuration
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_LOGW(TAG, "Erasing NVS flash to fix corruption");
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // Launch the application
    auto& app = Application::GetInstance();
    app.Start();
    app.MainEventLoop();

    // ***以下为远程测试***

    // 启动 Application 主循环（放在单独的任务里）
    // xTaskCreate(
    //      [](void* arg) {
    //          Application::GetInstance().MainEventLoop();
    //          vTaskDelete(NULL); // 如果 Start() 退出，删除任务
    //      },
    //      "AppMainTask",
    //      4096,
    //      NULL,
    //      5, // 优先级
    //      NULL
    //  );
    //  // 创建并启动1分钟的定时器
    //  esp_timer_handle_t auto_test_timer;
    //  const esp_timer_create_args_t auto_test_timer_args = {
    //     .callback = [](void* arg) {
    //          ESP_LOGI(TAG, "Auto test timer triggered");
    //         Application::GetInstance().WakeWordInvoke("小爱同学");
    //      },
    //      .arg = nullptr,
    //      .dispatch_method = ESP_TIMER_TASK,
    //      .name = "auto_test_timer"
    // };
    //  ESP_ERROR_CHECK(esp_timer_create(&auto_test_timer_args, &auto_test_timer));
    //  ESP_ERROR_CHECK(esp_timer_start_once(auto_test_timer, 10 * 1000000)); // 60秒后触发
    //  ESP_LOGI(TAG, "Auto test timer set for 60 seconds later");

}
