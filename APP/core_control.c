#include "core_control.h"
#include "DCCP_comand.h"
#include "ESP32_comand.h"
#include "can_app.h"
#include "bsp_can.h" 
#include "usart_app.h"
#include "usart_protocol.h"
#include <string.h>
#include "hal_rgb.h"
#include "Timer.h"


#include "SSD1306.h"
S_TaskRequest_t g_s_task_req={0};
void mian_task_run (void)
{
    DCCP_comand_process();
    ESP32_comand_process();       // 处理小智 + 云端指令
}

void onekey_task_run (void)
{

    Button_Control_Car();

    // ========== 消费统一任务请求：将 g_s_task_req 参数桥接到 grindcar_ctrl ==========
    if (g_s_task_req.trigger == 1)
    {
        g_s_task_req.trigger = 0;                     // 清除触发标志，防止重复触发

        grindcar_ctrl.Step_Total  = g_s_task_req.step_x;
        grindcar_ctrl.Loop_Total  = g_s_task_req.loop_y;
        grindcar_ctrl.foc_speed_set           = g_s_task_req.foc_speed;
        grindcar_ctrl.car_farward_speed_set   = g_s_task_req.car_speed;
        grindcar_ctrl.car_backward_speed_set  = g_s_task_req.car_speed;
        grindcar_ctrl.car_turn_speed_set      = g_s_task_req.car_speed;
        grindcar_ctrl.grind_down_set          = g_s_task_req.lift_high;

        grindcar_ctrl.task_S_cnt  = 1;                // 标记任务开始
        grindcar_ctrl.complete_step = 0;
        grindcar_ctrl.Loop_Finished = 0;
        memset(&grindcar_ctrl.car, 0, sizeof(grindcar_ctrl.car));
        memset(&grindcar_ctrl.foc, 0, sizeof(grindcar_ctrl.foc));
        memset(&grindcar_ctrl.grind, 0, sizeof(grindcar_ctrl.grind));
    }

    S_Comand_Control_Car();
 // TEST_Control_Car();
}





static void SendCommand (ActionType_t action, uint8_t Seq_id) {
    switch (action) {
    case ACTION_BUJING_UP: S_ComandTo_BuJing (3, 1, 1, grindcar_ctrl.grind_up_set, Seq_id); break;
    case ACTION_BUJING_DOWN: S_ComandTo_BuJing (3, 1, 0, grindcar_ctrl.grind_down_set, Seq_id); break;
    case ACTION_BUJING_GRIND: S_ComandTo_BuJing (2, 1, 0, 0000, Seq_id); break;
    case ACTION_CAR_FORWARD: S_ComandTo_Car (1, grindcar_ctrl.car_farward_speed_set, 1, Seq_id); break;
    case ACTION_CAR_BACKWARD: S_ComandTo_Car (2, grindcar_ctrl.car_backward_speed_set, 1, Seq_id); break;
    case ACTION_CAR_TURN_LEFT: S_ComandTo_Car (3, grindcar_ctrl.car_turn_speed_set, 1, Seq_id); break;
    case ACTION_CAR_STOP: S_ComandTo_Car (5, 0, 2, Seq_id); break;
    case ACTION_FOC_START: S_ComandTo_FOC (1, grindcar_ctrl.foc_speed_set, Seq_id); break;
    case ACTION_FOC_STOP: S_ComandTo_FOC (2, 0, Seq_id); break;
    default: break;
    }
}


