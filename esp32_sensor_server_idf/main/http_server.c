#include "http_server.h"
#include "sensor_data.h"
#include "esp_log.h"
#include "cJSON.h"
#include <string.h>

static const char *TAG = "HTTP";

// 嵌入的网页数据
extern const char index_html_start[] asm("_binary_index_html_start");
extern const char index_html_end[]   asm("_binary_index_html_end");

// GET / 主页处理器
static esp_err_t root_get_handler(httpd_req_t *req)
{
    httpd_resp_set_type(req, "text/html; charset=utf-8");
    httpd_resp_send(req, index_html_start, index_html_end - index_html_start);
    return ESP_OK;
}

// GET /api/sensors 传感器数据API
static esp_err_t api_sensors_get_handler(httpd_req_t *req)
{
    // 设置 CORS 头
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    httpd_resp_set_type(req, "application/json");

    // 获取传感器数据
    sensor_data_t data = get_sensor_data();

    // 创建 JSON 对象
    cJSON *root = cJSON_CreateObject();
    cJSON_AddNumberToObject(root, "temperature", data.temperature);
    cJSON_AddNumberToObject(root, "humidity", data.humidity);
    cJSON_AddNumberToObject(root, "light", data.light);
    cJSON_AddNumberToObject(root, "soilMoisture", data.soilMoisture);

    // 转换为字符串
    char *json_str = cJSON_Print(root);
    if (json_str == NULL) {
        cJSON_Delete(root);
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "JSON creation failed");
        return ESP_FAIL;
    }

    httpd_resp_sendstr(req, json_str);

    // 清理
    cJSON_free(json_str);
    cJSON_Delete(root);

    return ESP_OK;
}

// OPTIONS 处理（CORS 预检）
static esp_err_t options_handler(httpd_req_t *req)
{
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    httpd_resp_set_hdr(req, "Access-Control-Allow-Methods", "GET, POST, OPTIONS");
    httpd_resp_set_hdr(req, "Access-Control-Allow-Headers", "Content-Type");
    httpd_resp_send(req, NULL, 0);
    return ESP_OK;
}

// URI 定义
static const httpd_uri_t root_uri = {
    .uri       = "/",
    .method    = HTTP_GET,
    .handler   = root_get_handler,
    .user_ctx  = NULL
};

static const httpd_uri_t api_sensors_uri = {
    .uri       = "/api/sensors",
    .method    = HTTP_GET,
    .handler   = api_sensors_get_handler,
    .user_ctx  = NULL
};

static const httpd_uri_t options_uri = {
    .uri       = "/*",
    .method    = HTTP_OPTIONS,
    .handler   = options_handler,
    .user_ctx  = NULL
};

httpd_handle_t start_webserver(void)
{
    httpd_handle_t server = NULL;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();

    // 设置服务器端口
    config.server_port = 80;
    config.lru_purge_enable = true;

    ESP_LOGI(TAG, "Starting HTTP Server on port %d", config.server_port);

    if (httpd_start(&server, &config) == ESP_OK) {
        // 注册 URI 处理器
        httpd_register_uri_handler(server, &root_uri);
        httpd_register_uri_handler(server, &api_sensors_uri);
        httpd_register_uri_handler(server, &options_uri);

        ESP_LOGI(TAG, "HTTP server started successfully");
        return server;
    }

    ESP_LOGE(TAG, "Failed to start HTTP server");
    return NULL;
}

void stop_webserver(httpd_handle_t server)
{
    if (server != NULL) {
        httpd_stop(server);
        ESP_LOGI(TAG, "HTTP server stopped");
    }
}
