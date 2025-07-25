#ifndef __MP5475GU_DRIVER_H
#define __MP5475GU_DRIVER_H

#include "main.h"
#include "cmsis_os.h"

// MP5475GU Default I2C Slave Address
#define MP5475GU_I2C_ADDR (0x60 << 1) // 7-bit address, left-shifted for HAL functions

// Register Addresses
typedef enum {
    MP5475GU_REG_VOUT_A_HIGH = 0x13,
    MP5475GU_REG_VOUT_A_LOW  = 0x14,
    MP5475GU_REG_VOUT_B_HIGH = 0x1B,
    MP5475GU_REG_VOUT_B_LOW  = 0x1C,
    MP5475GU_REG_VOUT_C_HIGH = 0x23,
    MP5475GU_REG_VOUT_C_LOW  = 0x24,
    MP5475GU_REG_VOUT_D_HIGH = 0x2B,
    MP5475GU_REG_VOUT_D_LOW  = 0x2C,
    MP5475GU_REG_STATUS_UV   = 0x07
} MP5475GU_Register_t;

// Buck Channel Selector
typedef enum {
    BUCK_A,
    BUCK_B,
    BUCK_C,
    BUCK_D
} MP5475GU_BuckChannel_t;

// STATUS_UV Register (0x07) Bit-field Structure
typedef union {
    uint8_t data;
    struct {
        uint8_t BUCKD_UV : 1;
        uint8_t BUCKC_UV : 1;
        uint8_t BUCKB_UV : 1;
        uint8_t BUCKA_UV : 1;
        uint8_t reserved : 4;
    } bits;
} MP5475GU_StatusUV_t;


// Function Prototypes
void mp5475gu_init(void);
HAL_StatusTypeDef mp5475gu_set_vout(I2C_HandleTypeDef *hi2c, MP5475GU_BuckChannel_t channel, float voltage);
HAL_StatusTypeDef mp5475gu_read_uv_status(I2C_HandleTypeDef *hi2c, MP5475GU_StatusUV_t *status);

#endif /* __MP5475GU_DRIVER_H */
