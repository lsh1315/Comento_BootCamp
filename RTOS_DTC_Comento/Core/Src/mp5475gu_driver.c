#include "mp5475gu_driver.h"

// Semaphore for I2C DMA synchronization
static osSemaphoreId_t i2cTxRxSemHandle;

/**
 * @brief  Initializes the MP5475GU driver, creating the semaphore.
 */
void mp5475gu_init(void)
{
    i2cTxRxSemHandle = osSemaphoreNew(1, 0, NULL); // Create a binary semaphore, initially taken
}

/**
 * @brief  Set the output voltage for a specific buck converter using DMA.
 * @param  hi2c: Pointer to the I2C handle.
 * @param  channel: The buck channel to configure (BUCK_A, BUCK_B, BUCK_C, or BUCK_D).
 * @param  voltage: The desired output voltage in volts.
 * @retval HAL status
 */
HAL_StatusTypeDef mp5475gu_set_vout(I2C_HandleTypeDef *hi2c, MP5475GU_BuckChannel_t channel, float voltage)
{
    uint8_t reg_high_addr;
    uint16_t vref_val;
    static uint8_t data[2]; // Use static to ensure data is valid during DMA transfer
    HAL_StatusTypeDef status;

    // Determine register addresses based on the channel
    // Vout High and Low registers are contiguous, so we only need the high address
    switch (channel) {
        case BUCK_A: reg_high_addr = MP5475GU_REG_VOUT_A_HIGH; break;
        case BUCK_B: reg_high_addr = MP5475GU_REG_VOUT_B_HIGH; break;
        case BUCK_C: reg_high_addr = MP5475GU_REG_VOUT_C_HIGH; break;
        case BUCK_D: reg_high_addr = MP5475GU_REG_VOUT_D_HIGH; break;
        default: return HAL_ERROR;
    }

    if (voltage < 0.3f || voltage > 2.048f) {
        return HAL_ERROR; // Voltage out of range
    }
    vref_val = (uint16_t)((voltage - 0.3f) / 0.002f);

    data[0] = (uint8_t)((vref_val >> 8) & 0x03);
    data[1] = (uint8_t)(vref_val & 0xFF);

    status = HAL_I2C_Mem_Write_DMA(hi2c, MP5475GU_I2C_ADDR, reg_high_addr, I2C_MEMADD_SIZE_8BIT, data, 2);
    if (status != HAL_OK) {
        return status;
    }

    // Wait for DMA transfer to complete
    if (osSemaphoreAcquire(i2cTxRxSemHandle, 100) != osOK) { // 100ms timeout
        return HAL_TIMEOUT;
    }

    return HAL_OK;
}

/**
 * @brief  Read the Under-Voltage (UV) status register using DMA.
 * @param  hi2c: Pointer to the I2C handle.
 * @param  status: Pointer to a MP5475GU_StatusUV_t union to store the status.
 * @retval HAL status
 */
HAL_StatusTypeDef mp5475gu_read_uv_status(I2C_HandleTypeDef *hi2c, MP5475GU_StatusUV_t *status)
{
    HAL_StatusTypeDef dma_status;

    dma_status = HAL_I2C_Mem_Read_DMA(hi2c, MP5475GU_I2C_ADDR, MP5475GU_REG_STATUS_UV, I2C_MEMADD_SIZE_8BIT, &status->data, 1);
    if (dma_status != HAL_OK) {
        return dma_status;
    }

    // Wait for DMA transfer to complete
    if (osSemaphoreAcquire(i2cTxRxSemHandle, 100) != osOK) { // 100ms timeout
        return HAL_TIMEOUT;
    }

    return HAL_OK;
}

/**
  * @brief  Memory Tx Transfer completed callback.
  * @param  hi2c Pointer to a I2C_HandleTypeDef structure that contains
  *                the configuration information for the specified I2C.
  * @retval None
  */
void HAL_I2C_MemTxCpltCallback(I2C_HandleTypeDef *hi2c)
{
    // This callback is shared among all I2C peripherals. Check if it's the one we're using.
    // For simplicity, we assume only one I2C is used for this driver.
    osSemaphoreRelease(i2cTxRxSemHandle);
}

/**
  * @brief  Memory Rx Transfer completed callback.
  * @param  hi2c Pointer to a I2C_HandleTypeDef structure that contains
  *                the configuration information for the specified I2C.
  * @retval None
  */
void HAL_I2C_MemRxCpltCallback(I2C_HandleTypeDef *hi2c)
{
    osSemaphoreRelease(i2cTxRxSemHandle);
}

/**
  * @brief  I2C error callback.
  * @param  hi2c Pointer to a I2C_HandleTypeDef structure that contains
  *                the configuration information for the specified I2C.
  * @retval None
  */
void HAL_I2C_ErrorCallback(I2C_HandleTypeDef *hi2c)
{
    // Handle I2C error: release the semaphore to unblock the waiting task
    osSemaphoreRelease(i2cTxRxSemHandle);
    // Optionally, log the error or set a flag
}
