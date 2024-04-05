#include <cmath>
#include <cstdio>
#include <fcntl.h>
#include <cstdlib>
#include <unistd.h>

#include "pimoroni-led-shim.h"

#include <linux/i2c-dev.h>
#include "i2c/i2c.h"

static I2CDevice i2cDevice{.bus=-1, .addr=0x75, .tenbit=0, .delay=1, .flags=0, .page_bytes=8, .iaddr_bytes=1};

void I2C_WriteByte(int DeviceAddress, int WriteAddress, int SendByte) {
    i2c_write(&i2cDevice, WriteAddress, &SendByte, 1);
}

void I2C_BufferWrite(const uint8_t *pBuffer, int length, int WriteAddress, int DeviceAddress) {
    i2c_write(&i2cDevice, WriteAddress, pBuffer, length);
}

void setupI2C(const char *name) {
    i2cDevice.bus = i2c_open(name);
    if (i2cDevice.bus < 0) {
        fprintf(stderr, "Cannot open file %s", name);
        exit(1);
    }
}


typedef struct RGBColor {
    int r;
    int g;
    int b;
} RGBColor;

RGBColor hsv2rgb(float H, float S, float V) {
    float r, g, b;

    float h = H / 360;
    float s = S / 100;
    float v = V / 100;

    int i = floor(h * 6);
    float f = h * 6 - i;
    float p = v * (1 - s);
    float q = v * (1 - f * s);
    float t = v * (1 - (1 - f) * s);

    switch (i % 6) {
        case 0:
            r = v, g = t, b = p;
            break;
        case 1:
            r = q, g = v, b = p;
            break;
        case 2:
            r = p, g = v, b = t;
            break;
        case 3:
            r = p, g = q, b = v;
            break;
        case 4:
            r = t, g = p, b = v;
            break;
        case 5:
            r = v, g = p, b = q;
            break;
    }

    RGBColor color;
    color.r = r * 255;
    color.g = g * 255;
    color.b = b * 255;

    return color;
}

[[noreturn]]int main() {
    setupI2C("/dev/i2c-1");
    IS31FL3731_Init();
    printf("Hello, LED World!\r\n");
    set_brightness(0.6);

    float spacing = 360.0 / 16.0;
    unsigned long hue = 0;

    for (unsigned long cnt = 0; true; ++cnt) {
        hue = cnt % 360;
        for (int x = 0; x < 28; ++x) {
            int offset = x * spacing;
            float h = ((hue + offset) % 360);
            RGBColor color = hsv2rgb(h, 100, 100);
            set_pixel(x, color.r, color.g, color.b);
        }
        flip_frame();
        usleep(3000);
    }
}


