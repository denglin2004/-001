# CH32V307 打磨机器人主控

## 项目简介

本项目是2026年嵌入式芯片与系统设计竞赛 - AI打磨小车的核心控制程序，基于沁恒微CH32V307RCT6微控制器开发。

## 硬件平台

- **主控芯片**: CH32V307RCT6 (RISC-V架构, 144MHz, 64KB SRAM, 256KB Flash)
- **外设模块**:
  - CAN总线通信
  - USART串口通信
  - OLED显示屏 (SSD1306)
  - RGB LED指示
  - 蜂鸣器
  - ESP32 WiFi模块通信

## 项目结构

```
├── APP/          # 应用层代码
│   ├── DCCP_comand.c/h    # DCCP协议处理
│   ├── ESP32_comand.c/h   # ESP32通信
│   ├── can_app.c/h        # CAN总线应用
│   ├── core_control.c/h   # 核心控制逻辑
│   └── scheduler.c/h      # 任务调度器
├── BSP/          # 板级支持包
├── Core/         # RISC-V核心代码
├── Drivers/      # 外设驱动 (OLED等)
├── HAL/          # 硬件抽象层
├── Ld/           # 链接脚本
├── Middleware/   # 中间件
├── Peripheral/   # 芯片外设库
├── Startup/      # 启动文件
├── User/         # 用户代码
├── esp32_sensor_server/        # Arduino版ESP32程序
└── esp32_sensor_server_idf/    # IDF版ESP32程序
```

## 开发环境

- MounRiver Studio (MRS)
- GCC RISC-V工具链

## 许可证

本项目仅供学习交流使用。