static int8_t CheckResponse (ActionType_t action, uint32_t timeout_ms, uint32_t send_time)
{
    switch (action)
    {
    case ACTION_CAR_STOP:
        if (CAN2A0_Rec.sCAN.ack == 0x02)
            {
            for(uint8_t i = 0; i < 8; i++)
            {
                canrxbuf[0][i] =0;
            }
            return 1;
            }
        break;

    case ACTION_BUJING_UP:
            if (CAN2A1_Rec.sCAN.pzd_function_code == 0x01)
            {
            for(uint8_t i = 0; i < 8; i++)
            {
                canrxbuf[1][i] =0;
            }
            return 1;
            }
        break;
    case ACTION_BUJING_DOWN:
            if (CAN2A1_Rec.sCAN.pzd_function_code == 0x02)
            {
            for(uint8_t i = 0; i < 8; i++)
            {
                canrxbuf[1][i] =0;
            }
            return 1;
            }
        break;
    case ACTION_BUJING_GRIND:
        if (CAN2A1_Rec.sCAN.pzd_function_code == 0x03)
            {
            for(uint8_t i = 0; i < 8; i++)
            {
                canrxbuf[1][i] =0;
            }
            return 1;
            }
        break;

    case ACTION_FOC_START:
        if (CAN2A2_Rec.sCAN.FOC_Start == 0x01)
        {
            for(uint8_t i = 0; i < 8; i++)
            {
                canrxbuf[2][i] =0;
            }
        return 1;
        }

        break;

    case ACTION_FOC_STOP:
        if (CAN2A2_Rec.sCAN.FOC_Start == 0x02)
        {
            for(uint8_t i = 0; i < 8; i++)
            {
                canrxbuf[2][i] =0;
            }
        return 1;
        }
        break;

    default: break;
    }

    if ((GetTick() - send_time) >= timeout_ms) {
        return -1;
    }
    return 0;
}

S_Comand_Ctrl_Params_t grindcar_ctrl = {0};
static MainState_t g_main_state = STATE_IDLE;
static SubState_t g_sub_state = SUBSTATE_NONE;

