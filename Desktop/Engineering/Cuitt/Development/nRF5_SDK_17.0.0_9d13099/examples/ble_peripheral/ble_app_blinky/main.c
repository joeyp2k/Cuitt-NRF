/**
 * Copyright (c) 2015 - 2020, Nordic Semiconductor ASA
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form, except as embedded into a Nordic
 *    Semiconductor ASA integrated circuit in a product or a software update for
 *    such product, must reproduce the above copyright notice, this list of
 *    conditions and the following disclaimer in the documentation and/or other
 *    materials provided with the distribution.
 *
 * 3. Neither the name of Nordic Semiconductor ASA nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * 4. This software, with or without modification, must only be used with a
 *    Nordic Semiconductor ASA integrated circuit.
 *
 * 5. Any software provided in binary form under this license must not be reverse
 *    engineered, decompiled, modified and/or disassembled.
 *
 * THIS SOFTWARE IS PROVIDED BY NORDIC SEMICONDUCTOR ASA "AS IS" AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY, NONINFRINGEMENT, AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL NORDIC SEMICONDUCTOR ASA OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */
/**
 * @brief Blinky Sample Application main file.
 *
 * This file contains the source code for a sample server application using the LED Button service.
 */

#include <stdint.h>
#include <string.h>
#include "nordic_common.h"
#include "nrf.h"
#include "app_error.h"
#include "ble.h"
#include "ble_err.h"
#include "ble_hci.h"
#include "ble_srv_common.h"
#include "ble_advdata.h"
#include "ble_conn_params.h"
#include "nrf_sdh.h"
#include "nrf_sdh_ble.h"
#include "boards.h"
#include "app_timer.h"
#include "app_button.h"
#include "ble_lbs.h"
#include "nrf_ble_gatt.h"
#include "nrf_ble_qwr.h"
#include "nrf_pwr_mgmt.h"
#include "nrf_drv_rtc.h"
#include "nrf_drv_clock.h"
#include "nrf_drv_twi.h"

#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"


#define BLUE_LED                        BSP_BOARD_LED_0                         /**< Is on when device is advertising. */
#define RED_LED                         BSP_BOARD_LED_1                         /**< Is on when device has connected. */
#define GREEN_LED                       BSP_BOARD_LED_2                         /**< LED to be toggled with the help of the LED Button Service. */ 
#define LEDBUTTON_BUTTON                BSP_BUTTON_0                            /**< Button that will trigger the notification event with the LED Button Service */
#define STATBUTTON_BUTTON               BSP_BUTTON_1    
//change to red blue green led
//implement calculations and read/write current time
//implement variable modification for ble send
#define DEVICE_NAME                     "CuittNew"                         /**< Name of device. Will be included in the advertising data. */

#define APP_BLE_OBSERVER_PRIO           3                                       /**< Application's BLE observer priority. You shouldn't need to modify this value. */
#define APP_BLE_CONN_CFG_TAG            1                                       /**< A tag identifying the SoftDevice BLE configuration. */

#define APP_ADV_INTERVAL                64                                      /**< The advertising interval (in units of 0.625 ms; this value corresponds to 40 ms). */
#define APP_ADV_DURATION                BLE_GAP_ADV_TIMEOUT_GENERAL_UNLIMITED   /**< The advertising time-out (in units of seconds). When set to 0, we will never time out. */


#define MIN_CONN_INTERVAL               MSEC_TO_UNITS(100, UNIT_1_25_MS)        /**< Minimum acceptable connection interval (0.5 seconds). */
#define MAX_CONN_INTERVAL               MSEC_TO_UNITS(200, UNIT_1_25_MS)        /**< Maximum acceptable connection interval (1 second). */
#define SLAVE_LATENCY                   0                                       /**< Slave latency. */
#define CONN_SUP_TIMEOUT                MSEC_TO_UNITS(4000, UNIT_10_MS)         /**< Connection supervisory time-out (4 seconds). */

