cmake_minimum_required(VERSION 3.5)

# 设置目标芯片
set(IDF_TARGET esp32)

# 包含项目
include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(esp32_sensor_server)
