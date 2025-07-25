/*
 * eeprom_25lc256.c
 *
 *  Created on: 2025. 7. 25.
 *      Author: Gemini
 */

#include "eeprom_25lc256.h"
#include "main.h" // For HAL_Delay

// Module-level static variables
static SPI_HandleTypeDef* s_hspi;
static GPIO_TypeDef* s_cs_port;
static uint16_t s_cs_pin;

// Volatile flag for DMA transfer completion
static volatile uint8_t spi_dma_tx_complete = 0;
static volatile uint8_t spi_dma_rx_complete = 0;

// --- Private Helper Functions ---

/**
 * @brief Pulls the Chip Select (CS) pin LOW.
 */
static void EEPROM_CS_Low(void)
{
    HAL_GPIO_WritePin(s_cs_port, s_cs_pin, GPIO_PIN_RESET);
}

/**
 * @brief Pulls the Chip Select (CS) pin HIGH.
 */
static void EEPROM_CS_High(void)
{
    HAL_GPIO_WritePin(s_cs_port, s_cs_pin, GPIO_PIN_SET);
}

/**
 * @brief Sends the Write Enable (WREN) command to the EEPROM.
 */
static void EEPROM_WriteEnable(void)
{
    uint8_t cmd = EEPROM_CMD_WREN;
    EEPROM_CS_Low();
    HAL_SPI_Transmit(s_hspi, &cmd, 1, HAL_MAX_DELAY);
    EEPROM_CS_High();
}

/**
 * @brief Polls the EEPROM's status register until the Write-In-Progress (WIP) bit is cleared.
 */
static void EEPROM_WaitForWriteComplete(void)
{
    uint8_t cmd = EEPROM_CMD_RDSR;
    uint8_t status = 0;

    EEPROM_CS_Low();
    HAL_SPI_Transmit(s_hspi, &cmd, 1, HAL_MAX_DELAY);
    do {
        HAL_SPI_Receive(s_hspi, &status, 1, HAL_MAX_DELAY);
    } while (status & EEPROM_WIP_BIT);
    EEPROM_CS_High();
}

// --- Public API Functions ---

void EEPROM_Init(SPI_HandleTypeDef* hspi, GPIO_TypeDef* cs_port, uint16_t cs_pin)
{
    s_hspi = hspi;
    s_cs_port = cs_port;
    s_cs_pin = cs_pin;

    // Ensure CS is high initially
    EEPROM_CS_High();
}

HAL_StatusTypeDef EEPROM_Read_DMA(uint16_t address, uint8_t* p_data, uint16_t size)
{
    uint8_t header[3];
    header[0] = EEPROM_CMD_READ;
    header[1] = (address >> 8) & 0xFF; // MSB
    header[2] = address & 0xFF;        // LSB

    spi_dma_rx_complete = 0;

    EEPROM_CS_Low();
    // Send Read command and address
    if (HAL_SPI_Transmit(s_hspi, header, 3, HAL_MAX_DELAY) != HAL_OK) {
        EEPROM_CS_High();
        return HAL_ERROR;
    }

    // Receive data using DMA
    if (HAL_SPI_Receive_DMA(s_hspi, p_data, size) != HAL_OK) {
        EEPROM_CS_High();
        return HAL_ERROR;
    }

    // Wait for DMA transfer to complete
    while (spi_dma_rx_complete == 0) {
        // You can add a timeout mechanism here if needed
    }

    EEPROM_CS_High();
    return HAL_OK;
}

HAL_StatusTypeDef EEPROM_Write_DMA(uint16_t address, uint8_t* p_data, uint16_t size)
{
    uint8_t header[3];
    uint16_t bytes_to_write;

    while (size > 0) {
        EEPROM_WriteEnable();

        uint16_t page_offset = address % EEPROM_PAGE_SIZE;
        uint16_t bytes_left_in_page = EEPROM_PAGE_SIZE - page_offset;
        bytes_to_write = (size > bytes_left_in_page) ? bytes_left_in_page : size;

        header[0] = EEPROM_CMD_WRITE;
        header[1] = (address >> 8) & 0xFF; // MSB
        header[2] = address & 0xFF;        // LSB

        spi_dma_tx_complete = 0;

        EEPROM_CS_Low();

        // Send Write command and address
        if (HAL_SPI_Transmit(s_hspi, header, 3, HAL_MAX_DELAY) != HAL_OK) {
            EEPROM_CS_High();
            return HAL_ERROR;
        }

        // Transmit data using DMA
        if (HAL_SPI_Transmit_DMA(s_hspi, p_data, bytes_to_write) != HAL_OK) {
            EEPROM_CS_High();
            return HAL_ERROR;
        }

        // Wait for DMA transfer to complete
        while (spi_dma_tx_complete == 0) {
            // You can add a timeout mechanism here if needed
        }

        EEPROM_CS_High();

        // Wait for the internal write cycle of the EEPROM to finish
        EEPROM_WaitForWriteComplete();

        address += bytes_to_write;
        p_data += bytes_to_write;
        size -= bytes_to_write;
    }

    return HAL_OK;
}

// --- HAL SPI DMA Callback Functions ---

/**
  * @brief  Tx Transfer completed callback.
  * @param  hspi: pointer to a SPI_HandleTypeDef structure that contains
  *               the configuration information for SPI module.
  * @retval None
  */
void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi)
{
    if (hspi->Instance == s_hspi->Instance) {
        spi_dma_tx_complete = 1;
    }
}

/**
  * @brief  Rx Transfer completed callback.
  * @param  hspi: pointer to a SPI_HandleTypeDef structure that contains
  *               the configuration information for SPI module.
  * @retval None
  */
void HAL_SPI_RxCpltCallback(SPI_HandleTypeDef *hspi)
{
    if (hspi->Instance == s_hspi->Instance) {
        spi_dma_rx_complete = 1;
    }
}