#define FIRST_CONN_PARAMS_UPDATE_DELAY  APP_TIMER_TICKS(20000)                  /**< Time from initiating event (connect or start of notification) to first time sd_ble_gap_conn_param_update is called (15 seconds). */
#define NEXT_CONN_PARAMS_UPDATE_DELAY   APP_TIMER_TICKS(5000)                   /**< Time between each call to sd_ble_gap_conn_param_update after the first call (5 seconds). */
#define MAX_CONN_PARAMS_UPDATE_COUNT    3                                       /**< Number of attempts before giving up the connection parameter negotiation. */

#define BUTTON_DETECTION_DELAY          APP_TIMER_TICKS(50)                     /**< Delay from a GPIOTE event until a button is reported as pushed (in number of timer ticks). */

#define DEAD_BEEF                       0xDEADBEEF                              /**< Value used as error code on stack dump, can be used to identify stack location on stack unwind. */


BLE_LBS_DEF(m_lbs);                                                             /**< LED Button Service instance. */
NRF_BLE_GATT_DEF(m_gatt);                                                       /**< GATT module instance. */
NRF_BLE_QWR_DEF(m_qwr);                                                         /**< Context for the Queued Write module.*/

unsigned char daynum = 0;//day index
unsigned char weeknum = 1;//week index
volatile unsigned int click = 0;//click counter for BLE pairing
uint16_t Draw_length = 0;//the length of time of the draw
uint16_t Draw_length_last;
uint16_t Draw_length_average_3 = 0;//the average time taking the last 3 draws
uint16_t Draw_length_average = 0;//the average time taking a draw
uint16_t Draw_length_average_t = 0;//the average time taking a draw today (used for total average)
unsigned char Buffer = 100;//the current buffer for draws taken in close intervals
unsigned char T = 1;//the max draws that can be taken before the session counter incrementns for draws taken in close intervals: estimated from average hits taken in 3 hour intervals
unsigned int DCT = 0;//the amount of draws taken
unsigned int SCT = 0;//the amount of sessions
unsigned int SCT_average = 0;//daily sessions average
unsigned int DCT_average = 0;//daily count average
uint16_t Draw_length_total = 0;//todays total time taking draws
uint16_t DLT_average = 0;//the average total time taking draws each daysigned int DL_average = 0;//average draw time taking a draw
uint32_t Time_between = 0;//time between sessions
uint32_t Time_between_average_3 = 0;//average time between last three sessions
uint32_t Time_between_average = 0;//average time between sessions
uint32_t Time_between_t = 0;
uint32_t Wait_period;// = 16/SCT_average;//calculated wait period between seshes to make progress
unsigned int Over_count;//the amount of hits taken after today's draw total surpasses the draw length total average
unsigned int Relapse_code;//whether the user is relapsing or not (1 or 0)
unsigned int Suggestion;//the suggested draw length
unsigned int Relapse_count;//the amount of hits taken with a negative progress code
unsigned int Progress_count;//the amount of hits taken with a positive progress code
unsigned int Progress_code;//qualitative value determining how much the user is progressing
unsigned int Progress;//whether positive, negative, or neutral
unsigned int Prog_ranking;//the ranking of the hit length or wait period in successfully making progress towards the user's goal
float Decay = 0.95;//the seconds the user is expected to reduce their average draw length by
unsigned int Session_interval = 500;//interval after hit for session count increment: if weed setting, 2*60*60 and if nicotine setting, 1
unsigned int Three_average_array[3][2] = {0};
bool voltage_lock = false;
bool auto_lock = false;
unsigned int Calculation_array[7][5] = {0};
unsigned char z;

uint8_t tx_data[8] = {0x00};

/* TWI instance ID. */
#define TWI_INSTANCE_ID     0

#define RTC_ADDRESS          (0x68U)
uint8_t sec, min, hr, day, date, month, year;

const nrf_drv_rtc_t rtc = NRF_DRV_RTC_INSTANCE(2);

static uint16_t m_conn_handle = BLE_CONN_HANDLE_INVALID;                        /**< Handle of the current connection. */

static uint8_t m_adv_handle = BLE_GAP_ADV_SET_HANDLE_NOT_SET;                   /**< Advertising handle used to identify an advertising set. */
static uint8_t m_enc_advdata[BLE_GAP_ADV_SET_DATA_SIZE_MAX];                    /**< Buffer for storing an encoded advertising set. */
static uint8_t m_enc_scan_response_data[BLE_GAP_ADV_SET_DATA_SIZE_MAX];         /**< Buffer for storing an encoded scan data. */

