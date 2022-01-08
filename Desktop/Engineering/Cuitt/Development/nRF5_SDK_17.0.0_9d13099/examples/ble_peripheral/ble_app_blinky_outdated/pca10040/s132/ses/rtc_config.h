#include <stdint.h>
#include "nrf_drv_rtc.h"
#include "boards.h"

const nrf_drv_rtc_t rtc = NRF_DRV_RTC_INSTANCE(2); /**< Declaring an instance of nrf_drv_rtc for RTC0. */

#define CLK_LED                         BSP_BOARD_LED_3                         /**< LED for clock ticks>**/

#define RTC_ADDRESS          (0x68U)
uint8_t sec, min, hr, day, date, month, year;

volatile unsigned int dc = 0;//milliseconds
volatile unsigned char Button = 0;//button pressed value

static void rtc_handler(nrf_drv_rtc_int_type_t int_type)
{
    if (int_type == NRF_DRV_RTC_INT_TICK)
    {
        nrf_gpio_pin_toggle(CLK_LED);
        dc = dc + 100;
    }
}

static void lfclk_config(void)
{
    ret_code_t err_code = nrf_drv_clock_init();
    APP_ERROR_CHECK(err_code);

    nrf_drv_clock_lfclk_request(NULL);
}

static void rtc_config(void)
{
    uint32_t err_code;

    //Initialize RTC instance
    nrf_drv_rtc_config_t config = NRF_DRV_RTC_DEFAULT_CONFIG;
    config.prescaler = 3276;//100ms RTCoutner
    err_code = nrf_drv_rtc_init(&rtc, &config, rtc_handler);
    APP_ERROR_CHECK(err_code);
}