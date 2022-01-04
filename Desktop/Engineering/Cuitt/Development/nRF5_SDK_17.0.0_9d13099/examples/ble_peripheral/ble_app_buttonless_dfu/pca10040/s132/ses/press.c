#include "sensor_data.c"
#include "app_error.h"
#include "stdio.h"
#include "twi.h"
#include "bmp2.h"
#include "common.h"

/*
signed long uncomp_temp;
signed long comp_temp;
signed long uncomp_press;
signed long comp_press;

void params(){
    uint8_t temp[BMP280_CALIB_DATA_SIZE] = { 0 };
    uint8_t start_reg[1] = {BMP280_DIG_T1_LSB_ADDR};

    ret_code_t err_code;

    err_code = nrf_drv_twi_tx(&twi, PRESS_ADDRESS, start_reg, 1, false);
    APP_ERROR_CHECK(err_code);
    err_code = nrf_drv_twi_rx(&twi, PRESS_ADDRESS, temp, sizeof(temp));
    APP_ERROR_CHECK(err_code);

    dig_T1 =
        (uint16_t) (((uint16_t) temp[BMP280_DIG_T1_MSB_POS] << 8) | ((uint16_t) temp[BMP280_DIG_T1_LSB_POS]));
    dig_T2 =
        (int16_t) (((int16_t) temp[BMP280_DIG_T2_MSB_POS] << 8) | ((int16_t) temp[BMP280_DIG_T2_LSB_POS]));
    dig_T3 =
        (int16_t) (((int16_t) temp[BMP280_DIG_T3_MSB_POS] << 8) | ((int16_t) temp[BMP280_DIG_T3_LSB_POS]));
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
    uint8_t temp[6] = {0x00};

    err_code = nrf_drv_twi_tx(&twi, PRESS_ADDRESS, start_reg, 1, false);
    APP_ERROR_CHECK(err_code);
    err_code = nrf_drv_twi_rx(&twi, PRESS_ADDRESS, temp, sizeof(temp));
    APP_ERROR_CHECK(err_code);

    uncomp_press =
                (int32_t) ((((uint32_t) (temp[0])) << 12) | (((uint32_t) (temp[1])) << 4) | ((uint32_t) temp[2] >> 4));
    uncomp_temp =
                (int32_t) ((((int32_t) (temp[3])) << 12) | (((int32_t) (temp[4])) << 4) | (((int32_t) (temp[5])) >> 4));
}

// Returns temperature in DegC, resolution is 0.01 DegC. Output value of “5123” equals 51.23 DegC

BMP280_S32_t bmp280_compensate_T_int32(BMP280_S32_t adc_T)
{
    BMP280_S32_t var1, var2, T;
    var1 = ((((adc_T>>3) - ((BMP280_S32_t)dig_T1<<1))) * ((BMP280_S32_t)dig_T2)) >> 11;
    var2 = (((((adc_T>>4) - ((BMP280_S32_t)dig_T1)) * ((adc_T>>4) - ((BMP280_S32_t)dig_T1))) >> 12) *
    ((BMP280_S32_t)dig_T3)) >> 14;
    t_fine = var1 + var2;
    T = (t_fine * 5 + 128) >> 8;
    return T;
}

// Returns pressure in Pa as unsigned 32 bit integer. Output value of “96386” equals 96386 Pa = 963.86 hPa
BMP280_U32_t bmp280_compensate_P_int64(BMP280_S32_t adc_P)
{
    BMP280_S64_t var1, var2, p;
    var1 = ((BMP280_S64_t)t_fine) - 128000;
    var2 = var1 * var1 * (BMP280_S64_t)dig_P6;
    var2 = var2 + ((var1*(BMP280_S64_t)dig_P5)<<17);
    var2 = var2 + (((BMP280_S64_t)dig_P4)<<35);
    var1 = ((var1 * var1 * (BMP280_S64_t)dig_P3)>>8) + ((var1 * (BMP280_S64_t)dig_P2)<<12);
    var1 = (((((BMP280_S64_t)1)<<47)+var1))*((BMP280_S64_t)dig_P1)>>33;
    if (var1 == 0)
    {
    return 0; // avoid exception caused by division by zero
    }
    p = 1048576-adc_P;
    p = (((p<<31)-var2)*3125)/var1;
    var1 = (((BMP280_S64_t)dig_P9) * (p>>13) * (p>>13)) >> 25;
    var2 = (((BMP280_S64_t)dig_P8) * p) >> 19;
    p = ((p + var1 + var2) >> 8) + (((BMP280_S64_t)dig_P7)<<4);
    return (BMP280_U32_t)p;
}


void read_press(void){
    printf("READING PRESSURE\n");
    ret_code_t err_code;
    params();
    uncomp_data();
    printf("UNCOMPENSATED PRESSURE: %d\n", uncomp_press);
    printf("UNCOMPENSATED TEMPERATURE: %d\n", uncomp_temp);
    comp_temp = bmp280_compensate_T_int32(uncomp_temp);
    printf("COMPENSATED TEMPERATURE: %d\n", comp_temp);
    comp_press = bmp280_compensate_P_int64(uncomp_press);
    printf("COMPENSATED PRESSURE: %d\n", comp_press);
}
*/

