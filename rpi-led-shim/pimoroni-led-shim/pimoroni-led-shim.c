#include "pimoroni-led-shim.h"
#include <unistd.h>
#include <string.h>

#define Addr_GND 0x75
static float brightness = 1;

void set_brightness(float new_brightness) {
    brightness = new_brightness;
}


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
static const uint8_t LED_GAMMA[256] = {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2,
        2, 2, 2, 3, 3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5,
        6, 6, 6, 7, 7, 7, 8, 8, 8, 9, 9, 9, 10, 10, 11, 11,
        11, 12, 12, 13, 13, 13, 14, 14, 15, 15, 16, 16, 17, 17, 18, 18,
        19, 19, 20, 21, 21, 22, 22, 23, 23, 24, 25, 25, 26, 27, 27, 28,
        29, 29, 30, 31, 31, 32, 33, 34, 34, 35, 36, 37, 37, 38, 39, 40,
        40, 41, 42, 43, 44, 45, 46, 46, 47, 48, 49, 50, 51, 52, 53, 54,
        55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70,
        71, 72, 73, 74, 76, 77, 78, 79, 80, 81, 83, 84, 85, 86, 88, 89,
        90, 91, 93, 94, 95, 96, 98, 99, 100, 102, 103, 104, 106, 107, 109, 110,
        111, 113, 114, 116, 117, 119, 120, 121, 123, 124, 126, 128, 129, 131, 132, 134,
        135, 137, 138, 140, 142, 143, 145, 146, 148, 150, 151, 153, 155, 157, 158, 160,
        162, 163, 165, 167, 169, 170, 172, 174, 176, 178, 179, 181, 183, 185, 187, 189,
        191, 193, 194, 196, 198, 200, 202, 204, 206, 208, 210, 212, 214, 216, 218, 220,
        222, 224, 227, 229, 231, 233, 235, 237, 239, 241, 244, 246, 248, 250, 252, 255};

static inline uint8_t correct_color(uint8_t c ) {
    return LED_GAMMA[(int)(c * brightness)];

}
void set_pixel(int x, uint8_t r, uint8_t g, uint8_t b) {
    //todo gamma control
    colors[x * 3] = correct_color(r);
    colors[x * 3 + 1] = correct_color(g);
    colors[x * 3 + 2] = correct_color(b);
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
        I2C_BufferWrite(output + offset, chunk_size, 0x24 + offset, Addr_GND);
    }

    I2C_WriteByte(Addr_GND, 0xfd, 0xb);//functions(9th) frame
    I2C_WriteByte(Addr_GND, 0x01, page);//switch frame
    page = 1 - page;
}
