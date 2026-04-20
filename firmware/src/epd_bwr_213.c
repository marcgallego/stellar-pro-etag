#include <stdint.h>
#include "tl_common.h"
#include "main.h"
#include "epd.h"
#include "epd_spi.h"
#include "epd_bwr_213.h"
#include "drivers.h"
#include "stack/ble/ble.h"

// UC8151C / JD79656 EPD Controller (NOT SSD1680)

enum PSR_FLAGS
{
    RES_96x230 = 0b00000000,
    RES_96x252 = 0b01000000,
    RES_128x296 = 0b10000000,
    RES_160x296 = 0b11000000,

    LUT_OTP = 0b00000000,
    LUT_REG = 0b00100000,

    FORMAT_BWR = 0b00000000,
    FORMAT_BW = 0b00010000,

    SCAN_DOWN = 0b00000000,
    SCAN_UP = 0b00001000,

    SHIFT_LEFT = 0b00000000,
    SHIFT_RIGHT = 0b00000100,

    BOOSTER_OFF = 0b00000000,
    BOOSTER_ON = 0b00000010,

    RESET_SOFT = 0b00000000,
    RESET_NONE = 0b00000001
};

#define BWR_213_Len 10
uint8_t LUT_bwr_213_20_part[] = {
    0x20, 0x00, BWR_213_Len, 0x01, 0x00, 0x00, 0x01,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00};
uint8_t LUT_bwr_213_22_part[] = {
    0x22, 0x80, BWR_213_Len, 0x01, 0x00, 0x00, 0x01,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
uint8_t LUT_bwr_213_23_part[] = {
    0x23, 0x40, BWR_213_Len, 0x01, 0x00, 0x00, 0x01,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

#define scan_direction (SCAN_UP | RES_160x296 | FORMAT_BWR | BOOSTER_ON | RESET_NONE | LUT_OTP | SHIFT_RIGHT)

_attribute_ram_code_ uint8_t EPD_BWR_213_detect(void)
{
    return 0;
}

_attribute_ram_code_ uint8_t EPD_BWR_213_read_temp(void)
{
    uint8_t epd_temperature = 0;

    // power on
    EPD_WriteCmd(0x04);
    EPD_CheckStatus(100);

    // read temperature
    EPD_WriteCmd(0x40);
    epd_temperature = EPD_SPI_read();
    EPD_SPI_read();

    // power off
    EPD_WriteCmd(0x02);

    // deep sleep
    EPD_WriteCmd(0x07);
    EPD_WriteData(0xa5);

    return epd_temperature;
}

_attribute_ram_code_ uint8_t EPD_BWR_213_Display(unsigned char *image, unsigned char *red_image, int size, uint8_t full_or_partial)
{
    uint8_t epd_temperature = 0;
    int i;

    // booster soft start
    EPD_WriteCmd(0x06);
    EPD_WriteData(0x17);
    EPD_WriteData(0x17);
    EPD_WriteData(0x17);

    // power on
    EPD_WriteCmd(0x04);
    EPD_CheckStatus(100);

    // read temperature
    EPD_WriteCmd(0x40);
    epd_temperature = EPD_SPI_read();
    EPD_SPI_read();

    // panel setting
    EPD_WriteCmd(0x00);
    if (full_or_partial)
        EPD_WriteData(scan_direction);
    else
        EPD_WriteData(scan_direction | LUT_REG);
    EPD_WriteData(0x0f);

    if (!full_or_partial)
    {
        EPD_send_lut(LUT_bwr_213_20_part, sizeof(LUT_bwr_213_20_part));
        EPD_send_empty_lut(0x21, 260);
        EPD_send_lut(LUT_bwr_213_22_part, sizeof(LUT_bwr_213_22_part));
        EPD_send_lut(LUT_bwr_213_23_part, sizeof(LUT_bwr_213_23_part));
        EPD_send_empty_lut(0x24, 260);
    }

    // B/W data
    EPD_WriteCmd(0x10);
    for (i = 0; i < size; i++)
    {
        EPD_WriteData(image[i]);
    }

    // Red data
    EPD_WriteCmd(0x13);
    if (red_image != NULL)
    {
        for (i = 0; i < size; i++)
        {
            EPD_WriteData(red_image[i]);
        }
    }
    else
    {
        for (i = 0; i < size; i++)
        {
            EPD_WriteData(0x00);
        }
    }

    // trigger display refresh
    EPD_WriteCmd(0x12);

    return epd_temperature;
}

_attribute_ram_code_ void EPD_BWR_213_set_sleep(void)
{
    // Vcom and data interval setting
    EPD_WriteCmd(0x50);
    EPD_WriteData(0xf7);

    // power off
    EPD_WriteCmd(0x02);

    // deep sleep
    EPD_WriteCmd(0x07);
    EPD_WriteData(0xa5);
}
