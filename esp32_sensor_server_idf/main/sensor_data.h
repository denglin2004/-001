#ifndef SENSOR_DATA_H
#define SENSOR_DATA_H

typedef struct {
    float temperature;
    float humidity;
    int light;
    int soilMoisture;
} sensor_data_t;

// 获取当前传感器数据
sensor_data_t get_sensor_data(void);

// 传感器数据采集任务
void sensor_data_task(void *pvParameters);

// 更新传感器数据（实际硬件读取）
void update_sensor_data(void);

#endif