void S_Comand_Control_Car (void) 
{

    if (grindcar_ctrl.task_S_cnt == 2)
        return;

    if (grindcar_ctrl.Step_Total == 0 
     || grindcar_ctrl.Loop_Total == 0)
    {
       Drv_RGB_SetColor(RGB_COLOR_RED);
       g_main_state = STATE_IDLE;
       return;
    }

    switch (g_main_state) 
    {

    // ===================== 空闲状�? =====================
    case STATE_IDLE:
    //状态机参数初�?�化
        grindcar_ctrl.car.Forward_Count = 0;
        grindcar_ctrl.car.Backward_Count = 0;
        grindcar_ctrl.car.is_forward_phase = 1;
        grindcar_ctrl.car.stop_sent = 0;
        grindcar_ctrl.foc.foc_sent = 0;
        grindcar_ctrl.grind.grind_sent = 0;
        grindcar_ctrl.complete_step = 0;
        grindcar_ctrl.Loop_Finished = 0;
        g_main_state = STATE_PREPARE;
        g_sub_state = SUBSTATE_SEND_CMD;
        Drv_RGB_SetColor(RGB_COLOR_YELLOW);
     //该状态机任务下各�?模块参数初�?�化
        grindcar_ctrl.foc_speed_set=g_dccp_temp.foc_speed;
        grindcar_ctrl.car_backward_speed_set=g_dccp_temp.car_speed;
        grindcar_ctrl.car_farward_speed_set=g_dccp_temp.car_speed;
        grindcar_ctrl.car_turn_speed_set=g_dccp_temp.car_speed;
        grindcar_ctrl.grind_down_set=g_dccp_temp.lift_high;
        break;

    // ===================== 准�?�状�????? =====================
    case STATE_PREPARE:
        switch (g_sub_state) 
        {
        case SUBSTATE_SEND_CMD:
            SendCommand (ACTION_BUJING_UP, 1);
            grindcar_ctrl.time_wait = GetTick();
            g_sub_state = SUBSTATE_WAIT_ACK;
            break;

        case SUBSTATE_WAIT_ACK:
        {
            int8_t result = CheckResponse (ACTION_BUJING_UP, 80000, grindcar_ctrl.time_wait);
            if (result == 1 || result == -1)
            {
                g_sub_state = SUBSTATE_WAIT_TIME;
                grindcar_ctrl.time_wait = GetTick();
            }
            else if((GetTick()- grindcar_ctrl.time_wait)>=2000)
            {
            g_sub_state = SUBSTATE_SEND_CMD;  
            }

        }break;

        case SUBSTATE_WAIT_TIME:
            if ((GetTick() - grindcar_ctrl.time_wait) >= 2000) 
            {
                grindcar_ctrl.car.is_forward_phase = 1;
                g_sub_state = SUBSTATE_SEND_CMD;
                g_main_state = STATE_WORKING;
            }
            break;

        default:
            g_sub_state = SUBSTATE_SEND_CMD;
            break;
        }
        break;

    // ===================== 工作状�? =====================
    case STATE_WORKING:
        switch (g_sub_state) {
        case SUBSTATE_SEND_CMD:
            SendCommand (ACTION_FOC_STOP, 2);
            grindcar_ctrl.time_wait = GetTick();
            g_sub_state = SUBSTATE_WAIT_ACK;
            break;

        case SUBSTATE_WAIT_ACK: {
            int8_t result = CheckResponse (ACTION_FOC_STOP, 80000, grindcar_ctrl.time_wait);
            if (result == 1 || result == -1) 
            {
                g_sub_state = SUBSTATE_WAIT_TIME;
                grindcar_ctrl.time_wait = GetTick();
            }
            else if((GetTick()- grindcar_ctrl.time_wait)>=2000)
            {
             g_sub_state = SUBSTATE_SEND_CMD; 
            }
        } break;

        case SUBSTATE_WAIT_TIME:
            if ((GetTick() - grindcar_ctrl.time_wait) >= 2000)
            {
                grindcar_ctrl.car.stop_sent = 0;
                g_sub_state = grindcar_ctrl.car.is_forward_phase ? SUBSTATE_MOVE_FORWARD
                                                                 : SUBSTATE_MOVE_BACKWARD;
            }
            break;

        case SUBSTATE_MOVE_FORWARD:
        {
            static uint8_t fwd_init = 0;
            if (!fwd_init) {
                SendCommand (ACTION_CAR_FORWARD, 3);
                Drv_RGB_SetColor(RGB_COLOR_BLUE);
                grindcar_ctrl.car.time_move = GetTick();
                fwd_init = 1;
            }

            if ((GetTick() - grindcar_ctrl.car.time_move) >= S_foward_time) 
            {
                fwd_init = 0;
                grindcar_ctrl.car.stop_sent = 0;
                g_sub_state = SUBSTATE_STOP;
            }
        } break;

        case SUBSTATE_MOVE_BACKWARD: {
            static uint8_t bwd_init = 0;

            if (!bwd_init) 
            {
                SendCommand(ACTION_CAR_BACKWARD, 4);
                Drv_RGB_SetColor(RGB_COLOR_GREEN);
                grindcar_ctrl.car.time_move = GetTick();
                bwd_init = 1;
            }

            if ((GetTick() - grindcar_ctrl.car.time_move) >=  S_backward_time) 
            {
                bwd_init = 0;
                grindcar_ctrl.car.stop_sent = 0;
                g_sub_state = SUBSTATE_STOP;
            }
        } break;

        case SUBSTATE_STOP:
            if (!grindcar_ctrl.car.stop_sent) {
                SendCommand (ACTION_CAR_STOP, 5);
                Drv_RGB_SetColor(RGB_COLOR_RED);
                grindcar_ctrl.car.time_stop = GetTick();
                grindcar_ctrl.car.stop_sent = 1;
            }
            {
                int8_t result = CheckResponse (ACTION_CAR_STOP, 80000, grindcar_ctrl.car.time_stop);
                if (result == 1 || result == -1) 
                {
                    grindcar_ctrl.car.stop_sent = 0;
                    grindcar_ctrl.foc.foc_sent = 0;
                    g_sub_state = SUBSTATE_FOC_START;
                }
               else if((GetTick()-grindcar_ctrl.car.time_stop)>=2000)
               {
               g_sub_state = SUBSTATE_STOP; 
               }
            }
            break;

        case SUBSTATE_FOC_START: {
            static uint8_t foc_init = 0;

            if (!foc_init) {
                SendCommand (ACTION_FOC_START, 6);
                grindcar_ctrl.foc.time_foc = GetTick();
                foc_init = 1;
            }

            int8_t result = CheckResponse (ACTION_FOC_START, 80000, grindcar_ctrl.foc.time_foc);

            if (result == 1 || result == -1)
            {
                foc_init = 0;
                grindcar_ctrl.grind.grind_sent = 0;
                g_sub_state = SUBSTATE_GRINDING;
            }
             else if((GetTick()-grindcar_ctrl.foc.time_foc)>=2000)
            {
            g_sub_state =SUBSTATE_FOC_START;
            foc_init = 0; 
            }
        } break;

        case SUBSTATE_GRINDING:
            if (!grindcar_ctrl.grind.grind_sent)
            {
                SendCommand (ACTION_BUJING_GRIND, 7);
                grindcar_ctrl.grind.time_grind = GetTick();
                grindcar_ctrl.grind.grind_sent = 1;
            }

            {
                int8_t result = CheckResponse (ACTION_BUJING_GRIND, 80000, grindcar_ctrl.grind.time_grind);
                if (result == 1 || result == -1)
                {
                    grindcar_ctrl.grind.grind_sent = 0;

                    if (grindcar_ctrl.car.is_forward_phase)
                    {
                        grindcar_ctrl.car.Forward_Count++;
                    }
                    else
                    {
                        grindcar_ctrl.car.Backward_Count++;
                    }

                    if ((grindcar_ctrl.car.is_forward_phase && grindcar_ctrl.car.Forward_Count >= grindcar_ctrl.Step_Total) ||
                       (!grindcar_ctrl.car.is_forward_phase && grindcar_ctrl.car.Backward_Count >= grindcar_ctrl.Loop_Total))
                    {
                        g_main_state = STATE_TURNING;
                        g_sub_state = SUBSTATE_SEND_CMD;
                    }
                    else
                    {
                        g_sub_state = SUBSTATE_SEND_CMD;
                    }
                }
               else if((GetTick()- grindcar_ctrl.grind.time_grind)>=2000)
               {
               g_sub_state =SUBSTATE_GRINDING;
               grindcar_ctrl.grind.grind_sent = 0; 
               }
            }
            break;

        default:
            g_sub_state = SUBSTATE_SEND_CMD;
            break;
        }
        break;

    // ===================== �????向状�????? =====================
    case STATE_TURNING:
        switch (g_sub_state) {
        case SUBSTATE_SEND_CMD:
        {
            static uint8_t turn_init = 0;

            if (!turn_init) {
                SendCommand (ACTION_CAR_TURN_LEFT, 8);
                grindcar_ctrl.car.time_turn = GetTick();
                turn_init = 1;
            }

            if ((GetTick() - grindcar_ctrl.car.time_turn) >= 500) {
                turn_init = 0;
                grindcar_ctrl.car.stop_sent = 0;
                g_sub_state = SUBSTATE_STOP;
            }
        } break;

        case SUBSTATE_STOP:
            if (!grindcar_ctrl.car.stop_sent) {
                SendCommand (ACTION_CAR_STOP, 5);
                grindcar_ctrl.car.time_stop = GetTick();
                grindcar_ctrl.car.stop_sent = 1;
            }
            {
                int8_t result = CheckResponse (ACTION_CAR_STOP, 80000, grindcar_ctrl.car.time_stop);
                if (result == 1 || result == -1)
                {
                    grindcar_ctrl.car.stop_sent = 0;
                    if (grindcar_ctrl.car.is_forward_phase)
                    {
                        grindcar_ctrl.car.is_forward_phase = 0;
                        g_main_state = STATE_WORKING;
                        g_sub_state = SUBSTATE_SEND_CMD;
                    }
                    else
                    {
                        grindcar_ctrl.Loop_Finished++;
                        grindcar_ctrl.car.is_forward_phase = 1;
                        grindcar_ctrl.car.Forward_Count = 0;
                        grindcar_ctrl.car.Backward_Count = 0;

                        if (grindcar_ctrl.Loop_Finished < grindcar_ctrl.Loop_Total)
                        {
                          g_main_state = STATE_WORKING;
                          g_sub_state = SUBSTATE_SEND_CMD;
                        }
                        else
                        {
                            grindcar_ctrl.complete_step = 0;
                            g_main_state = STATE_COMPLETE;
                        }
                    }
                }
                else if((GetTick()- grindcar_ctrl.car.time_stop)>=2000)
               {
               g_sub_state =SUBSTATE_STOP; 
               grindcar_ctrl.car.stop_sent=0;
               }
            }
            break;

        default:
            g_sub_state = SUBSTATE_SEND_CMD;
            break;
        }
        break;

    // ===================== 完成状�? =====================
    case STATE_COMPLETE:
        switch (grindcar_ctrl.complete_step)
      {
          case 0:
              SendCommand (ACTION_CAR_STOP, 10);
              grindcar_ctrl.car.time_stop = GetTick();
              grindcar_ctrl.complete_step = 1;
              break;

          case 1: {
              int8_t result = CheckResponse (ACTION_CAR_STOP, 80000, grindcar_ctrl.car.time_stop);
              if (result == 1 || result == -1) {
                  grindcar_ctrl.complete_step = 2;
              }
          } break;

          case 2:
              SendCommand (ACTION_FOC_STOP, 11);
              grindcar_ctrl.foc.time_foc = GetTick();
              grindcar_ctrl.complete_step = 3;
              break;

          case 3:
          {
              int8_t result = CheckResponse (ACTION_FOC_STOP, 80000, grindcar_ctrl.foc.time_foc);
              if (result == 1 || result == -1) {
                  grindcar_ctrl.complete_step = 0;
                  g_main_state = STATE_IDLE;
                  grindcar_ctrl.task_S_cnt = 2;
              }
              else if((GetTick()- grindcar_ctrl.foc.time_foc)>=2000)
              {
              grindcar_ctrl.complete_step = 2;
              }
          } break;
        }
        break;
    }
}

