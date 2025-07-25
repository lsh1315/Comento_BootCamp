#include "dtc_manager.h"

// We can use a bitmask to store the status of all DTCs.
// A 32-bit integer can hold up to 32 DTC statuses.
// For more DTCs, an array of uint32_t can be used.
static uint32_t dtc_status_bitmask = 0;

/**
 * @brief Initializes the DTC manager.
 */
void DTC_Init(void)
{
    // In a real application, this function should read the last known DTC status
    // from non-volatile memory (e.g., EEPROM) and restore it.
    // For now, we just clear it.
    // Example: dtc_status_bitmask = EEPROM_Read(DTC_STATUS_ADDRESS);
    dtc_status_bitmask = 0;
}

/**
 * @brief Sets a specific DTC to indicate a fault has occurred.
 * @param code The DTC to set.
 */
void DTC_Set(DTC_Code_t code)
{
    if (code < DTC_CODE_COUNT) {
        uint32_t old_bitmask = dtc_status_bitmask;
        dtc_status_bitmask |= (1UL << code);

        // If the status has changed, save it to non-volatile memory.
        if (old_bitmask != dtc_status_bitmask) {
            // Example: EEPROM_Write(DTC_STATUS_ADDRESS, dtc_status_bitmask);
        }
    }
}

/**
 * @brief Clears a specific DTC, indicating a fault is resolved.
 * @param code The DTC to clear.
 */
void DTC_Clear(DTC_Code_t code)
{
    if (code < DTC_CODE_COUNT) {
        uint32_t old_bitmask = dtc_status_bitmask;
        dtc_status_bitmask &= ~(1UL << code);

        // If the status has changed, save it to non-volatile memory.
        if (old_bitmask != dtc_status_bitmask) {
            // Example: EEPROM_Write(DTC_STATUS_ADDRESS, dtc_status_bitmask);
        }
    }
}

/**
 * @brief Checks if a specific DTC is currently set.
 * @param code The DTC to check.
 * @return true if the DTC is set, false otherwise.
 */
bool DTC_IsSet(DTC_Code_t code)
{
    if (code < DTC_CODE_COUNT) {
        return (dtc_status_bitmask & (1UL << code)) != 0;
    }
    return false;
}

/**
 * @brief Gets the entire DTC status bitmask.
 * @return The 32-bit bitmask representing all DTC statuses.
 */
uint32_t DTC_GetStatusBitmask(void)
{
    return dtc_status_bitmask;
}

/**
 * @brief Sets the entire DTC status bitmask.
 * @param bitmask The 32-bit bitmask to restore DTC statuses from.
 */
void DTC_SetStatusBitmask(uint32_t bitmask)
{
    dtc_status_bitmask = bitmask;
}
