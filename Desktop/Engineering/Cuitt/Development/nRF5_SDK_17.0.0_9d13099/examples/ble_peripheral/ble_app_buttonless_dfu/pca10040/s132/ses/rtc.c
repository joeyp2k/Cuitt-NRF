#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "rtc.h"
#include "nrf_drv_rtc.h"
#include "voltage.h"

const nrf_drv_rtc_t rtc = NRF_DRV_RTC_INSTANCE(2);
bool sleep = false;
bool from_wake = true;
int stat_press_count = 0;

static void rtc_handler(nrf_drv_rtc_int_type_t int_type)
{
    if (int_type == NRF_DRV_RTC_INT_TICK)
    {
        dc = dc + 1;
        //printf("tick: %d\n",dc);
    }

    //if draw button held, check whether limit exceeded
    if(Button == 1){
        draw_length = dc;
        //printf("Draw Length: %dms\n", draw_length);
        if(draw_length_total >= draw_length_total_average){
            printf("Exceeding today's total\n");
            bsp_board_led_on(RED_LED);
            if(voltage_lock == 1){
                //Lock voltage
                printf("Voltage Lock Enabled\n");
            }
        }else if(time_between <= wait_period){
            printf("Wait longer to take a hit\n");
            bsp_board_led_on(RED_LED);
            if(voltage_lock == 1){
                //Lock voltage
                printf("Voltage Lock Enabled\n");
            }
        }else if(draw_length >= suggestion - 3){
            printf("Stop Suggested\n");
            bsp_board_led_on(BLUE_LED);
            if(auto_lock == 1){
                if(draw_length >= suggestion){
                    printf("Auto Suggestion Lock Enabled\n");
                    //Lock voltage
                    bsp_board_led_off(BLUE_LED);
                    bsp_board_led_on(RED_LED);
                }
            }
            else if(draw_length >= draw_length_average){
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
}

void rtc_config(void)
{
    uint32_t err_code;

    nrf_drv_rtc_config_t config = NRF_DRV_RTC_DEFAULT_CONFIG;
    config.prescaler = 3276;//100ms RTCoutner
    err_code = nrf_drv_rtc_init(&rtc, &config, rtc_handler);
}
