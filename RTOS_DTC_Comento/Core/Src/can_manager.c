/*
 * can_manager.c
 *
 *  Created on: 2025. 7. 25.
 *      Author: Gemini
 */

#include "can_manager.h"
#include "main.h" // For CAN_HandleTypeDef

// --- Private Variables ---
static CAN_TxHeaderTypeDef tx_header;
static CAN_RxHeaderTypeDef rx_header;
static uint8_t rx_data[8];

// State for interrupt-driven transmission
static uint8_t* p_tx_data = NULL;
static volatile uint16_t tx_data_size = 0;
static volatile uint16_t tx_data_sent_count = 0;
static volatile uint8_t is_tx_in_progress = 0;

// State for received command
static volatile CAN_Command_t received_command = CMD_NONE;

// --- Private Function Prototypes ---
static void CAN_Send_Next_Frame(CAN_HandleTypeDef* hcan);
static void Process_CAN_Response(uint8_t* data);

// --- Public API Functions ---

HAL_StatusTypeDef CAN_Manager_Init(CAN_HandleTypeDef* hcan)
{
    CAN_FilterTypeDef sFilterConfig;

    // Configure CAN filter for diagnostic requests
    sFilterConfig.FilterBank = 0;
    sFilterConfig.FilterMode = CAN_FILTERMODE_IDMASK;
    sFilterConfig.FilterScale = CAN_FILTERSCALE_32BIT;
    sFilterConfig.FilterIdHigh = (CAN_DIAG_RECEIVE_ID >> 13) & 0xFFFF;
    sFilterConfig.FilterIdLow = ((CAN_DIAG_RECEIVE_ID << 3) | CAN_ID_EXT) & 0xFFF8;
    sFilterConfig.FilterMaskIdHigh = 0xFFFF;
    sFilterConfig.FilterMaskIdLow = 0xFFF8;
    sFilterConfig.FilterFIFOAssignment = CAN_RX_FIFO0;
    sFilterConfig.FilterActivation = ENABLE;
    sFilterConfig.SlaveStartFilterBank = 14; // Meaningless for single CAN

    if (HAL_CAN_ConfigFilter(hcan, &sFilterConfig) != HAL_OK) {
        return HAL_ERROR;
    }

    // Configure transmission header
    tx_header.ExtId = CAN_DTC_TRANSMIT_ID;
    tx_header.IDE = CAN_ID_EXT;
    tx_header.RTR = CAN_RTR_DATA;
    tx_header.DLC = 8;
    tx_header.TransmitGlobalTime = DISABLE;

    if (HAL_CAN_Start(hcan) != HAL_OK) {
        return HAL_ERROR;
    }

    // Activate both TX and RX notifications
    if (HAL_CAN_ActivateNotification(hcan, CAN_IT_TX_MAILBOX_EMPTY | CAN_IT_RX_FIFO0_MSG_PENDING) != HAL_OK) {
        return HAL_ERROR;
    }

    return HAL_OK;
}

HAL_StatusTypeDef CAN_Manager_Transmit_DTC(CAN_HandleTypeDef* hcan, uint8_t* dtc_data, uint16_t size)
{
    if (is_tx_in_progress) {
        return HAL_BUSY;
    }
    if (size == 0 || dtc_data == NULL) {
        return HAL_ERROR;
    }

    p_tx_data = dtc_data;
    tx_data_size = size;
    tx_data_sent_count = 0;
    is_tx_in_progress = 1;

    CAN_Send_Next_Frame(hcan);

    return HAL_OK;
}

CAN_Command_t CAN_Manager_Get_Command(void)
{
    return received_command;
}

void CAN_Manager_Clear_Command(void)
{
    received_command = CMD_NONE;
}

// --- Private Helper Functions ---

static void CAN_Send_Next_Frame(CAN_HandleTypeDef* hcan)
{
    if (tx_data_sent_count >= tx_data_size) {
        is_tx_in_progress = 0;
        return;
    }

    uint32_t tx_mailbox;
    uint8_t tx_payload[8] = {0};
    uint8_t bytes_to_send = (tx_data_size - tx_data_sent_count >= 8) ? 8 : (tx_data_size - tx_data_sent_count);

    for (int i = 0; i < bytes_to_send; i++) {
        tx_payload[i] = p_tx_data[tx_data_sent_count + i];
    }
    tx_header.DLC = bytes_to_send;

    if (HAL_CAN_AddTxMessage(hcan, &tx_header, tx_payload, &tx_mailbox) == HAL_OK) {
        tx_data_sent_count += bytes_to_send;
    } else {
        is_tx_in_progress = 0;
    }
}

/**
 * @brief Processes the data received from the diagnostic tool.
 */
static void Process_CAN_Response(uint8_t* data)
{
    // Example diagnostic frame: data[0] is command type
    // 0x31: Clear DTC, 0x19: Read DTC
    if (data[0] == 0x31) { // A simplified UDS-like command
        received_command = CMD_CLEAR_DTC;
    } else if (data[0] == 0x19) {
        received_command = CMD_READ_DTC;
    }
}

// --- HAL CAN Callback Functions ---

void HAL_CAN_TxMailbox0CompleteCallback(CAN_HandleTypeDef *hcan)
{
    if (is_tx_in_progress) CAN_Send_Next_Frame(hcan);
}

void HAL_CAN_TxMailbox1CompleteCallback(CAN_HandleTypeDef *hcan)
{
    if (is_tx_in_progress) CAN_Send_Next_Frame(hcan);
}

void HAL_CAN_TxMailbox2CompleteCallback(CAN_HandleTypeDef *hcan)
{
    if (is_tx_in_progress) CAN_Send_Next_Frame(hcan);
}

/**
  * @brief  FIFO 0 message pending callback.
  * @param  hcan: pointer to a CAN_HandleTypeDef structure.
  * @retval None
  */
void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan)
{
    if (HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &rx_header, rx_data) == HAL_OK) {
        // Check if the message is for us (optional, as filter should handle it)
        if (rx_header.ExtId == CAN_DIAG_RECEIVE_ID) {
            Process_CAN_Response(rx_data);
        }
    }
}
