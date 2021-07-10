#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "rtc.h"
#include "nrf_drv_rtc.h"
#include "voltage.h"

const nrf_drv_rtc_t rtc = NRF_DRV_RTC_INSTANCE(2);

static void rtc_handler(nrf_drv_rtc_int_type_t int_type)
{
    if (int_type == NRF_DRV_RTC_INT_TICK)
    {
        dc = dc + 100;
        printf("tick: %d\n",dc);
    }
    if(Button == 1){
        Draw_length = dc;
        printf("Draw Length: %dms\n", Draw_length);
        if(Draw_length_total >= DLT_average){
            printf("Exceeding today's total\n");
            bsp_board_led_on(RED_LED);
            if(voltage_lock = 1){
                //Lock voltage
                printf("Voltage Lock Enabled\n");
            }
        }else if(Time_between <= Wait_period){
            printf("Wait longer to take a hit\n");
            bsp_board_led_on(RED_LED);
            if(voltage_lock = 1){
                //Lock voltage
                printf("Voltage Lock Enabled\n");
            }
        }else if(Draw_length >= Suggestion - 3){
            printf("Stop Suggested\n");
            bsp_board_led_on(BLUE_LED);
            if(auto_lock = 1){
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
                if(voltage_lock = 1){
                    //Lock voltage
                    printf("Voltage Lock Enabled\n");
                }
            }
        }
    }
}


void rtc_config(void)
{
    uint32_t err_code;

    //Initialize RTC instance
    nrf_drv_rtc_config_t config = NRF_DRV_RTC_DEFAULT_CONFIG;
    config.prescaler = 3276;//100ms RTCoutner
    err_code = nrf_drv_rtc_init(&rtc, &config, rtc_handler);
    APP_ERROR_CHECK(err_code);
}
