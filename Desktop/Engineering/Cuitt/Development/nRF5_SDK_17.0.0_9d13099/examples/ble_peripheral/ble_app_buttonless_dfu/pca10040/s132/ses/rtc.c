#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "rtc.h"
#include "nrf_drv_rtc.h"
#include "voltage.h"
#include "sleep.c"

//const nrf_drv_rtc_t timeout = NRF_DRV_RTC_INSTANCE(0);
const nrf_drv_rtc_t rtc = NRF_DRV_RTC_INSTANCE(2);
bool sleep = false;
bool from_wake = true;
int stat_press_count = 0;

static void rtc_handler(nrf_drv_rtc_int_type_t int_type)
{
    if (int_type == NRF_DRV_RTC_INT_TICK)
    {
        dc = dc + 100;
        printf("tick: %d\n",dc);
    }

    //if draw button held, check whether limit exceeded
    if(Button == 1){
        Draw_length = dc;
        printf("Draw Length: %dms\n", Draw_length);
        if(Draw_length_total >= DLT_average){
            printf("Exceeding today's total\n");
            bsp_board_led_on(RED_LED);
            if(voltage_lock == 1){
                //Lock voltage
                printf("Voltage Lock Enabled\n");
            }
        }else if(Time_between <= Wait_period){
            printf("Wait longer to take a hit\n");
            bsp_board_led_on(RED_LED);
            if(voltage_lock == 1){
                //Lock voltage
                printf("Voltage Lock Enabled\n");
            }
        }else if(Draw_length >= Suggestion - 3){
            printf("Stop Suggested\n");
            bsp_board_led_on(BLUE_LED);
            if(auto_lock == 1){
                if(Draw_length >= Suggestion){
                    printf("Auto Suggestion Lock Enabled\n");
                    //Lock voltage
                    bsp_board_led_off(BLUE_LED);
                    bsp_board_led_on(RED_LED);
                }
            }
            else if(Draw_length >= Draw_length_average){
                printf("Draw length longer than average\n");
                bsp_board_led_off(BLUE_LED);
                bsp_board_led_on(RED_LED);
                if(voltage_lock == 1){
                    //Lock voltage
                    printf("Voltage Lock Enabled\n");
                }
            }
        }
    }
//    if(Button_b == 0 && dc >= 4000){
//        bsp_board_led_off(RED_LED);
//        bsp_board_led_off(BLUE_LED);
//        bsp_board_led_off(GREEN_LED);
//        printf("STATUS OFF\n");
//        nrf_drv_rtc_disable(&rtc);
//        nrf_drv_rtc_tick_disable(&rtc);
//        dc = 0;
//        stat_press_count = 0;
//    }
//    if(from_wake){
//        if(Draw_length_total >= DLT_average){
//            printf("DAILY TOTAL EXCEEDED\n");
//            bsp_board_led_on(RED_LED);
//        }
//        else if(Time_between <= Wait_period){
//            printf("WAIT PERIOD NOT == 0\n");
//            bsp_board_led_on(RED_LED);
//        }
//        else{
//            printf("DRAW PERMITTED\n");
//            bsp_board_led_on(BLUE_LED);
//        }
//        from_wake = false;
//    }
}

void rtc_config(void)
{
    uint32_t err_code;

    nrf_drv_rtc_config_t config = NRF_DRV_RTC_DEFAULT_CONFIG;
    config.prescaler = 3276;//100ms RTCoutner
    err_code = nrf_drv_rtc_init(&rtc, &config, rtc_handler);
}
