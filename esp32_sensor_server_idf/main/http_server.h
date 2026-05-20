#ifndef HTTP_SERVER_H
#define HTTP_SERVER_H

#include <esp_http_server.h>

// 启动 Web 服务器
httpd_handle_t start_webserver(void);

// 停止 Web 服务器
void stop_webserver(httpd_handle_t server);

#endif
