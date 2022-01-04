#include "voltage.h"
#include "app_error.h"
#include "nrf_drv_rtc.h"
#include "rtc.h"
#include "nrf_log.h"
#include "twi.h"
#include "rtclk.h"
#include "press.h"
#include "acclrm.h"

uint16_t m_conn_handle = BLE_CONN_HANDLE_INVALID;  

uint32_t current_time = 0;
uint32_t current_time_b;

bool connected = false;//ble connection
bool transmit_success = false;

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
volatile unsigned int dc = 0;//milliseconds
volatile unsigned char Button = 0;//button pressed value
volatile unsigned char Button_b = 0;//button pressed value b
volatile unsigned char Button_c = 0;//button pressed value c
volatile unsigned char Button_d = 0;//button pressed value d
uint8_t arr[14];

void log_draw(){
    ret_code_t err_code;

    nrf_drv_rtc_enable(&rtc);
    nrf_drv_rtc_tick_enable(&rtc, true);
    z = 0;
    //take current time
    
    uint8_t start_reg[1] = {0x00};//START REGISTER OF TRANSMIT BUFFER
    err_code = nrf_drv_twi_tx(&twi, RTCLK_ADDRESS, start_reg, 1, false);
    APP_ERROR_CHECK(err_code);
    err_code = nrf_drv_twi_rx(&twi, RTCLK_ADDRESS, rtclk_data, sizeof(rtclk_data));
    APP_ERROR_CHECK(err_code);
    for(uint32_t i = 0; i <= 6; i++){
        rtclk_data[i] = bcdToDec(rtclk_data[i]);
        printf("0x%02x %d\n", i, rtclk_data[i]);
    }
    current_time = rtclk_data[0] + rtclk_data[1]*60 + rtclk_data[2]*60*60 + rtclk_data[3]*24*60*60;
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

void process_packet(){
    ret_code_t err_code;
    nrf_drv_rtc_disable(&rtc);
    nrf_drv_rtc_tick_disable(&rtc);
    //negate count for miniscule draws to maintain integrity of suggestion
    if(Draw_length < 400){
    DCT--;
    SCT--;
    }
    //take current time
    
    uint8_t start_reg[1] = {0x00};
    err_code = nrf_drv_twi_tx(&twi, RTCLK_ADDRESS, start_reg, 1, false);
    APP_ERROR_CHECK(err_code);
    err_code = nrf_drv_twi_rx(&twi, RTCLK_ADDRESS, rtclk_data, sizeof(rtclk_data));
    APP_ERROR_CHECK(err_code);
    for(uint32_t i = 0; i <= 6; i++){
        rtclk_data[i] = bcdToDec(rtclk_data[i]);
        printf("0x%02x %d\n", i, rtclk_data[i]);
    }
    current_time = rtclk_data[0] + rtclk_data[1]*60 + rtclk_data[2]*60*60 + rtclk_data[3]*24*60*60;
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

   
}

void increment_stack(){
    printf("Increment stack");
}

void transmit_packet(){
    ret_code_t err_code;
    printf("__________BLE transmit attempt___________\n");
    if(connected == true){
        err_code = ble_lbs_on_button_change(m_conn_handle, &m_cus, arr);
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
    }
}

void check_status(){
    Button_b = 1 - Button_b;
    if(Button_b == 1){
    printf("STATUS ON\n");
    nrf_drv_rtc_enable(&rtc);
    nrf_drv_rtc_tick_enable(&rtc,true);
    //Current_time();
    if(Draw_length_total >= DLT_average){
            printf("DAILY TOTAL EXCEEDED\n");
            bsp_board_led_on(RED_LED);
        }
        else if(Time_between <= Wait_period){
            printf("WAIT PERIOD NOT == 0\n");
            bsp_board_led_on(RED_LED);
        }
        else{
            printf("DRAW PERMITTED\n");
            bsp_board_led_on(BLUE_LED);
        }
    while(dc <= 3000){
        
    }
    bsp_board_led_off(RED_LED);
    bsp_board_led_off(BLUE_LED);
    bsp_board_led_off(GREEN_LED);
    printf("STATUS OFF\n");
    nrf_drv_rtc_disable(&rtc);
    nrf_drv_rtc_tick_disable(&rtc);
    dc = 0;
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
                             if(transmit_success == true){
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
                read_pressure();
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