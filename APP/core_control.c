#include "core_control.h"
#include "DCCP_comand.h"
#include "ESP32_comand.h"
#include "can_app.h"
#include "usart_app.h"
#include "usart_protocol.h"
#include <string.h>
#include "hal_rgb.h"
#include "Timer.h"

void mian_task_run (void)
{

   DCCP_comand_process()  ;
   Button_Control_Car();
   //S_Comand_Control_Car();
   ESP32_comand_process()  ;

}


// 静态结构体实例（替代所有零散全局变量）
Car_Ctrl_Params_t car_ctrl = {
    .Forward_Count  = 0,
    .Backward_Count = 0,
    .Loop_Finished  = 0,
    .task_S_cnt     = 0,
};

// 状态机全局变量（保留必要状态）
static MainState_t g_main_state = STATE_IDLE;
static SubState_t g_sub_state = SUBSTATE_NONE;
static ActionType_t g_current_action = ACTION_NONE;

// =================== 动作执行函数（计时已修复） ===================
static int8_t ExecuteAction(ActionType_t action, uint32_t timeout_ms, uint8_t Seq_id)
{
    if (g_current_action != action) {
        g_current_action = action;
        car_ctrl.action_start_time = GetTick();  // 结构体成员

        switch (action) {
            case ACTION_BUJING_UP:          S_ComandTo_BuJing(3, 1,1,16,0,Seq_id); break;
            case ACTION_BUJING_DOWN:        S_ComandTo_BuJing(3, 1,0,16,0,Seq_id); break;
            case ACTION_BUJING_GRIND:       S_ComandTo_BuJing(2, 1,0,0,0,Seq_id); break;
            case ACTION_CAR_FORWARD:        S_ComandTo_Car(1, 40, 1, Seq_id); break;
            case ACTION_CAR_BACKWARD:       S_ComandTo_Car(2, 40, 1, Seq_id); break;
            case ACTION_CAR_TURN_LEFT:      S_ComandTo_Car(3, 50, 1, Seq_id); break;
            case ACTION_CAR_STOP:           S_ComandTo_Car(5, 0, 2, Seq_id); break;						
            case ACTION_FOC_START:          S_ComandTo_FOC(1, 255, Seq_id);   break;						
            case ACTION_FOC_STOP:           S_ComandTo_FOC(2, 180, Seq_id);    break;
            default: break;
        }
        return 0;
    }

    uint32_t current_time = GetTick();
    uint32_t elapsed_time = current_time - car_ctrl.action_start_time;  // 结构体成员

    // 等待CAN应答
    switch (action) {
				case ACTION_CAR_STOP:  if(CAN2A0_Rec.sCAN.ack==0x02) {g_current_action=ACTION_NONE; return 1;} break;
				
        case ACTION_BUJING_UP: case ACTION_BUJING_DOWN: case ACTION_BUJING_GRIND:
            if (CAN2A1_Rec.sCAN.pzd_function_code != 0x08&&CAN2A1_Rec.sCAN.pzd_function_code !=0x00) 
            {g_current_action=ACTION_NONE; return 1;} break;

        case ACTION_FOC_START: if(CAN2A2_Rec.sCAN.FOC_Start==0x01) {g_current_action=ACTION_NONE; return 1;} break;
        case ACTION_FOC_STOP:  if(CAN2A2_Rec.sCAN.FOC_Start==0x02) {g_current_action=ACTION_NONE; return 1;} break;
        default: break;
    }

    if (elapsed_time >= timeout_ms) {g_current_action=ACTION_NONE; return -1;}
    return 0;
}
//=================== 核心状态机（严格：1个case=1个ExecuteAction） ===================
void S_Comand_Control_Car(void)
{
   if (car_ctrl.task_S_cnt == 1)
    return;

    switch (g_main_state) {
        case STATE_IDLE:
            car_ctrl.Step_Total =3;
            car_ctrl.Loop_Total =3;
            car_ctrl.Forward_Count = 0;
            car_ctrl.Backward_Count = 0;
            car_ctrl.Loop_Finished = 0;
            g_main_state = STATE_PREPARE;
            g_sub_state = SUBSTATE_NONE;
            break;

        case STATE_PREPARE:
            switch (g_sub_state) {
                case SUBSTATE_NONE:
                    if (ExecuteAction(ACTION_BUJING_UP, 2000,1) == 1) 
										{
                        g_sub_state = SUBSTATE_WAITING;
                        car_ctrl.task_start_time = GetTick();
                    }
                    break;
                case SUBSTATE_WAITING:
                    if ((GetTick() - car_ctrl.task_start_time) >= 2000) {
											
                        g_sub_state = SUBSTATE_NONE;
                        g_main_state = STATE_WORKING;
                    }
                    break;
                default: break;
            }
            break;

        case STATE_WORKING:  
            switch (g_sub_state) {
                case SUBSTATE_NONE:									 
                    if (ExecuteAction(ACTION_FOC_STOP, 3000, 2) == 1) 
					{      
										
                    car_ctrl.task_start_time = GetTick();
					g_sub_state = SUBSTATE_WAITING;

                    }
                    break;

                case SUBSTATE_WAITING :
                    if ((GetTick() - car_ctrl.task_start_time) >= 2000) 
									{
                        g_sub_state = (car_ctrl.Forward_Count < car_ctrl.Step_Total)
											 ? SUBSTATE_MOVE_FORWARD : SUBSTATE_MOVE_BACKWARD;
                        car_ctrl.task_start_time =GetTick();
                  }
                    break;

                case SUBSTATE_MOVE_FORWARD:
                    ExecuteAction(ACTION_CAR_FORWARD, 3000, 2);
                    if ((GetTick() - car_ctrl.task_start_time) >= 500) 
										{
                        g_sub_state = SUBSTATE_STOP;
                        car_ctrl.task_start_time =GetTick();
                    }
                    break;

                case SUBSTATE_MOVE_BACKWARD:
                    ExecuteAction(ACTION_CAR_BACKWARD, 3000, 4);
                    if ((GetTick() - car_ctrl.task_start_time) >= 500) 
										{
                        g_sub_state = SUBSTATE_STOP;
                        car_ctrl.task_start_time = GetTick();
                    }
                    break;

                case SUBSTATE_STOP:
                    if (ExecuteAction(ACTION_CAR_STOP, 3000, 3) == 1) {
                        g_sub_state = SUBSTATE_FOC_START;
                    }
                    break;

                case SUBSTATE_FOC_START:
                    if (ExecuteAction(ACTION_FOC_START, 3000, 6) == 1) 
										{
                       
											 if ((GetTick() - car_ctrl.task_start_time) >= 2000)
												  g_sub_state = SUBSTATE_GRINDING;
                    }
                    break;

                case SUBSTATE_GRINDING:
                    if (ExecuteAction(ACTION_BUJING_GRIND, 3000, 7) == 1) {
                        (car_ctrl.Forward_Count < car_ctrl.Step_Total) ? 
											 car_ctrl.Forward_Count++ : car_ctrl.Backward_Count++;
                        g_sub_state = SUBSTATE_NONE;

                        if (car_ctrl.Forward_Count >= car_ctrl.Step_Total
													|| car_ctrl.Backward_Count >= car_ctrl.Step_Total) 
												{
                            g_main_state = STATE_TURNING;
                        }
                    }
                    break;

                default: break;
            }
            break;

        case STATE_TURNING:
            switch (g_sub_state) {
                case SUBSTATE_NONE:                   
                    ExecuteAction(ACTION_CAR_TURN_LEFT, 3000, 8);
                    g_sub_state = SUBSTATE_WAITING;
                    car_ctrl.task_start_time =GetTick();
                    break;

                case SUBSTATE_WAITING:
                    if ((GetTick() - car_ctrl.task_start_time) >= 800) 
										{
                        g_sub_state = SUBSTATE_STOP;
                    }
                    break;

                case SUBSTATE_STOP:
                    if (ExecuteAction(ACTION_CAR_STOP, 1000, 9) == 1) 
										{
                        if (car_ctrl.Forward_Count >= car_ctrl.Step_Total 
													&& car_ctrl.Backward_Count < car_ctrl.Step_Total) 
												{
                            g_main_state = STATE_WORKING;
                        } else if (car_ctrl.Backward_Count >= car_ctrl.Step_Total) 
												{
                            g_main_state = STATE_COMPLETE;
                        }
                        g_sub_state = SUBSTATE_NONE;
                    }
                    break;

                default: break;
            }
            break;
            
        case STATE_COMPLETE:
            car_ctrl.Loop_Finished++;
            if (car_ctrl.Loop_Finished < car_ctrl.Loop_Total) 
							{
                car_ctrl.Forward_Count = 0;
                car_ctrl.Backward_Count = 0;
                g_main_state = STATE_WORKING;
            } else {
                if (g_current_action == ACTION_NONE) {
                    ExecuteAction(ACTION_CAR_STOP, 1000, 10);
                } 
								else if (ExecuteAction(ACTION_CAR_STOP, 1000, 11)==1) 
								{
                    ExecuteAction(ACTION_FOC_STOP, 1000, 12);
                    g_main_state = STATE_IDLE;
                    car_ctrl.task_S_cnt = 1;
                }
            }
            break;
    }
}