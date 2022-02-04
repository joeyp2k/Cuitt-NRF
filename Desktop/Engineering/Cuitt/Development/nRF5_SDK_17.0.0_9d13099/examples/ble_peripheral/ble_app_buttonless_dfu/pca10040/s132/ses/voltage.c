#include "voltage.h"
#include "app_error.h"
#include "nrf_drv_rtc.h"
#include "rtc.h"
#include "nrf_log.h"
#include "twi.h"
#include "rtclk.h"
#include "acclrm.h"

uint16_t m_conn_handle = BLE_CONN_HANDLE_INVALID;  

uint64_t current_time = 0;
uint64_t current_time_b;

bool connected = false;//ble connection
bool transmit_success = false;

unsigned char daynum = 0;//day index
unsigned char weeknum = 1;//week index
volatile unsigned int click = 0;//click counter for BLE pairing
uint8_t draw_length = 0;//the length of time of the draw
uint16_t draw_length_average_3 = 0;//the average time taking the last 3 draws
uint16_t draw_length_average = 0;//the average time taking a draw
uint16_t draw_length_average_t = 0;//the average time taking a draw today (used for total average)
unsigned char buffer = 100;//the current buffer for draws taken in close intervals
unsigned int draw_count = 0;//the amount of draws taken
unsigned int draw_count_average = 0;//daily count average
uint16_t draw_length_total = 0;//todays total time taking draws
uint16_t draw_length_total_average = 0;//the average total time taking draws each day
uint32_t time_between = 0;//time between sessions
uint32_t time_between_average_3 = 0;//average time between last three sessions
uint32_t time_between_average = 0;//average time between sessions
uint32_t time_between_t = 0;
uint8_t days_passed = 0;
uint32_t wait_period;//calculated wait period between draws to make progress
unsigned int suggestion;//the suggested draw length
float decay = 0.95;//the seconds the user is expected to reduce their average draw length by
unsigned int three_average[3][2] = {0};
bool voltage_lock = false;
bool auto_lock = false;
unsigned int calculation[7][5] = {0};
volatile unsigned int dc = 0;//milliseconds
volatile unsigned char Button = 0;//button pressed value
volatile unsigned char Button_b = 0;//button pressed value b
volatile unsigned char Button_c = 0;//button pressed value c
volatile unsigned char Button_d = 0;//button pressed value d
uint8_t arr[10];
int stack_size = 0;

void log_draw(){
    ret_code_t err_code;
    
    nrf_drv_rtc_enable(&rtc);
    nrf_drv_rtc_tick_enable(&rtc, true);

    //take current time
    uint8_t start_reg = 0x00;//START REGISTER OF TRANSMIT buffer
    err_code = nrf_drv_twi_tx(&twi, RTCLK_ADDRESS, &start_reg, 1, false);
    APP_ERROR_CHECK(err_code);
    err_code = nrf_drv_twi_rx(&twi, RTCLK_ADDRESS, rtclk_data, sizeof(rtclk_data));
    APP_ERROR_CHECK(err_code);
    for(uint32_t i = 0; i < 7; i++){
        rtclk_data[i] = bcdToDec(rtclk_data[i]);
        printf("0x%02x %d\n", i, rtclk_data[i]);
    }
    current_time = rtclk_data[0] + rtclk_data[1]*60 + rtclk_data[2]*60*60 + rtclk_data[3]*24*60*60;
    draw_count++;
}

