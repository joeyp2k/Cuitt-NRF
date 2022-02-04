#define RTCLK_ADDRESS          (0x68U)
#define CONTROL_REGISTER (0x0E) //00011101 0x1D
#define STATUS_REGISTER (0x0F)
#define ALARM_1_HOURS (0x09) //11101101 0xED 
#define ALARM_1_DAYS (0x0A) //10000000 0x80


extern uint8_t rtclk_data[8];

uint8_t bcdToDec(uint8_t val);
void utcToRTClk(uint32_t utc);