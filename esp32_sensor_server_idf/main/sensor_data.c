#include "sensor_data.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "esp_log.h"
#include "driver/adc.h"
#include "esp_adc_cal.h"
#include <math.h>
#include <stdlib.h>

static const char *TAG = "SENSOR";

// 静态传感器数据
static sensor_data_t g_sensor_data = {
    .temperature = 25.0f,
    .humidity = 60.0f,
    .light = 500,
    .soilMoisture = 45
};

// 互斥锁保护数据访问
static SemaphoreHandle_t data_mutex = NULL;

// ADC 校准句柄
static esp_adc_cal_characteristics_t adc_chars;
static bool adc_initialized = false;

// 初始化ADC
static void adc_init(void)
{
    // 配置ADC1
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(ADC1_CHANNEL_0, ADC_ATTEN_DB_12);  // GPIO36 - 光照
    adc1_config_channel_atten(ADC1_CHANNEL_3, ADC_ATTEN_DB_12);  // GPIO39 - 土壤湿度

    // ADC校准
    esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_12, ADC_WIDTH_BIT_12, 1100, &adc_chars);
    adc_initialized = true;

    ESP_LOGI(TAG, "ADC initialized");
}

// 从ADC读取光照值
static int read_light_sensor(void)
{
    if (!adc_initialized) return g_sensor_data.light;

    int raw = adc1_get_raw(ADC1_CHANNEL_0);
    uint32_t voltage = esp_adc_cal_raw_to_voltage(raw, &adc_chars);
    // 转换为lux（根据实际传感器校准）
    int lux = (int)(voltage * 1.5f);
    return lux > 1000 ? 1000 : lux;
}

// 从ADC读取土壤湿度
static int read_soil_moisture(void)
{
    if (!adc_initialized) return g_sensor_data.soilMoisture;

    int raw = adc1_get_raw(ADC1_CHANNEL_3);
    uint32_t voltage = esp_adc_cal_raw_to_voltage(raw, &adc_chars);
    // 转换为百分比（根据实际传感器校准）
    int moisture = (int)((3300 - voltage) / 3300.0f * 100);
    if (moisture < 0) moisture = 0;
    if (moisture > 100) moisture = 100;
    return moisture;
}

// 模拟读取温度（实际使用DHT11/DHT22/DS18B20等）
static float read_temperature(void)
{
    // TODO: 替换为实际传感器读取
    // 例如：dht_read_temperature()
    // 目前返回带微小变化的模拟值
    static float base_temp = 25.0f;
    float variation = ((float)(rand() % 100) - 50.0f) / 10.0f;
    base_temp += variation * 0.1f;

    // 限制范围
    if (base_temp > 40.0f) base_temp = 40.0f;
    if (base_temp < 10.0f) base_temp = 10.0f;

    return base_temp;
}

// 模拟读取湿度（实际使用DHT11/DHT22等）
static float read_humidity(void)
{
    // TODO: 替换为实际传感器读取
    static float base_humi = 60.0f;
    float variation = ((float)(rand() % 100) - 50.0f) / 10.0f;
    base_humi += variation * 0.1f;

    // 限制范围
    if (base_humi > 90.0f) base_humi = 90.0f;
    if (base_humi < 20.0f) base_humi = 20.0f;

    return base_humi;
}

void update_sensor_data(void)
{
    if (data_mutex == NULL) return;

    if (xSemaphoreTake(data_mutex, pdMS_TO_TICKS(100)) == pdTRUE) {
        // 读取各个传感器
        g_sensor_data.temperature = read_temperature();
        g_sensor_data.humidity = read_humidity();
        g_sensor_data.light = read_light_sensor();
        g_sensor_data.soilMoisture = read_soil_moisture();

        xSemaphoreGive(data_mutex);
    }
}

sensor_data_t get_sensor_data(void)
{
    sensor_data_t data = {0};

    if (data_mutex != NULL && xSemaphoreTake(data_mutex, pdMS_TO_TICKS(100)) == pdTRUE) {
        data = g_sensor_data;
        xSemaphoreGive(data_mutex);
    }

    return data;
}

void sensor_data_task(void *pvParameters)
{
    // 创建互斥锁
    data_mutex = xSemaphoreCreateMutex();
    if (data_mutex == NULL) {
        ESP_LOGE(TAG, "Failed to create mutex");
        vTaskDelete(NULL);
        return;
    }

    // 初始化ADC
    adc_init();

    ESP_LOGI(TAG, "Sensor data task started");

    while (1) {
        update_sensor_data();

        ESP_LOGI(TAG, "Temp: %.1f°C, Humi: %.1f%%, Light: %d lux, Soil: %d%%",
                 g_sensor_data.temperature,
                 g_sensor_data.humidity,
                 g_sensor_data.light,
                 g_sensor_data.soilMoisture);

        vTaskDelay(pdMS_TO_TICKS(3000)); // 每3秒更新一次
    }
}
