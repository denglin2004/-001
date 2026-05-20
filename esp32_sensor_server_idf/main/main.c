#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "wifi_connect.h"
#include "http_server.h"
#include "sensor_data.h"

static const char *TAG = "MAIN";

void app_main(void)
{
    // 初始化 NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    ESP_LOGI(TAG, "ESP32 Sensor Server Starting...");

    // 初始化 WiFi
    wifi_init_sta();

    // 等待 WiFi 连接
    vTaskDelay(pdMS_TO_TICKS(2000));

    // 启动传感器数据采集任务
    xTaskCreate(sensor_data_task, "sensor_task", 4096, NULL, 5, NULL);

    // 启动 HTTP 服务器
    start_webserver();

    ESP_LOGI(TAG, "System initialized successfully");
}