void process_packet(){
    ret_code_t err_code;

    //negate count for miniscule draws to maintain integrity of suggestion
    if(draw_length < 4){
    draw_count--;
    }
    //take current time
    
    uint8_t start_reg = 0x00;
    err_code = nrf_drv_twi_tx(&twi, RTCLK_ADDRESS, &start_reg, 1, false);
    APP_ERROR_CHECK(err_code);
    err_code = nrf_drv_twi_rx(&twi, RTCLK_ADDRESS, rtclk_data, sizeof(rtclk_data));
    APP_ERROR_CHECK(err_code);
    for(uint32_t i = 0; i < 7; i++){
        rtclk_data[i] = bcdToDec(rtclk_data[i]);
        printf("0x%02x %d\n", i, rtclk_data[i]);
    }
    current_time = rtclk_data[0] + rtclk_data[1]*60 + rtclk_data[2]*60*60 + rtclk_data[4]*24*60*60 + rtclk_data[5]*2629800 + rtclk_data[6]*3.154e+7;
    //compare with last time taken
    if(current_time_b == 0){
        time_between = 0;
    }else{
        time_between = current_time - current_time_b;
    }
    current_time_b = current_time;
    bsp_board_led_off(RED_LED);
    bsp_board_led_off(BLUE_LED);
    bsp_board_led_off(GREEN_LED);

    static signed char c = -1;
    static unsigned char a = 0;//three count index
    if(c != 2){
        c++;
    }
    if(a == 3){
    a = 0;
    }

    draw_length_total += draw_length;

    calculation[daynum][0] = draw_length_total;
    draw_length_total_average = 0;
    for(char i = 0; i <= daynum; i++){
        draw_length_total_average += calculation[i][0];
    }
    draw_length_total_average /= (daynum + 1);

    three_average[a][0] = draw_length;
    draw_length_average_3 = 0;
    for(char i = 0; i <= c; i++){
        draw_length_average_3 += three_average[i][0];
    }
    draw_length_average_3 /= (c + 1);

    three_average[a][1] = time_between;
    time_between_average_3 = 0;
    for(int i = 0; i <= c; i++){
        time_between_average_3 += three_average[i][1];
    }
    time_between_average_3 /= (c + 1);

    a++;

    calculation[daynum][2] = draw_count;
    draw_count_average = 0;
    for(char i = 0; i <= daynum; i++){
        draw_count_average += calculation[i][2];
    }
    draw_count_average /= (daynum + 1);

    draw_length_average_t += draw_length;

    calculation[daynum][3] = draw_length_average_t;
    draw_length_average_t = 0;
    for(char i = 0; i <= daynum; i++){
        draw_length_average_t += calculation[i][3];
    }
    draw_length_average = draw_length_average_t/draw_count;

    time_between_t += time_between;

    calculation[daynum][4] = time_between_t;
    time_between_t = 0;
    for(char i = 0; i <= daynum; i++){
        time_between_t += calculation[i][4];
    }
    time_between_average = time_between_t/draw_count;

    wait_period = 16/draw_count_average;

    suggestion = draw_length_total_average/draw_count_average * decay;//FIX SUGGESITON TOO SMALL
    
   
    /*
    printf("Average draw length for individual draws: %d\n", draw_length_average);
    printf("Average length of the last three draws: %d\n", draw_length_average_3);
    */
    arr[0] = current_time >> 56;
    arr[1] = current_time >> 48;
    arr[2] = current_time >> 40;
    arr[3] = current_time >> 32;
    arr[4] = current_time >> 24;
    arr[5] = current_time >> 16;
    arr[6] = current_time >> 8;
    arr[7] = current_time;

    arr[8] = draw_length;

    arr[9] = days_passed;

    dc = 0;
    draw_length = 0;

  
}

void increment_stack(){
    printf("Increment stack\n");
    stack_size++;
    //increase stack
}

