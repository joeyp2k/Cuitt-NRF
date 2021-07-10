#include <stdint.h>

//Convert BCD in the real time clock to decimal

uint8_t rtclk_data[8] = {0x00};

uint8_t bcdToDec(uint8_t val)
{ 
    return( (val/16*10) + (val%16) );
}