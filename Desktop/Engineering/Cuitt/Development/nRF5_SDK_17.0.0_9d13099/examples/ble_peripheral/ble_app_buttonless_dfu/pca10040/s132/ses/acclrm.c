#include "acclrm.h"
#include "app_error.h"
#include "twi.h"

uint8_t acclrm_data[8] = {0x00};

void read_acclrm(void){
    printf("READING ACCELEROMETER\n");
    ret_code_t err_code;
    uint8_t start_register[1] = {0x00};
    err_code = nrf_drv_twi_tx(&twi, ACCLRM_ADDRESS, start_register, 1, false);
    APP_ERROR_CHECK(err_code);
    err_code = nrf_drv_twi_rx(&twi, ACCLRM_ADDRESS, acclrm_data, sizeof(acclrm_data));
    APP_ERROR_CHECK(err_code);
    printf("%d\n", acclrm_data);
}