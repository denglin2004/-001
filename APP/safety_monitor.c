#include "safety_monitor.h"
#include "Timer.h"

static SystemState_t g_sys_state = SYS_STATE_INIT;
static uint32_t g_busoff_recover_tick = 0;

void SafetyMonitor_Task(void) 
{
    static uint32_t last_tick = 0;
    uint32_t now = GetTick();
    if ((now - last_tick) < SAFETY_MONITOR_PERIOD_MS) return;
    last_tick = now;
    
    bool all_heartbeat_ok = true;
    bool any_fatal = false;
    uint8_t fault_axis = 0xFF;
    
    // ========== 1. 从站心跳监控（核心） ==========

        // AxisDevice_t *ax = &g_axis[i];
        // // 检测心跳超时
        // if ((now - ax->last_heartbeat_tick) > HEARTBEAT_TIMEOUT_MS) 
        // {
        //     all_heartbeat_ok = false;
        //     ax->online = false;
            
        //     // 故障分级：单轴心跳丢失 → Error（单轴停机，系统降级）
        //     Fault_Report(FAULT_CAN_HB_LOST, FAULT_LEVEL_ERROR, ax->node_id);
            
        //     // 该轴进入安全态：位置保持，抱闸
        //     Axis_EnterSafeState(ax);
        // }
        
        // 检测从站已上报的故障（通过心跳中的health_word）
        if (ax->health_word & (HEALTH_STALL_RISK 
            | HEALTH_DRV_FAULT | HEALTH_BUS_OV)) 
        {
            any_fatal = true;
            fault_axis = ax->node_id;
            
            // 堵转：如果是丝杆卡死，立即冻结该轴
            if (ax->health_word & HEALTH_STALL_RISK) 
            {
                Fault_Report(FAULT_AXIS_STALL, FAULT_LEVEL_ERROR, ax->node_id);
                Axis_HoldPosition(ax);  // 保持当前位置，不再跟随路径
            }
        }

    
    // // ========== 2. CAN总线物理层监控 ==========
    // if (CAN_IsBusOff()) {
    //     all_heartbeat_ok = false;
    //     any_fatal = true;
        
    //     // BusOff：总线物理故障，可能是线缆断开、终端电阻丢失、强干扰
    //     Fault_Report(FAULT_CAN_BUSOFF, FAULT_LEVEL_FATAL, 0xFF);
        
    //     // 尝试自动恢复（IEC 61508要求：BusOff后必须尝试恢复，但限流）
    //     if ((now - g_busoff_recover_tick) > BUSOFF_RECOVER_MS) {
    //         CAN_BusOffRecovery();       // 重新初始化CAN控制器
    //         g_busoff_recover_tick = now;
    //     }
    // }
    
    // // 检测总线负载率（过高说明有节点狂发错误帧）
    // uint8_t bus_load = CAN_GetBusLoadPercent();
    // if (bus_load > 85) {
    //     Fault_Report(FAULT_MODULE_CAN | 0x03, FAULT_LEVEL_WARNING, 0xFF);
    // }
    
    // ========== 3. 主控自身健康检查 ==========
    // 栈溢出检测（CH32可用MPU或水位线）
    // if (Stack_GetHighWatermark() > STACK_DANGER_THRESHOLD) {
    //     Fault_Report(FAULT_MODULE_SYS | 0x01, FAULT_LEVEL_WARNING, 0xFF);
    // }
    
    // // 主控温度（如果有片内传感器）
    // if (GetMCUCoreTemp() > 85) {
    //     any_fatal = true;
    //     Fault_Report(FAULT_MODULE_SYS | 0x02, FAULT_LEVEL_FATAL, 0xFF);
    // }
    
    // ========== 4. 系统状态机切换 ==========
    switch (g_sys_state) {
       case SYS_STATE_INIT:
            if (any_fatal) {

            } else if (!all_heartbeat_ok) 
            {
           g_sys_state = SYS_STATE_OPERATIONAL;
            }
            break;
        case SYS_STATE_OPERATIONAL:
            if (any_fatal) {
                g_sys_state = SYS_STATE_SAFE_STOP;
                //Motion_EmergencyStop();     // 停止S路径

            } else if (!all_heartbeat_ok) {
                g_sys_state = SYS_STATE_DEGRADED;
                //Motion_PausePath();          // 暂停路径，等待恢复
            }
            break;
            
        case SYS_STATE_DEGRADED:
            if (all_heartbeat_ok) {
                // 故障恢复：所有从站重新在线
                g_sys_state = SYS_STATE_OPERATIONAL;
                Motion_ResumePath();
            }
            break;
            
        case SYS_STATE_SAFE_STOP:
            // 需人工确认后才能退出
            if (Fault_IsManualResetRequested() && all_heartbeat_ok) {
                g_sys_state = SYS_STATE_STANDBY;
            }
            break;
            
        default:
            break;
    }
    
    // // ========== 5. 喂狗（唯一出口） ==========
    // // 只有系统未进入Fatal/Emergency时才喂狗，否则让看门狗复位
    // if (g_sys_state != SYS_STATE_EMERGENCY && g_sys_state != SYS_STATE_SAFE_STOP) {
    //     WDG_Feed();
    // }
    // // 如果进入SAFE_STOP，故意不喂狗，让IWDG在1秒后硬复位系统（彻底恢复）
}
