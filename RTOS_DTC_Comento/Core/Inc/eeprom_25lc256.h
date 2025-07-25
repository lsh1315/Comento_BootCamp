/*
 * eeprom_25lc256.h
 *
 *  Created on: 2025. 7. 25.
 *      Author: Gemini
 */

#ifndef INC_EEPROM_25LC256_H_
#define INC_EEPROM_25LC256_H_

#include "stm32f4xx_hal.h"

/* EEPROM Commands */
#define EEPROM_CMD_READ  0x03  // Read data from memory
#define EEPROM_CMD_WRITE 0x02  // Write data to memory
#define EEPROM_CMD_WREN  0x06  // Set the write enable latch
#define EEPROM_CMD_WRDI  0x04  // Reset the write enable latch
#define EEPROM_CMD_RDSR  0x05  // Read Status register
#define EEPROM_CMD_WRSR  0x01  // Write Status register

/* EEPROM Status Register Bits */
#define EEPROM_WIP_BIT   0x01  // Write-In-Progress bit
#define EEPROM_WEL_BIT   0x02  // Write-Enable-Latch bit

/* EEPROM Memory Constants */
#define EEPROM_PAGE_SIZE    64    // 64 bytes per page
#define EEPROM_TOTAL_SIZE   32768 // 256Kbit = 32768 bytes

/**
 * @brief Initializes the EEPROM driver.
 * @param hspi Pointer to a SPI_HandleTypeDef structure that contains
 *             the configuration information for SPI module.
 * @param cs_port The GPIO port for the Chip Select pin.
 * @param cs_pin The GPIO pin for the Chip Select.
 */
void EEPROM_Init(SPI_HandleTypeDef* hspi, GPIO_TypeDef* cs_port, uint16_t cs_pin);

/**
 * @brief Reads a block of data from the EEPROM using DMA.
 * @note This function is blocking and will wait until the DMA transfer is complete.
 * @param address The starting address to read from.
 * @param p_data Pointer to the buffer that will receive the data.
 * @param size The number of bytes to read.
 * @retval HAL_StatusTypeDef HAL status.
 */
HAL_StatusTypeDef EEPROM_Read_DMA(uint16_t address, uint8_t* p_data, uint16_t size);

/**
 * @brief Writes a block of data to the EEPROM using DMA.
 * @note This function handles page boundaries automatically.
 *       It is blocking and will wait for the write and DMA transfers to complete.
 * @param address The starting address to write to.
 * @param p_data Pointer to the buffer containing the data to be written.
 * @param size The number of bytes to write.
 * @retval HAL_StatusTypeDef HAL status.
 */
HAL_StatusTypeDef EEPROM_Write_DMA(uint16_t address, uint8_t* p_data, uint16_t size);

#endif /* INC_EEPROM_25LC256_H_ */
