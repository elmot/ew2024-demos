#include "pimoroni-led-shim.h"
#include <unistd.h>
#include <string.h>

#define Addr_GND 0x75

static inline void Delay_ms(unsigned int ms) {
    usleep(ms * 1000);
}

static uint8_t colors[28 * 3];
static uint8_t output[144];
static const uint8_t all_led_on_IS31FL3731[18] = {
        0b00000000, 0b10111111,
        0b00111110, 0b00111110,
        0b00111111, 0b10111110,
        0b00000111, 0b10000110,
        0b00110000, 0b00110000,
        0b00111111, 0b10111110,
        0b00111111, 0b10111110,
        0b01111111, 0b11111110,
        0b01111111, 0b00000000,
};

void IS31FL3731_Init(void) {
    uint8_t i;

    I2C_WriteByte(Addr_GND, 0xfd, 0x0b);//function registers
    I2C_WriteByte(Addr_GND, 0x0A, 0x00);
    for (int page = 0; page < 2; ++page) {
        I2C_WriteByte(Addr_GND, 0xfd, page);//write page 0
        I2C_BufferWrite(all_led_on_IS31FL3731, 18, 0x00, Addr_GND); //enable LEDS
        for (i = 0x24; i < 0xB4; i++) {
            I2C_WriteByte(Addr_GND, i, 0x00);//PWM set
        }
    }
    I2C_WriteByte(Addr_GND, 0xfd, 0x0b);//write function register
    I2C_WriteByte(Addr_GND, 0x0a, 0x01);//normal mode
    I2C_WriteByte(Addr_GND, 0x00, 0x00);//normal mode
    I2C_WriteByte(Addr_GND, 0x01, 0x00);//normal mode
    I2C_WriteByte(Addr_GND, 0x02, 0x00);//normal mode
    I2C_WriteByte(Addr_GND, 0xfd, 0x00);//write function register
}

static int page = 0;


void line_fill(uint8_t r, uint8_t g, uint8_t b) {
    for (int i = 0; i < 28; ++i) {
        set_pixel(i, r, g, b);
    }
}

static const uint8_t pixel_map[28 * 3] =
        {
                118, 69, 85,
                117, 68, 101,
                116, 84, 100,
                115, 83, 99,
                114, 82, 98,
                113, 81, 97,
                112, 80, 96,
                134, 21, 37,
                133, 20, 36,
                132, 19, 35,
                131, 18, 34,
                130, 17, 50,
                129, 33, 49,
                128, 32, 48,

                127, 47, 63,
                121, 41, 57,
                122, 25, 58,
                123, 26, 42,
                124, 27, 43,
                125, 28, 44,
                126, 29, 45,
                15, 95, 111,
                8, 89, 105,
                9, 90, 106,
                10, 91, 107,
                11, 92, 108,
                12, 76, 109,
                13, 77, 93,
        };

void set_pixel(int x, uint8_t r, uint8_t g, uint8_t b) {
    //todo gamma control
    colors[x * 3] = r;
    colors[x * 3 + 1] = g;
    colors[x * 3 + 2] = b;
}

void flip_frame() {
    memset(output, 0, 144);
    for (int i = 0; i < sizeof pixel_map; ++i) {
        output[pixel_map[i]] = colors[i];
    }
    I2C_WriteByte(Addr_GND, 0xfd, page);//write page 0 or 1
//    I2C_BufferWrite(all_led_on_IS31FL3731, 18, 0x00, Addr_GND); //enable LEDS

    const int chunk_size = 144;
    for (int offset = 0; offset < 144; offset += chunk_size) {
        I2C_BufferWrite(output + offset,  chunk_size, 0x24 + offset, Addr_GND);
    }

    I2C_WriteByte(Addr_GND, 0xfd, 0xb);//functions(9th) frame
    I2C_WriteByte(Addr_GND, 0x01, page);//switch frame
    page = 1 - page;
}
