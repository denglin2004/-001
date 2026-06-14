#ifndef SAFETY_MONITOR_H
#define SAFETY_MONITOR_H

#include <stdint.h>
#include <stdbool.h>

// 安全监控周期：50ms（必须快于从站心跳周期100ms）
#define SAFETY_MONITOR_PERIOD_MS    50
#define HEARTBEAT_TIMEOUT_MS        300   // 3个心跳周期
#define PDO_TIMEOUT_MS              150   // 3个PDO周期（假设50ms发一次PDO）
#define BUSOFF_RECOVER_MS           1000  // BusOff后尝试恢复间隔
#include "ch32v30x.h"
typedef enum 
{
    SYS_STATE_INIT = 0,
    SYS_STATE_SELFTEST,     // 上电自检
    SYS_STATE_STANDBY,      // 待机（所有从站在线但未使能）
    SYS_STATE_OPERATIONAL,  // 正常运行
    SYS_STATE_DEGRADED,     // 降级（部分从站故障，其余继续）
    SYS_STATE_SAFE_STOP,    // 安全停机（保持位置，抱闸）
    SYS_STATE_EMERGENCY     // 紧急停机（切断动力，需人工复位）
} SystemState_t;

// API
void SafetyMonitor_Init(void);
void SafetyMonitor_Task(void);          // 周期性调用（50ms）
SystemState_t SafetyMonitor_GetState(void);
bool SafetyMonitor_IsOperational(void);   // 业务层查询能否运动

#endif