void transmit_packet(){
    nrf_drv_rtc_disable(&rtc);
    nrf_drv_rtc_tick_disable(&rtc);

    ret_code_t err_code;
    if(connected == true){
        while(stack_size > 0){
            //send stack
            printf("__________BLE transmit attempt___________\n");
            err_code = ble_lbs_on_button_change(m_conn_handle, arr);
            if(err_code == NRF_SUCCESS){
              printf("BLE transmit complete\n");
              transmit_success = true;
              stack_size--;
            }else{
              printf("BLE transmit failed\n");
              transmit_success = false;
              return;
            }
            if (err_code != NRF_SUCCESS &&
            err_code != BLE_ERROR_INVALID_CONN_HANDLE &&
            err_code != NRF_ERROR_INVALID_STATE &&
            err_code != BLE_ERROR_GATTS_SYS_ATTR_MISSING)
            {
                APP_ERROR_CHECK(err_code);
            }
        }
        //send current draw
        printf("__________BLE transmit attempt___________\n");
        err_code = ble_lbs_on_button_change(m_conn_handle, arr);
        if(err_code == NRF_SUCCESS){
          printf("BLE transmit complete\n");
          transmit_success = true;
        }else{
          printf("BLE transmit failed\n");
          transmit_success = false;
        }
        if (err_code != NRF_SUCCESS &&
        err_code != BLE_ERROR_INVALID_CONN_HANDLE &&
        err_code != NRF_ERROR_INVALID_STATE &&
        err_code != BLE_ERROR_GATTS_SYS_ATTR_MISSING)
        {
            APP_ERROR_CHECK(err_code);
        }
        printf("Time 1: %lu\n",current_time);
        printf("Time 2: %lu\n",current_time_b);
        printf("Time Between: %lu\n",time_between);
        printf("Draw length for this draw: %d\n", draw_length);
        printf("Current time: %d\n", current_time);
        printf("Draws today: %d\n", draw_count);
        printf("Draw length total for today: %d\n", draw_length_total);
        printf("Average amount of draws a day: %d\n", draw_count_average);
        printf("Average draw length total for all days: %d\n", draw_length_total_average);
        printf("Average time between the past three draws: %d\n", time_between_average_3);
        printf("Average time between draws: %d\n", time_between_average);
        printf("Suggested wait period between draws: %d\n", wait_period);
        printf("Suggested draw length: %d\n", suggestion);
    }
}

void check_status(){
    Button_b = 1 - Button_b;
    if(dc <= 4000 && stat_press_count == 5){
            nrf_drv_rtc_disable(&rtc);
            nrf_drv_rtc_tick_disable(&rtc);
            dc = 0;
            stat_press_count = 0;
            sleep = true;
    }
    if(Button_b == 1 && stat_press_count == 0){
        printf("STATUS ON\n");
        stat_press_count++;
        nrf_drv_rtc_enable(&rtc);
        nrf_drv_rtc_tick_enable(&rtc,true);
        if(draw_length_total >= draw_length_total_average){
                printf("DAILY TOTAL EXCEEDED\n");
                bsp_board_led_on(RED_LED);
            }
            else if(time_between <= wait_period){
                printf("WAIT PERIOD NOT == 0\n");
                bsp_board_led_on(RED_LED);
            }
            else{
                printf("DRAW PERMITTED\n");
                bsp_board_led_on(BLUE_LED);
            }
    }else if(Button_b == 1){
        stat_press_count++;
    }
}

static void button_event_handler(uint8_t pin_no, uint8_t button_action)
{
    ret_code_t err_code;

    switch (pin_no)
    {
        case DRAW_BUTTON:
            NRF_LOG_INFO("Send button state change.");
            Button = 1 - Button;
            printf("Button: %d\n", Button);
            if(Button == 1){
                log_draw();
            }
            else{
                process_packet();
                transmit_packet();
                   if(transmit_success == false){
                      increment_stack();
                   }
                }                     
            break;
        case STAT_BUTTON:
                check_status();
            break;
        case ACCLRM_BUTTON:
            Button_d = 1 - Button_d;
            if(Button_d == 1){
                read_acclrm();
            }
            break;
        case PRESS_BUTTON:
            Button_c = 1 - Button_c;
            if(Button_c == 1){
                //read_pressure();
            }
            break;
        default:
            APP_ERROR_HANDLER(pin_no);
            break;
    }
}

void buttons_init(void)
{
    ret_code_t err_code;

    //The array must be static because a pointer to it will be saved in the button handler module.
    static app_button_cfg_t buttons[] =
    {
        {BSP_BUTTON_0, false, BUTTON_PULL, button_event_handler},
        {BSP_BUTTON_1, false, BUTTON_PULL, button_event_handler},
        {BSP_BUTTON_2, false, BUTTON_PULL, button_event_handler},
        {BSP_BUTTON_3, false, BUTTON_PULL, button_event_handler},
    };

    err_code = app_button_init(buttons, ARRAY_SIZE(buttons),
                               BUTTON_DETECTION_DELAY);
    APP_ERROR_CHECK(err_code);

    err_code = app_button_enable();

    APP_ERROR_CHECK(err_code);
}

static void voltage_init(){

};