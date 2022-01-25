#include <stdint.h>
#include "twi.h"
#include "rtclk.h"

//Convert BCD in the real time clock to decimal

uint8_t rtclk_data[8] = {0x00};

uint8_t bcdToDec(uint8_t val)
{ 
    return( (val/16*10) + (val%16) );
}

uint8_t decToBcd(uint8_t val)
{   
    unsigned int ones = 0;
    unsigned int tens = 0;
    unsigned int temp = 0;

    ones = val%10;
    temp = val/10;
    tens = temp<<4;
    return (tens + ones);
}

void init_rtclk_alarm(){
    ret_code_t err_code;
    uint8_t setting_register[3] = {0x00, 0x00, 0x00};
    uint8_t setting_value[3] = {0x00, 0x00, 0x00};
    uint8_t setting_buffer[2] = {0x00};

    uint8_t start_reg = 0x00;
    for(int i = 0; i < 3; i++){
        setting_buffer[0] = setting_buffer[i];
        setting_buffer[1] = setting_value[i];
        err_code = nrf_drv_twi_tx(&twi, RTCLK_ADDRESS, setting_buffer, sizeof(setting_buffer), false);
        APP_ERROR_CHECK(err_code);  
    }
}

void utcToRTClk(uint64_t utc){
    ret_code_t err_code;
    
    //get seconds, minutes, hours, days, months, and years from utc

    rtclk_data[0] = utc % 60;
    rtclk_data[1] = utc % (60*60) / 60;
    rtclk_data[2] = utc % (24*60*60) / (60*60);
    rtclk_data[3];
    rtclk_data[4] = utc % 2629800 / (24*60*60);
    rtclk_data[5] = (utc % (int) 3.154e+7 / 2629800) + 1;//+1 for RTClk month register range 1-12
    rtclk_data[6] = utc / (int) 3.154e+7;
    
    //decimal to bcd
    
    for(int i = 0; i < 7; i++){
        printf("0x%02x %d\n", i, rtclk_data[i]);
    }
    printf("\n");
    for(int i = 0; i < 7; i++){
        rtclk_data[i] = decToBcd(rtclk_data[i]);
        printf("0x%02x %d\n", i, rtclk_data[i]);
    }

    //update registers with new time
    //uint8_t start_reg = 0x00;
    //err_code = nrf_drv_twi_tx(&twi, RTCLK_ADDRESS, &start_reg, rtclk_data, false);
}