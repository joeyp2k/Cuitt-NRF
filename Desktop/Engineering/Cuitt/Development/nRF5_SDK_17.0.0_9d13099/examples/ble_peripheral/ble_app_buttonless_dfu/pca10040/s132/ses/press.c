#include "press.h"
#include "app_error.h"
#include "twi.h"

typedef long signed int BMP280_S32_t;
typedef long unsigned int BMP280_U32_t;

unsigned short dig_P1;
signed short dig_P2;
signed short dig_P3;
signed short dig_P4;
signed short dig_P5;
signed short dig_P6;
signed short dig_P7;
signed short dig_P8;
signed short dig_P9;

signed long uncomp_press;
signed long comp_press;

BMP280_S32_t t_fine;

void params(){
    uint8_t temp[BMP280_CALIB_DATA_SIZE] = { 0 };
    uint8_t start_reg[1] = {BMP280_DIG_T1_LSB_ADDR};

    ret_code_t err_code;

    err_code = nrf_drv_twi_tx(&twi, PRESS_ADDRESS, start_reg, 1, false);
    APP_ERROR_CHECK(err_code);
    err_code = nrf_drv_twi_rx(&twi, PRESS_ADDRESS, temp, sizeof(temp));
    APP_ERROR_CHECK(err_code);

            dig_P1 =
                (uint16_t) (((uint16_t) temp[BMP280_DIG_P1_MSB_POS] << 8) | ((uint16_t) temp[BMP280_DIG_P1_LSB_POS]));
            dig_P2 =
                (int16_t) (((int16_t) temp[BMP280_DIG_P2_MSB_POS] << 8) | ((int16_t) temp[BMP280_DIG_P2_LSB_POS]));
            dig_P3 =
                (int16_t) (((int16_t) temp[BMP280_DIG_P3_MSB_POS] << 8) | ((int16_t) temp[BMP280_DIG_P3_LSB_POS]));
            dig_P4 =
                (int16_t) (((int16_t) temp[BMP280_DIG_P4_MSB_POS] << 8) | ((int16_t) temp[BMP280_DIG_P4_LSB_POS]));
            dig_P5 =
                (int16_t) (((int16_t) temp[BMP280_DIG_P5_MSB_POS] << 8) | ((int16_t) temp[BMP280_DIG_P5_LSB_POS]));
            dig_P6 =
                (int16_t) (((int16_t) temp[BMP280_DIG_P6_MSB_POS] << 8) | ((int16_t) temp[BMP280_DIG_P6_LSB_POS]));
            dig_P7 =
                (int16_t) (((int16_t) temp[BMP280_DIG_P7_MSB_POS] << 8) | ((int16_t) temp[BMP280_DIG_P7_LSB_POS]));
            dig_P8 =
                (int16_t) (((int16_t) temp[BMP280_DIG_P8_MSB_POS] << 8) | ((int16_t) temp[BMP280_DIG_P8_LSB_POS]));
            dig_P9 =
                (int16_t) (((int16_t) temp[BMP280_DIG_P9_MSB_POS] << 8) | ((int16_t) temp[BMP280_DIG_P9_LSB_POS]));
}

void uncomp_data(){
    ret_code_t err_code;
    uint8_t start_reg[1] = {BMP280_PRES_MSB_ADDR};
    uint8_t press_data[3] = {0x00};

    err_code = nrf_drv_twi_tx(&twi, PRESS_ADDRESS, start_reg, 1, false);
    APP_ERROR_CHECK(err_code);
    err_code = nrf_drv_twi_rx(&twi, PRESS_ADDRESS, press_data, sizeof(press_data));
    APP_ERROR_CHECK(err_code);

    uncomp_press =
                (int32_t) ((((uint32_t) (press_data[0])) << 12) | (((uint32_t) (press_data[1])) << 4) | ((uint32_t) press_data[2] >> 4));
}

// Returns pressure in Pa as unsigned 32 bit integer. Output value of “96386” equals 96386 Pa = 963.86 hPa
BMP280_U32_t bmp280_compensate_P_int32(BMP280_S32_t adc_P)
{
    BMP280_S32_t var1, var2;
    BMP280_U32_t p;
    var1 = (((BMP280_S32_t)t_fine)>>1) - (BMP280_S32_t)64000;
    var2 = (((var1>>2) * (var1>>2)) >> 11 ) * ((BMP280_S32_t)dig_P6);
    var2 = var2 + ((var1*((BMP280_S32_t)dig_P5))<<1);
    var2 = (var2>>2)+(((BMP280_S32_t)dig_P4)<<16);
    var1 = (((dig_P3 * (((var1>>2) * (var1>>2)) >> 13 )) >> 3) + ((((BMP280_S32_t)dig_P2) * var1)>>1))>>18;
    var1 =((((32768+var1))*((BMP280_S32_t)dig_P1))>>15);
    if (var1 == 0)
    {
        return 0; // avoid exception caused by division by zero
    }
    p = (((BMP280_U32_t)(((BMP280_S32_t)1048576)-adc_P)-(var2>>12)))*3125;
    if (p < 0x80000000)
    {
        p = (p << 1) / ((BMP280_U32_t)var1);
    }
    else
    {
        p = (p / (BMP280_U32_t)var1) * 2;
    }
    var1 = (((BMP280_S32_t)dig_P9) * ((BMP280_S32_t)(((p>>3) * (p>>3))>>13)))>>12;
    var2 = (((BMP280_S32_t)(p>>2)) * ((BMP280_S32_t)dig_P8))>>13;
    p = (BMP280_U32_t)((BMP280_S32_t)p + ((var1 + var2 + dig_P7) >> 4));
    return p;
}

void read_press(){
    ret_code_t err_code;
    params();
    uncomp_data();
    printf("%d", uncomp_press);
    comp_press = bmp280_compensate_P_int32(uncomp_press);
    printf("%d", comp_press);
}