/******************************************************************************/
/*!         Static Function Declaration                                       */

/*!
 *  @brief This internal API is used to get compensated pressure and temperature data.
 *
 *  @param[in] period   : Contains the delay in microseconds
 *  @param[in] conf     : Structure instance of bmp2_config.
 *  @param[in] dev      : Structure instance of bmp2_dev.
 *
 *  @return Status of execution.
 */
static int8_t get_data(uint32_t period, struct bmp2_config *conf, struct bmp2_dev *dev);

void pressure_config(void)
{      
    int8_t rslt;
    uint32_t meas_time;
    struct bmp2_dev dev;
    struct bmp2_config conf;

    /* Interface selection is to be updated as parameter
     * For I2C :  BMP2_I2C_INTF
     * For SPI :  BMP2_SPI_INTF
     */
    rslt = bmp2_interface_selection(&dev, BMP2_I2C_INTF);
    bmp2_error_codes_print_result("bmp2_interface_selection", rslt);

    rslt = bmp2_init(&dev);
    bmp2_error_codes_print_result("bmp2_init", rslt);

    /* Always read the current settings before writing, especially when all the configuration is not modified */
    rslt = bmp2_get_config(&conf, &dev);
    bmp2_error_codes_print_result("bmp2_get_config", rslt);

    /* Configuring the over-sampling mode, filter coefficient and output data rate */
    /* Overwrite the desired settings */
    conf.filter = BMP2_FILTER_OFF;

    /* Over-sampling mode is set as high resolution i.e., os_pres = 8x and os_temp = 1x */
    conf.os_mode = BMP2_OS_MODE_HIGH_RESOLUTION;

    /* Setting the output data rate */
    conf.odr = BMP2_ODR_250_MS;

    rslt = bmp2_set_config(&conf, &dev);
    bmp2_error_codes_print_result("bmp2_set_config", rslt);

    /* Set normal power mode */
    rslt = bmp2_set_power_mode(BMP2_POWERMODE_NORMAL, &conf, &dev);
    bmp2_error_codes_print_result("bmp2_set_power_mode", rslt);

    /* Calculate measurement time in microseconds */
    rslt = bmp2_compute_meas_time(&meas_time, &conf, &dev);
    bmp2_error_codes_print_result("bmp2_compute_meas_time", rslt);

    /* Read pressure and temperature data */
    rslt = get_data(meas_time, &conf, &dev);
    bmp2_error_codes_print_result("get_data", rslt);

    bmp2_coines_deinit();

    return 0;
}

/*!
 *  @brief This internal API is used to get compensated pressure and temperature data.
 */
static int8_t get_data(uint32_t period, struct bmp2_config *conf, struct bmp2_dev *dev)
{
    int8_t rslt = BMP2_E_NULL_PTR;
    int8_t idx = 1;
    struct bmp2_status status;
    struct bmp2_data comp_data;

    printf("Measurement delay : %lu us\n", (long unsigned int)period);

    while (idx <= 50)
    {
        rslt = bmp2_get_status(&status, dev);
        bmp2_error_codes_print_result("bmp2_get_status", rslt);

        if (status.measuring == BMP2_MEAS_DONE)
        {
            /* Delay between measurements */
            dev->delay_us(period, dev->intf_ptr);

            /* Read compensated data */
            rslt = bmp2_get_sensor_data(&comp_data, dev);
            bmp2_error_codes_print_result("bmp2_get_sensor_data", rslt);

            #ifdef BMP2_64BIT_COMPENSATION
            comp_data.pressure = comp_data.pressure / 256;
            #endif

            #ifdef BMP2_DOUBLE_COMPENSATION
            printf("Data[%d]:    Temperature: %.4lf deg C	Pressure: %.4lf Pa\n",
                   idx,
                   comp_data.temperature,
                   comp_data.pressure);
            #else
            printf("Data[%d]:    Temperature: %ld deg C	Pressure: %lu Pa\n", idx, (long int)comp_data.temperature,
                   (long unsigned int)comp_data.pressure);
            #endif

            idx++;
        }
    }

    return rslt;
}