volatile unsigned int dc = 0;//milliseconds
volatile unsigned char Button = 0;//button pressed value
volatile unsigned char Button_b = 0;//button pressed value b

/**@brief Struct that contains pointers to the encoded advertising data. */
static ble_gap_adv_data_t m_adv_data =
{
    .adv_data =
    {
        .p_data = m_enc_advdata,
        .len    = BLE_GAP_ADV_SET_DATA_SIZE_MAX
    },
    .scan_rsp_data =
    {
        .p_data = m_enc_scan_response_data,
        .len    = BLE_GAP_ADV_SET_DATA_SIZE_MAX

    }
};

/**@brief Function for assert macro callback.
 *
 * @details This function will be called in case of an assert in the SoftDevice.
 *
 * @warning This handler is an example only and does not fit a final product. You need to analyze
 *          how your product is supposed to react in case of Assert.
 * @warning On assert from the SoftDevice, the system can only recover on reset.
 *
 * @param[in] line_num    Line number of the failing ASSERT call.
 * @param[in] p_file_name File name of the failing ASSERT call.
 */
void assert_nrf_callback(uint16_t line_num, const uint8_t * p_file_name)
{
    app_error_handler(DEAD_BEEF, line_num, p_file_name);
}


/**@brief Function for the LEDs initialization.
 *
 * @details Initializes all LEDs used by the application.
 */
static void leds_init(void)
{
    bsp_board_init(BSP_INIT_LEDS);
}


/**@brief Function for the Timer initialization.
 *
 * @details Initializes the timer module.
 */
