#ifndef __IS31FL3731_H
#define __IS31FL3731_H 

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

void I2C_WriteByte(int DeviceAddress, int WriteAddress, int SendByte);

void I2C_BufferWrite(const uint8_t *pBuffer, int length, int WriteAddress, int DeviceAddress);

void IS31FL3731_Init(void);


void line_fill(uint8_t r, uint8_t g, uint8_t b);

void set_pixel(int x, uint8_t r, uint8_t g, uint8_t b);

void flip_frame();

#ifdef __cplusplus
}
#endif


#endif
