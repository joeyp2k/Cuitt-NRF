#include <stdint.h>

#include "twi.h"
#include "nrf_drv_twi.h"
#include "bsp.h"



const nrf_drv_twi_t twi = NRF_DRV_TWI_INSTANCE(1);

void twi_init (void)
{
    ret_code_t err_code;

    const nrf_drv_twi_config_t twi_rtc_config = {
       .scl                = ARDUINO_SCL_PIN,
       .sda                = ARDUINO_SDA_PIN,
       .frequency          = NRF_DRV_TWI_FREQ_100K,
       .interrupt_priority = APP_IRQ_PRIORITY_HIGH,
       .clear_bus_init     = false
    };

    err_code = nrf_drv_twi_init(&twi, &twi_rtc_config, NULL, NULL);
    APP_ERROR_CHECK(err_code);

    nrf_drv_twi_enable(&twi);
}