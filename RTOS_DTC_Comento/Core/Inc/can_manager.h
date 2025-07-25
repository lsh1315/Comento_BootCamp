/*
 * can_manager.h
 *
 *  Created on: 2025. 7. 25.
 *      Author: Gemini
 */

#ifndef INC_CAN_MANAGER_H_
#define INC_CAN_MANAGER_H_

#include "stm32f4xx_hal.h"

/* --- Defines --- */
#define CAN_DTC_TRANSMIT_ID   0x18FF50E5 // Example Extended CAN ID for DTC Transmission
#define CAN_DIAG_RECEIVE_ID   0x18DB33F1 // Example Extended CAN ID for Diagnostic Request

/* --- Enums --- */
// Commands received from the diagnostic tool
typedef enum {
    CMD_NONE = 0,
    CMD_CLEAR_DTC = 1,
    CMD_READ_DTC = 2,
} CAN_Command_t;

/* --- Public Function Prototypes --- */

/**
 * @brief Initializes the CAN Manager for transmission and reception filters.
 * @param hcan Pointer to a CAN_HandleTypeDef structure.
 * @retval HAL_StatusTypeDef HAL status.
 */
HAL_StatusTypeDef CAN_Manager_Init(CAN_HandleTypeDef* hcan);

/**
 * @brief Transmits DTC data over the CAN bus using interrupts.
 * @param hcan Pointer to a CAN_HandleTypeDef structure.
 * @param dtc_data Pointer to the DTC data buffer.
 * @param size The size of the DTC data in bytes.
 * @retval HAL_StatusTypeDef HAL status.
 */
HAL_StatusTypeDef CAN_Manager_Transmit_DTC(CAN_HandleTypeDef* hcan, uint8_t* dtc_data, uint16_t size);

/**
 * @brief Gets the last command received via CAN.
 * @retval The received command of type CAN_Command_t.
 */
CAN_Command_t CAN_Manager_Get_Command(void);

/**
 * @brief Clears the last received command.
 *        Should be called after a command has been processed.
 */
void CAN_Manager_Clear_Command(void);

#endif /* INC_CAN_MANAGER_H_ */
