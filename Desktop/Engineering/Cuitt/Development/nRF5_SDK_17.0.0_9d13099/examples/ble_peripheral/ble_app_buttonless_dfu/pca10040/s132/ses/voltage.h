#include "app_timer.h"
#include "bsp.h"
#include "ble_cus.h"

#define BUTTON_DETECTION_DELAY          APP_TIMER_TICKS(50)   

#define DRAW_BUTTON                BSP_BUTTON_0    
#define STAT_BUTTON                BSP_BUTTON_1
#define PRESS_BUTTON               BSP_BUTTON_2  
#define ACCLRM_BUTTON              BSP_BUTTON_3  

#define BLUE_LED                        BSP_BOARD_LED_0                         /**< Is on when device is advertising. */
#define RED_LED                         BSP_BOARD_LED_1                         /**< Is on when device has connected. */
#define GREEN_LED                       BSP_BOARD_LED_2   

BLE_CUS_DEF(m_cus);                          /**< Handle of the current connection. */

extern uint16_t Draw_length;
extern uint16_t Draw_length_average;
extern uint16_t Draw_length_total;
extern uint16_t DLT_average;
extern unsigned int Suggestion;
extern uint32_t Wait_period;
extern uint32_t Time_between;
extern bool voltage_lock;
extern bool auto_lock;
extern bool connected;
extern volatile unsigned int dc;
extern volatile unsigned char Button;
extern volatile unsigned char Button_b;
extern uint16_t m_conn_handle;  

void buttons_init(void);