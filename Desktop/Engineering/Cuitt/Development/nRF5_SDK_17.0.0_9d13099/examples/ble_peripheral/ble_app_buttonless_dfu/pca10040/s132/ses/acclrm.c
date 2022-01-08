#include "acclrm.h"
#include "app_error.h"
#include "twi.h"

void read_acclrm(void){
    uint8_t acclrm_data[8] = {0x00};

    ret_code_t err_code;

    printf("CONFIGURING ACCELEROMETER\n");

    uint8_t config_register[3] = {0x20, 0x24, 0x2E};
    uint8_t config_value[3] = {0x87, 0x40, 0x80};
    uint8_t config_buffer[2] = {0x00};
    for(int i = 0; i < 3; i++){
        config_buffer[0] = config_register[i];
        config_buffer[1] = config_value[i];
        err_code = nrf_drv_twi_tx(&twi, ACCLRM_ADDRESS, config_buffer, sizeof(config_buffer), false);
        APP_ERROR_CHECK(err_code);  
    }

    printf("READING ACCELEROMETER\n");
    uint8_t start_register[3] = {0x29, 0x2B, 0x2D};
    //displaying zeros because of 
    for(int i = 0; i < 3; i++){
        err_code = nrf_drv_twi_tx(&twi, ACCLRM_ADDRESS, &start_register[i], 1, false);
        APP_ERROR_CHECK(err_code);
        err_code = nrf_drv_twi_rx(&twi, ACCLRM_ADDRESS, acclrm_data, sizeof(acclrm_data));
        APP_ERROR_CHECK(err_code);
        for(int i = 0; i < 1; i++){
            printf("%d\n", acclrm_data[i]);
        }
    }
}

//bool shake_detect(){
//    uint_8t x_threshold = 30;
//    uint_8t y_threshold = 30;
//    uint_8t z_threshold = 30;
//
//    uint_8t x_value;
//    uint_8t y_value;
//    uint_8t z_value;
//
//    uint_8t x_value_b;
//    uint_8t y_value_b;
//    uint_8t z_value_b;
//
//    uint_8t x_diff;
//    uint_8t y_diff;
//    uint_8t z_diff;
//
//    //record at two different points
//
//    x_diff = x_value_b - x_value;
//    y_diff = y_value_b - y_value;
//    z_diff = z_value_b - z_value;
//
//    if(x_diff > x_threshold || y_diff > y_threshold || z_diff > z_threshold){
//        return true;
//    }else{
//        return false;
//    }
//}
//
//bool shaking = false;
//
//void shake_to_advertise_enable(){
//    shake_advertising = true;
//    while(shaking){
//        if(dc >= 5000){
//            printf("enable advertising");
//            //enable advertising
//        }
//    }
//}