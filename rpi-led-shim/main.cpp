#include <cstdio>
#include <fcntl.h>
#include <cstdlib>
#include <unistd.h>

#include <sys/ioctl.h>
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


int main() {
    setupI2C("/dev/i2c-1");
    IS31FL3731_Init();
    printf("Hello, LED World!\r\n");
    for (int i = 0; true; i = (i + 1) % 28) {

        line_fill(0, 0, 0);
        set_pixel(i, 255, 0, 0);
        flip_frame();
        usleep(5000);

        line_fill(0, 0, 0);
        set_pixel(i, 0, 255, 0);
        flip_frame();
        usleep(5000);

        line_fill(0, 0, 0);
        set_pixel(i, 0, 0, 255);
        flip_frame();
        usleep(5000);
    }
    return 0;
}
