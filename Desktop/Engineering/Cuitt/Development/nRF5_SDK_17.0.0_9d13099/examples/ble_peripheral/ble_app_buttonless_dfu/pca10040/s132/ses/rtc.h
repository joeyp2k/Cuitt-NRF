#include "nrf_drv_rtc.h"

void rtc_config(void);

extern const nrf_drv_rtc_t rtc;
extern bool sleep;
extern int stat_press_count;
extern const nrf_drv_rtc_t timeout;