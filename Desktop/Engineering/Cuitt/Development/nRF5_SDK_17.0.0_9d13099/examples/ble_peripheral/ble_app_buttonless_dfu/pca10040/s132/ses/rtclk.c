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
    uint8_t setting_register[3] = {CONTROL_REGISTER, ALARM_1_HOURS, ALARM_1_DAYS};
    uint8_t setting_value[3] = {0x1D, 0xED, 0x80};
    uint8_t setting_buffer[2] = {0x00};

    uint8_t start_reg = 0x00;
    for(int i = 0; i < 2; i++){
        setting_buffer[0] = setting_register[i];
        setting_buffer[1] = setting_value[i];
        err_code = nrf_drv_twi_tx(&twi, RTCLK_ADDRESS, setting_buffer, sizeof(setting_buffer), false);
        APP_ERROR_CHECK(err_code);  
    }
}

void utcToRTClk(uint32_t utc){
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
    printf("DECIMAL\n");
    for(int i = 0; i < 7; i++){
        printf("0x%02x %d\n", i, rtclk_data[i]);
    }
    printf("BCD\n");
    
    //update registers with new time
    uint8_t start_reg = 0x00;
    uint8_t update_buffer[2];
    
    for(int i = 0; i < 7; i++){
        rtclk_data[i] = decToBcd(rtclk_data[i]);
        printf("0x%02x %d\n", i, rtclk_data[i]);
        update_buffer[0] = start_reg;
        update_buffer[1] = rtclk_data[i];
        err_code = nrf_drv_twi_tx(&twi, RTCLK_ADDRESS, update_buffer, sizeof(update_buffer), false);
        start_reg++;
    }
     
    start_reg = 0x00;
    //read for updated registers
    err_code = nrf_drv_twi_tx(&twi, RTCLK_ADDRESS, &start_reg, 1, false);
    APP_ERROR_CHECK(err_code);
    err_code = nrf_drv_twi_rx(&twi, RTCLK_ADDRESS, rtclk_data, sizeof(rtclk_data));
    printf("UPDATED REGISTERS (DEC)\n");
    for(int i = 0; i < 7; i++){
        rtclk_data[i] = bcdToDec(rtclk_data[i]);
        printf("0x%02x %d\n", i, rtclk_data[i]);
    }
}