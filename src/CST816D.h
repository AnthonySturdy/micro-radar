#ifndef _CST816D_H
#define _CST816D_H

#include <Wire.h>

#define I2C_ADDR_CST816D 0x15

//gesture
enum GESTURE
{
    None = 0x00,       
    SlideDown = 0x01,  
    SlideUp = 0x02,    
    SlideLeft = 0x03,  
    SlideRight = 0x04, 
    SingleTap = 0x05,  
    DoubleTap = 0x0B,  
    LongPress = 0x0C  
};

/**************************************************************************/
/*!
    @brief  CST816D I2C CTP controller driver
*/
/**************************************************************************/
class CST816D
{
public:
    CST816D(int8_t sda_pin = -1, int8_t scl_pin = -1, int8_t rst_pin = -1, int8_t int_pin = -1);

    void begin(void);
    bool getTouch(uint16_t *x, uint16_t *y, uint8_t *gesture);

private:
    int8_t _sda, _scl, _rst, _int;

    uint8_t i2c_read(uint8_t addr);
    uint8_t i2c_read_continuous(uint8_t addr, uint8_t *data, uint32_t length);
    void i2c_write(uint8_t addr, uint8_t data);
    uint8_t i2c_write_continuous(uint8_t addr, const uint8_t *data, uint32_t length);
};
#endif
