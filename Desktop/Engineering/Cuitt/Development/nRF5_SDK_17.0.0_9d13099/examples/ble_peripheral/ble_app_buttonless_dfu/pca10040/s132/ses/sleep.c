#include <stdbool.h>
#include <stdint.h>
#include "app_error.h"
#include "nrf_log.h"
#include "sleep.h"

//PREVIOUSLY IN MAIN AFTER APPLICATION TIMERS START
/**@brief Function for putting the chip into sleep mode.
 *
 * @note This function will not return.
 */
 
static void sleep_mode_enter(void)
{   
    uint32_t err_code;
    NRF_LOG_INFO("Powering off.");
    //uint32_t err_code = bsp_indication_set(BSP_INDICATE_IDLE);

    APP_ERROR_CHECK(err_code);

    // Prepare wakeup buttons.
    err_code = bsp_btn_ble_sleep_mode_prepare();
    APP_ERROR_CHECK(err_code);

    //Disable SoftDevice. It is required to be able to write to GPREGRET2 register (SoftDevice API blocks it).
    //GPREGRET2 register holds the information about skipping CRC check on next boot.
    err_code = nrf_sdh_disable_request();
    APP_ERROR_CHECK(err_code);
}