// =================== OLED状态显�???? ===================

static const char *GetMainStateName (MainState_t state) 
{
    switch (state) {
    case STATE_IDLE: return "IDLE";
    case STATE_PREPARE: return "PREPARE";
    case STATE_WORKING: return "WORKING";
    case STATE_TURNING: return "TURNING";
    case STATE_COMPLETE: return "COMPLETE";
    default: return "UNKNOWN";
    }
}

static const char *GetSubStateName (SubState_t state) 
{
    switch (state) {
    case SUBSTATE_NONE: return "NONE";
    case SUBSTATE_SEND_CMD: return "SEND";
    case SUBSTATE_WAIT_ACK: return "WAIT_ACK";
    case SUBSTATE_WAIT_TIME: return "WAIT_T";
    case SUBSTATE_MOVE_FORWARD: return "FORWARD";
    case SUBSTATE_MOVE_BACKWARD: return "BACK";
    case SUBSTATE_STOP: return "STOP";
    case SUBSTATE_GRINDING: return "GRIND";
    case SUBSTATE_FOC_START: return "FOC_ON";
    case SUBSTATE_FOC_STOP: return "FOC_OFF";
    default: return "UNKNOWN";
    }
}

void OLED_DisplayStatus (uint8_t line) {
     char buf[16];

    sprintf (buf, "%s|%s",
             GetMainStateName (g_main_state),
             GetSubStateName (g_sub_state));
    SSD1306_ShowStr (0, line, buf, 8, 0);

    sprintf (buf, "F%3d B%3d L%3d",
             grindcar_ctrl.car.Forward_Count,
             grindcar_ctrl.car.Backward_Count,
             grindcar_ctrl.Loop_Finished);
    SSD1306_ShowStr (0, line + 2, buf, 8, 0);

    sprintf (buf, "S%3d L%3d",
             grindcar_ctrl.Step_Total,
             grindcar_ctrl.Loop_Total);
    SSD1306_ShowStr (0, line + 4, buf, 8, 0);
}
