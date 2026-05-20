# ESP32 传感器监控网页

## 文件说明

| 文件 | 说明 |
|------|------|
| `esp32_sensor_server.ino` | ESP32 Arduino 代码 |
| `sensor_page.html` | 单独的网页文件（可在浏览器直接打开） |

## 使用方法

### 1. 烧录 ESP32 代码

1. 打开 Arduino IDE
2. 安装依赖库：`ArduinoJson`
3. 修改代码中的 WiFi 信息：
   ```cpp
   const char* ssid = "你的WiFi名称";
   const char* password = "你的WiFi密码";
   ```
4. 上传到 ESP32
5. 打开串口监视器查看分配的 IP 地址

### 2. 访问网页

**方式一：ESP32 内置网页**
- 在浏览器访问 ESP32 的 IP 地址（如 `http://192.168.1.100`）

**方式二：单独网页文件**
- 用浏览器打开 `sensor_page.html`
- 输入 ESP32 的 IP 地址，点击连接

### 3. 连接真实传感器

代码中注释了模拟数据部分，接入真实传感器时替换为实际读取代码：

```cpp
// 示例：DHT11 温湿度传感器
#include <DHT.h>
DHT dht(D4, DHT11);

// 在 loop() 中替换为：
temperature = dht.readTemperature();
humidity = dht.readHumidity();
```

## 数据接口

ESP32 提供 REST API：
- `GET /` - 返回完整网页
- `GET /api/sensors` - 返回 JSON 传感器数据

### JSON 格式示例
```json
{
  "temperature": 25.5,
  "humidity": 60.0,
  "light": 500,
  "soilMoisture": 45
}
```

## 界面预览

- 🌡️ 温度显示（红色）
- 💧 湿度显示（青色）
- ☀️ 光照显示（黄色）
- 🌿 土壤湿度显示（绿色）

数据每 2 秒自动刷新。