static void timers_init(void)
{
    // Initialize timer module, making it use the scheduler
    ret_code_t err_code = app_timer_init();
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for the GAP initialization.
 *
 * @details This function sets up all the necessary GAP (Generic Access Profile) parameters of the
 *          device including the device name, appearance, and the preferred connection parameters.
 */
static void gap_params_init(void)
{
    ret_code_t              err_code;
    ble_gap_conn_params_t   gap_conn_params;
    ble_gap_conn_sec_mode_t sec_mode;

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&sec_mode);

    err_code = sd_ble_gap_device_name_set(&sec_mode,
                                          (const uint8_t *)DEVICE_NAME,
                                          strlen(DEVICE_NAME));
    APP_ERROR_CHECK(err_code);

    memset(&gap_conn_params, 0, sizeof(gap_conn_params));

    gap_conn_params.min_conn_interval = MIN_CONN_INTERVAL;
    gap_conn_params.max_conn_interval = MAX_CONN_INTERVAL;
    gap_conn_params.slave_latency     = SLAVE_LATENCY;
    gap_conn_params.conn_sup_timeout  = CONN_SUP_TIMEOUT;

    err_code = sd_ble_gap_ppcp_set(&gap_conn_params);
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for initializing the GATT module.
 */
static void gatt_init(void)
{
    ret_code_t err_code = nrf_ble_gatt_init(&m_gatt, NULL);
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for initializing the Advertising functionality.
 *
 * @details Encodes the required advertising data and passes it to the stack.
 *          Also builds a structure to be passed to the stack when starting advertising.
 */
static void advertising_init(void)
{
    ret_code_t    err_code;
    ble_advdata_t advdata;
    ble_advdata_t srdata;

    ble_uuid_t adv_uuids[] = {{LBS_UUID_SERVICE, m_lbs.uuid_type}};

    // Build and set advertising data.
    memset(&advdata, 0, sizeof(advdata));

    advdata.name_type          = BLE_ADVDATA_FULL_NAME;
    advdata.include_appearance = true;
    advdata.flags              = BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE;


    memset(&srdata, 0, sizeof(srdata));
    srdata.uuids_complete.uuid_cnt = sizeof(adv_uuids) / sizeof(adv_uuids[0]);
    srdata.uuids_complete.p_uuids  = adv_uuids;

    err_code = ble_advdata_encode(&advdata, m_adv_data.adv_data.p_data, &m_adv_data.adv_data.len);
    APP_ERROR_CHECK(err_code);

    err_code = ble_advdata_encode(&srdata, m_adv_data.scan_rsp_data.p_data, &m_adv_data.scan_rsp_data.len);
    APP_ERROR_CHECK(err_code);

    ble_gap_adv_params_t adv_params;

    // Set advertising parameters.
    memset(&adv_params, 0, sizeof(adv_params));

    adv_params.primary_phy     = BLE_GAP_PHY_1MBPS;
    adv_params.duration        = APP_ADV_DURATION;
    adv_params.properties.type = BLE_GAP_ADV_TYPE_CONNECTABLE_SCANNABLE_UNDIRECTED;
    adv_params.p_peer_addr     = NULL;
    adv_params.filter_policy   = BLE_GAP_ADV_FP_ANY;
    adv_params.interval        = APP_ADV_INTERVAL;

    err_code = sd_ble_gap_adv_set_configure(&m_adv_handle, &m_adv_data, &adv_params);
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for handling Queued Write Module errors.
 *
 * @details A pointer to this function will be passed to each service which may need to inform the
 *          application about an error.
 *
 * @param[in]   nrf_error   Error code containing information about what went wrong.
 */
static void nrf_qwr_error_handler(uint32_t nrf_error)
{
    APP_ERROR_HANDLER(nrf_error);
}


/**@brief Function for handling write events to the LED characteristic.
 *
 * @param[in] p_lbs     Instance of LED Button Service to which the write applies.
 * @param[in] led_state Written/desired state of the LED.
 */
static void led_write_handler(uint16_t conn_handle, ble_lbs_t * p_lbs, uint8_t led_state)
{
    if (led_state)
    {
        bsp_board_led_on(GREEN_LED);
        NRF_LOG_INFO("Received LED ON!");
    }
    else
    {
        bsp_board_led_off(GREEN_LED);
        NRF_LOG_INFO("Received LED OFF!");
    }
}


/**@brief Function for initializing services that will be used by the application.
 */
static void services_init(void)
{
    ret_code_t         err_code;
    ble_lbs_init_t     init     = {0};
    nrf_ble_qwr_init_t qwr_init = {0};

    // Initialize Queued Write Module.
    qwr_init.error_handler = nrf_qwr_error_handler;

    err_code = nrf_ble_qwr_init(&m_qwr, &qwr_init);
    APP_ERROR_CHECK(err_code);

    // Initialize LBS.
    init.led_write_handler = led_write_handler;

    err_code = ble_lbs_init(&m_lbs, &init);
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for handling the Connection Parameters Module.
 *
 * @details This function will be called for all events in the Connection Parameters Module that
 *          are passed to the application.
 *
 * @note All this function does is to disconnect. This could have been done by simply
 *       setting the disconnect_on_fail config parameter, but instead we use the event
 *       handler mechanism to demonstrate its use.
 *
 * @param[in] p_evt  Event received from the Connection Parameters Module.
 */
static void on_conn_params_evt(ble_conn_params_evt_t * p_evt)
{
    ret_code_t err_code;

    if (p_evt->evt_type == BLE_CONN_PARAMS_EVT_FAILED)
    {
        err_code = sd_ble_gap_disconnect(m_conn_handle, BLE_HCI_CONN_INTERVAL_UNACCEPTABLE);
        APP_ERROR_CHECK(err_code);
    }
}


/**@brief Function for handling a Connection Parameters error.
 *
 * @param[in] nrf_error  Error code containing information about what went wrong.
 */
static void conn_params_error_handler(uint32_t nrf_error)
{
    APP_ERROR_HANDLER(nrf_error);
}


/**@brief Function for initializing the Connection Parameters module.
 */
static void conn_params_init(void)
{
    ret_code_t             err_code;
    ble_conn_params_init_t cp_init;

    memset(&cp_init, 0, sizeof(cp_init));

    cp_init.p_conn_params                  = NULL;
    cp_init.first_conn_params_update_delay = FIRST_CONN_PARAMS_UPDATE_DELAY;
    cp_init.next_conn_params_update_delay  = NEXT_CONN_PARAMS_UPDATE_DELAY;
    cp_init.max_conn_params_update_count   = MAX_CONN_PARAMS_UPDATE_COUNT;
    cp_init.start_on_notify_cccd_handle    = BLE_GATT_HANDLE_INVALID;
    cp_init.disconnect_on_fail             = false;
    cp_init.evt_handler                    = on_conn_params_evt;
    cp_init.error_handler                  = conn_params_error_handler;

    err_code = ble_conn_params_init(&cp_init);
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for starting advertising.
 */
static void advertising_start(void)
{
    ret_code_t           err_code;

    err_code = sd_ble_gap_adv_start(m_adv_handle, APP_BLE_CONN_CFG_TAG);
    APP_ERROR_CHECK(err_code);

    bsp_board_led_on(BLUE_LED);
}


/**@brief Function for handling BLE events.
 *
 * @param[in]   p_ble_evt   Bluetooth stack event.
 * @param[in]   p_context   Unused.
 */
static void ble_evt_handler(ble_evt_t const * p_ble_evt, void * p_context)
{
    ret_code_t err_code;

    switch (p_ble_evt->header.evt_id)
    {
        case BLE_GAP_EVT_CONNECTED:
            NRF_LOG_INFO("Connected");
            bsp_board_led_off(BLUE_LED);
            m_conn_handle = p_ble_evt->evt.gap_evt.conn_handle;
            err_code = nrf_ble_qwr_conn_handle_assign(&m_qwr, m_conn_handle);
            APP_ERROR_CHECK(err_code);
            err_code = app_button_enable();
            APP_ERROR_CHECK(err_code);
            break;

        case BLE_GAP_EVT_DISCONNECTED:
            NRF_LOG_INFO("Disconnected");
            m_conn_handle = BLE_CONN_HANDLE_INVALID;
            err_code = app_button_disable();
            APP_ERROR_CHECK(err_code);
            advertising_start();
            break;

        case BLE_GAP_EVT_SEC_PARAMS_REQUEST:
            // Pairing not supported
            err_code = sd_ble_gap_sec_params_reply(m_conn_handle,
                                                   BLE_GAP_SEC_STATUS_PAIRING_NOT_SUPP,
                                                   NULL,
                                                   NULL);
            APP_ERROR_CHECK(err_code);
            break;

        case BLE_GAP_EVT_PHY_UPDATE_REQUEST:
        {
            NRF_LOG_DEBUG("PHY update request.");
            ble_gap_phys_t const phys =
            {
                .rx_phys = BLE_GAP_PHY_AUTO,
                .tx_phys = BLE_GAP_PHY_AUTO,
            };
            err_code = sd_ble_gap_phy_update(p_ble_evt->evt.gap_evt.conn_handle, &phys);
            APP_ERROR_CHECK(err_code);
        } break;

        case BLE_GATTS_EVT_SYS_ATTR_MISSING:
            // No system attributes have been stored.
            err_code = sd_ble_gatts_sys_attr_set(m_conn_handle, NULL, 0, 0);
            APP_ERROR_CHECK(err_code);
            break;

        case BLE_GATTC_EVT_TIMEOUT:
            // Disconnect on GATT Client timeout event.
            NRF_LOG_DEBUG("GATT Client Timeout.");
            err_code = sd_ble_gap_disconnect(p_ble_evt->evt.gattc_evt.conn_handle,
                                             BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
            APP_ERROR_CHECK(err_code);
            break;

        case BLE_GATTS_EVT_TIMEOUT:
            // Disconnect on GATT Server timeout event.
            NRF_LOG_DEBUG("GATT Server Timeout.");
            err_code = sd_ble_gap_disconnect(p_ble_evt->evt.gatts_evt.conn_handle,
                                             BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
            APP_ERROR_CHECK(err_code);
            break;

        default:
            // No implementation needed.
            break;
    }
}


/**@brief Function for initializing the BLE stack.
 *
 * @details Initializes the SoftDevice and the BLE event interrupt.
 */
static void ble_stack_init(void)
{
    ret_code_t err_code;

    err_code = nrf_sdh_enable_request();
    APP_ERROR_CHECK(err_code);

    // Configure the BLE stack using the default settings.
    // Fetch the start address of the application RAM.
    uint32_t ram_start = 0;
    err_code = nrf_sdh_ble_default_cfg_set(APP_BLE_CONN_CFG_TAG, &ram_start);
    APP_ERROR_CHECK(err_code);

    // Enable BLE stack.
    err_code = nrf_sdh_ble_enable(&ram_start);
    APP_ERROR_CHECK(err_code);

    // Register a handler for BLE events.
    NRF_SDH_BLE_OBSERVER(m_ble_observer, APP_BLE_OBSERVER_PRIO, ble_evt_handler, NULL);
}


/**@brief Function for handling events from the button handler module.
 *
 * @param[in] pin_no        The pin that the event applies to.
 * @param[in] button_action The button action (press/release).
 */

static void rtc_handler(nrf_drv_rtc_int_type_t int_type)
{
    if (int_type == NRF_DRV_RTC_INT_TICK)
    {
        dc = dc + 100;
        printf("tick: %d\n",dc);
    }
    if(Button == 1){
        Draw_length = dc;
        printf("Draw Length: %dms\n", Draw_length);
        if(Draw_length_total >= DLT_average){
            printf("Exceeding today's total\n");
            bsp_board_led_on(RED_LED);
            if(voltage_lock = 1){
                //Lock voltage
                printf("Voltage Lock Enabled\n");
            }
        }else if(Time_between <= Wait_period){
            printf("Wait longer to take a hit\n");
            bsp_board_led_on(RED_LED);
            if(voltage_lock = 1){
                //Lock voltage
                printf("Voltage Lock Enabled\n");
            }
        }else if(Draw_length >= Suggestion - 3){
            printf("Stop Suggested\n");
            bsp_board_led_on(BLUE_LED);
            if(auto_lock = 1){
                if(Draw_length >= Suggestion){
                    printf("Auto Suggestion Lock Enabled\n");
                    //Lock voltage
                    bsp_board_led_off(BLUE_LED);
                    bsp_board_led_on(RED_LED);
                }
            }
            else if(Draw_length >= Draw_length_average){
                printf("Draw length longer than average\n");
                bsp_board_led_off(BLUE_LED);
                bsp_board_led_on(RED_LED);
                if(voltage_lock = 1){
                    //Lock voltage
                    printf("Voltage Lock Enabled\n");
                }
            }
        }
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

uint32_t current_time = 0;
uint32_t current_time_b;

//Convert BCD in the real time clock to decimal
uint8_t bcdToDec(uint8_t val)
{ 
    return( (val/16*10) + (val%16) );
}


static void button_event_handler(uint8_t pin_no, uint8_t button_action)
{
    ret_code_t err_code;
    uint8_t arr[14];

    switch (pin_no)
    {
        case LEDBUTTON_BUTTON:
            NRF_LOG_INFO("Send button state change.");
            Button = 1 - Button;
            printf("Button: %d\n", Button);
            if(Button == 1){
                nrf_drv_rtc_enable(&rtc);
                nrf_drv_rtc_tick_enable(&rtc, true);
                z = 0;
                //take current time
                
                uint8_t addr[1] = {0x00};
                err_code = nrf_drv_twi_tx(&twi, RTC_ADDRESS, addr, 1, false);
                APP_ERROR_CHECK(err_code);
                err_code = nrf_drv_twi_rx(&twi, RTC_ADDRESS, tx_data, sizeof(tx_data));
                APP_ERROR_CHECK(err_code);
                for(uint32_t i = 0; i <= 6; i++){
                    tx_data[i] = bcdToDec(tx_data[i]);
                    printf("0x%02x %d\n", i, tx_data[i]);
                }
                current_time = tx_data[0]+tx_data[1]*60+tx_data[2]*60*60+tx_data[3]*24*60*60;
                if(Time_between >= Session_interval){
                    if(Buffer == T){
                        Buffer = 0;
                        DCT++;
                        SCT++;
                    }
                    else{
                        DCT++;
                        Buffer++;
                    }
                }
                else
                {
                    DCT++;
                    SCT++;
                }
            }
            else{
                nrf_drv_rtc_disable(&rtc);
                nrf_drv_rtc_tick_disable(&rtc);
                //negate count for miniscule draws to maintain integrity of suggestion
                if(Draw_length < 400){
                DCT--;
                SCT--;
                }
                //take current time
                
                uint8_t addr[1] = {0x00};
                err_code = nrf_drv_twi_tx(&twi, RTC_ADDRESS, addr, 1, false);
                APP_ERROR_CHECK(err_code);
                err_code = nrf_drv_twi_rx(&twi, RTC_ADDRESS, tx_data, sizeof(tx_data));
                APP_ERROR_CHECK(err_code);
                for(uint32_t i = 0; i <= 6; i++){
                    tx_data[i] = bcdToDec(tx_data[i]);
                    printf("0x%02x %d\n", i, tx_data[i]);
                }
                current_time = tx_data[0]+tx_data[1]*60+tx_data[2]*60*60+tx_data[3]*24*60*60;
                //compare with last time taken
                if(current_time_b == 0){
                    Time_between = 0;
                }else{
                    Time_between = current_time - current_time_b;
                }
                printf("Time 1: %lu\n",current_time);
                printf("Time 2: %lu\n",current_time_b);
                printf("Time Between: %lu\n",Time_between);
                current_time_b = current_time;
                bsp_board_led_off(RED_LED);
                bsp_board_led_off(BLUE_LED);
                bsp_board_led_off(GREEN_LED);

                static signed char c = -1;
                static unsigned char a = 0;//three count index
                if(c == 2){
                    //do nothing
                }
                else
                {
                    c++;
                }
                if(a == 3){
                a = 0;
                }

                Draw_length_total += Draw_length;

                Calculation_array[daynum][0] = Draw_length_total;
                DLT_average = 0;
                for(char i = 0; i <= daynum; i++){
                    DLT_average += Calculation_array[i][0];
                }
                DLT_average /= (daynum + 1);

                Three_average_array[a][0] = Draw_length;
                Draw_length_average_3 = 0;
                for(char i = 0; i <= c; i++){
                    Draw_length_average_3 += Three_average_array[i][0];
                }
                Draw_length_average_3 /= (c + 1);

                Three_average_array[a][1] = Time_between;
                Time_between_average_3 = 0;
                for(int i = 0; i <= c; i++){
                    Time_between_average_3 += Three_average_array[i][1];
                }
                Time_between_average_3 /= (c + 1);

                a++;

                Calculation_array[daynum][1] = SCT;
                SCT_average = 0;
                for(char i = 0; i <= daynum; i++){
                    SCT_average += Calculation_array[i][1];
                }
                SCT_average /= (daynum + 1);

                Calculation_array[daynum][2] = DCT;
                DCT_average = 0;
                for(char i = 0; i <= daynum; i++){
                    DCT_average += Calculation_array[i][2];
                }
                DCT_average /= (daynum + 1);

                Draw_length_average_t += Draw_length;

                Calculation_array[daynum][3] = Draw_length_average_t;
                Draw_length_average_t = 0;
                for(char i = 0; i <= daynum; i++){
                    Draw_length_average_t += Calculation_array[i][3];
                }
                Draw_length_average = Draw_length_average_t/DCT;

                Time_between_t += Time_between;

                Calculation_array[daynum][4] = Time_between_t;
                Time_between_t = 0;
                for(char i = 0; i <= daynum; i++){
                    Time_between_t += Calculation_array[i][4];
                }
                Time_between_average = Time_between_t/SCT;

                Wait_period = 16/SCT_average;
    
                Suggestion = DLT_average/DCT_average * Decay;//FIX SUGGESITON TOO SMALL
                
                printf("Draw length for this draw: %d\n", Draw_length);
                printf("Current time: %d\n", current_time);
                /*
                printf("Draws today: %d\n", DCT);
                printf("Sessions today: %d\n", SCT);
                printf("Draw length for this draw: %d\n", Draw_length);
                printf("Draw length total for today: %d\n", Draw_length_total);
                printf("Time between this draw and the last: %d\n", Time_between);
                printf("Average amount of sesions a day: %d\n", SCT_average);
                printf("Average amount of draws a day: %d\n", DCT_average);
                printf("Average draw length for individual draws: %d\n", Draw_length_average);
                printf("Average length of the last three draws: %d\n", Draw_length_average_3);
                printf("Average draw length total for all days: %d\n", DLT_average);
                printf("Average time between the past three draws: %d\n", Time_between_average_3);
                printf("Average time between draws: %d\n", Time_between_average);
                printf("Suggested wait period between draws: %d\n", Wait_period);
                printf("Suggested draw length: %d\n", Suggestion);
                */
                arr[0] = current_time >> 24;
                arr[1] = current_time >> 16;
                arr[2] = current_time >> 8;
                arr[3] = current_time;

                arr[4] = Draw_length >> 8;
                arr[5] = Draw_length;

                arr[6] = DCT >> 8;
                arr[7] = DCT;

                arr[8] = SCT >> 8;
                arr[9] = SCT;

                dc = 0;
                Draw_length = 0;

                printf("__________BLE transmit attempt___________\n");
                err_code = ble_lbs_on_button_change(m_conn_handle, &m_lbs, arr);
                printf("BLE transmit complete\n");
                if (err_code != NRF_SUCCESS &&
                err_code != BLE_ERROR_INVALID_CONN_HANDLE &&
                err_code != NRF_ERROR_INVALID_STATE &&
                err_code != BLE_ERROR_GATTS_SYS_ATTR_MISSING)
                {
                    APP_ERROR_CHECK(err_code);
                }
            }            
            break;
        case STATBUTTON_BUTTON:
            Button_b = 1 - Button_b;
            printf("Button B: %d\n", Button_b);
            nrf_drv_rtc_enable(&rtc);
            nrf_drv_rtc_tick_enable(&rtc,true);
            //Current_time();
            while(dc <= 3000){
                if(Draw_length_total >= DLT_average){
                    bsp_board_led_on(RED_LED);
                }
                else if(Time_between <= Wait_period){
                    bsp_board_led_on(RED_LED);
                }
                else{
                    bsp_board_led_on(BLUE_LED);
                }
            }
            bsp_board_led_off(RED_LED);
            bsp_board_led_off(BLUE_LED);
            bsp_board_led_off(GREEN_LED);
            nrf_drv_rtc_disable(&rtc);
            nrf_drv_rtc_tick_disable(&rtc);
            dc = 0;
            break;
        default:
            APP_ERROR_HANDLER(pin_no);
            break;
    }
}

/**@brief Function for initializing the button handler module.
 */
static void buttons_init(void)
{
    ret_code_t err_code;

    //The array must be static because a pointer to it will be saved in the button handler module.
    static app_button_cfg_t buttons[] =
    {
        {LEDBUTTON_BUTTON, false, BUTTON_PULL, button_event_handler}
    };

    err_code = app_button_init(buttons, ARRAY_SIZE(buttons),
                               BUTTON_DETECTION_DELAY);
    APP_ERROR_CHECK(err_code);
}


static void log_init(void)
{
    ret_code_t err_code = NRF_LOG_INIT(NULL);
    APP_ERROR_CHECK(err_code);

    NRF_LOG_DEFAULT_BACKENDS_INIT();
}


/**@brief Function for initializing power management.
 */
static void power_management_init(void)
{
    ret_code_t err_code;
    err_code = nrf_pwr_mgmt_init();
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for handling the idle state (main loop).
 *
 * @details If there is no pending log operation, then sleep until next the next event occurs.
 */
static void idle_state_handle(void)
{
    if (NRF_LOG_PROCESS() == false)
    {
        nrf_pwr_mgmt_run();
    }
}

/**@brief Function for application main entry.
 */
int main(void)
{
    // Initialize.
    log_init();
    leds_init();
    timers_init();
    buttons_init();
    power_management_init();
    ble_stack_init();
    gap_params_init();
    gatt_init();
    services_init();
    advertising_init();
    conn_params_init();
    rtc_config();
    twi_init();
    nrf_drv_rtc_disable(&rtc);
    nrf_drv_rtc_tick_disable(&rtc);
    
    // Start execution.
    NRF_LOG_INFO("Blinky example started.");
    advertising_start();

    // Enter main loop.
    for (;;)
    {
        idle_state_handle();
    }
}


/**
 * @}
 */
