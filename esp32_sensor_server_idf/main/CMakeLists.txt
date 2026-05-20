idf_component_register(
    SRCS "main.c" "wifi_connect.c" "http_server.c" "sensor_data.c"
    INCLUDE_DIRS "."
    EMBED_FILES "web/index.html"
)
