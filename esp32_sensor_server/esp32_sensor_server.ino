#include <WiFi.h>
#include <WebServer.h>
#include <ArduinoJson.h>

// WiFi 配置 - 请修改为你的 WiFi 信息
const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";

WebServer server(80);

// 模拟传感器数据（实际使用时替换为真实传感器读取）
float temperature = 25.0;
float humidity = 60.0;
int light = 500;
int soilMoisture = 45;

void handleRoot() {
  String html = R"(
<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>ESP32 传感器监控</title>
    <style>
        * { margin: 0; padding: 0; box-sizing: border-box; }
        body {
            font-family: 'Microsoft YaHei', Arial, sans-serif;
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            min-height: 100vh;
            padding: 20px;
        }
        .container {
            max-width: 800px;
            margin: 0 auto;
        }
        h1 {
            text-align: center;
            color: white;
            margin-bottom: 30px;
            font-size: 2.5em;
            text-shadow: 2px 2px 4px rgba(0,0,0,0.3);
        }
        .sensor-grid {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(180px, 1fr));
            gap: 20px;
            margin-bottom: 30px;
        }
        .sensor-card {
            background: white;
            border-radius: 20px;
            padding: 25px;
            text-align: center;
            box-shadow: 0 10px 30px rgba(0,0,0,0.2);
            transition: transform 0.3s;
        }
        .sensor-card:hover {
            transform: translateY(-5px);
        }
        .sensor-icon {
            font-size: 3em;
            margin-bottom: 15px;
        }
        .sensor-name {
            color: #666;
            font-size: 0.9em;
            margin-bottom: 10px;
        }
        .sensor-value {
            font-size: 2.5em;
            font-weight: bold;
            color: #333;
        }
        .sensor-unit {
            font-size: 1em;
            color: #999;
        }
        .temp { border-top: 5px solid #ff6b6b; }
        .humi { border-top: 5px solid #4ecdc4; }
        .light { border-top: 5px solid #ffe66d; }
        .soil { border-top: 5px solid #95e1d3; }
        .status {
            background: white;
            border-radius: 15px;
            padding: 20px;
            text-align: center;
            box-shadow: 0 5px 20px rgba(0,0,0,0.2);
        }
        .status-text {
            color: #666;
            margin-bottom: 10px;
        }
        .update-time {
            color: #333;
            font-weight: bold;
            font-size: 1.2em;
        }
        .loading {
            animation: pulse 1.5s infinite;
        }
        @keyframes pulse {
            0%, 100% { opacity: 1; }
            50% { opacity: 0.5; }
        }
    </style>
</head>
<body>
    <div class="container">
        <h1>🌱 ESP32 传感器监控</h1>
        <div class="sensor-grid">
            <div class="sensor-card temp">
                <div class="sensor-icon">🌡️</div>
                <div class="sensor-name">温度</div>
                <div class="sensor-value" id="temp">--</div>
                <div class="sensor-unit">°C</div>
            </div>
            <div class="sensor-card humi">
                <div class="sensor-icon">💧</div>
                <div class="sensor-name">湿度</div>
                <div class="sensor-value" id="humi">--</div>
                <div class="sensor-unit">%</div>
            </div>
            <div class="sensor-card light">
                <div class="sensor-icon">☀️</div>
                <div class="sensor-name">光照</div>
                <div class="sensor-value" id="light">--</div>
                <div class="sensor-unit">lux</div>
            </div>
            <div class="sensor-card soil">
                <div class="sensor-icon">🌿</div>
                <div class="sensor-name">土壤湿度</div>
                <div class="sensor-value" id="soil">--</div>
                <div class="sensor-unit">%</div>
            </div>
        </div>
        <div class="status">
            <div class="status-text">最后更新时间</div>
            <div class="update-time" id="time">等待数据...</div>
        </div>
    </div>

    <script>
        function updateData() {
            fetch('/api/sensors')
                .then(response => response.json())
                .then(data => {
                    document.getElementById('temp').textContent = data.temperature.toFixed(1);
                    document.getElementById('humi').textContent = data.humidity.toFixed(1);
                    document.getElementById('light').textContent = data.light;
                    document.getElementById('soil').textContent = data.soilMoisture;
                    document.getElementById('time').textContent = new Date().toLocaleString('zh-CN');
                })
                .catch(err => {
                    console.error('获取数据失败:', err);
                    document.getElementById('time').textContent = '连接失败 - ' + new Date().toLocaleTimeString('zh-CN');
                });
        }

        // 初始加载
        updateData();
        // 每2秒更新一次
        setInterval(updateData, 2000);
    </script>
</body>
</html>
)";
  server.send(200, "text/html", html);
}

void handleApi() {
  StaticJsonDocument<256> doc;
  doc["temperature"] = temperature;
  doc["humidity"] = humidity;
  doc["light"] = light;
  doc["soilMoisture"] = soilMoisture;

  String response;
  serializeJson(doc, response);
  server.send(200, "application/json", response);
}

void setup() {
  Serial.begin(115200);

  // 连接 WiFi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.print("Connected! IP: ");
  Serial.println(WiFi.localIP());

  // 设置路由
  server.on("/", handleRoot);
  server.on("/api/sensors", handleApi);
  server.begin();

  Serial.println("HTTP server started");
}

void loop() {
  server.handleClient();

  // 模拟传感器数据变化（实际使用时替换为真实传感器读取代码）
  static unsigned long lastUpdate = 0;
  if (millis() - lastUpdate > 3000) {
    temperature = 20 + random(0, 150) / 10.0;
    humidity = 40 + random(0, 400) / 10.0;
    light = 300 + random(0, 700);
    soilMoisture = 30 + random(0, 50);
    lastUpdate = millis();
  }
}
