#include <stdint.h>
#include "tl_common.h"
#include "drivers.h"
#include "stack/ble/ble.h"
#include "drivers/8258/flash.h"
#include "drivers/8258/pm.h"
#include "etime.h"
#include "main.h"

// Use the 32kHz RC timer which persists through suspend and deep retention.
// tick_32k_calib = number of 16MHz ticks per one 32kHz tick (set by SDK).
// Real ticks per second = 16000000 / tick_32k_calib.
extern unsigned short tick_32k_calib;

RAM uint32_t ticks_32k_per_second;
RAM uint32_t current_unix_time;
RAM struct date_time current_date = {0};

RAM uint32_t last_32k_tick;
RAM uint32_t last_reached_period[10] = {0};
RAM uint8_t has_ever_reached[10] = {0};

uint8_t map[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

_attribute_ram_code_ void recalibrate_32k(void)
{
    if (tick_32k_calib)
        ticks_32k_per_second = CLOCK_16M_SYS_TIMER_CLK_1S / tick_32k_calib;
    else
        ticks_32k_per_second = 32768;
}

_attribute_ram_code_ void init_time(void)
{
    recalibrate_32k();
    last_32k_tick = cpu_get_32k_tick();
    current_unix_time = 1709856857;
    current_date.tm_year = 2024;
    current_date.tm_month = 3;
    current_date.tm_week = 5;
    current_date.tm_day = 8;
    current_date.tm_min = 14;
    current_date.tm_hour = 0;
    current_date.tm_sec = 17;
}

_attribute_ram_code_ void handler_time(void)
{
    uint32_t now = cpu_get_32k_tick();
    uint32_t elapsed = now - last_32k_tick;
    if (elapsed >= ticks_32k_per_second)
    {
        uint32_t seconds = elapsed / ticks_32k_per_second;
        last_32k_tick += seconds * ticks_32k_per_second;
        current_unix_time += seconds;

        current_date.tm_min = (current_unix_time / 60) % 60;
        current_date.tm_hour = ((current_unix_time / 60) / 60) % 24;
        current_date.tm_sec = current_unix_time % 60;

        if (current_unix_time % 86400 == 0) {
            current_date.tm_month = current_date.tm_month % 12;
            if (current_date.tm_day + 1 > map[current_date.tm_month - 1]) {
                current_date.tm_day = 1;
                if (current_date.tm_month + 1 > 12) {
                    current_date.tm_month = 1;
                    current_date.tm_year += 1;
                } else {
                    current_date.tm_month += 1;
                }
            } else {
                current_date.tm_day = current_date.tm_day + 1;
            }

            current_date.tm_week = (current_date.tm_week + 1) % 7;
        }
    }
}

_attribute_ram_code_ uint8_t time_reached_period(timer_channel ch, uint32_t seconds)
{
    if (!has_ever_reached[ch])
    {
        has_ever_reached[ch] = 1;
        return 1;
    }
    if (current_unix_time - last_reached_period[ch] >= seconds)
    {
        last_reached_period[ch] = current_unix_time;
        return 1;
    }
    return 0;
}

_attribute_ram_code_ void set_time(uint32_t time_now, uint16_t time_year, uint8_t time_month, uint8_t time_day, uint8_t time_week)
{
    current_unix_time = time_now;
    current_date.tm_year = time_year;
    current_date.tm_month = time_month;
    current_date.tm_day = time_day;
    current_date.tm_week = time_week;
}

_attribute_ram_code_ struct date_time get_time(void) {
    return current_date;
}