#ifndef __DTC_MANAGER_H
#define __DTC_MANAGER_H

#include <stdint.h>
#include <stdbool.h>

/**
 * @brief Diagnostic Trouble Codes for the Brake System PMIC
 * @note These are example codes. Actual codes should follow automotive standards like SAE J2012.
 */
typedef enum {
    // PMIC Under-Voltage Faults
    DTC_PMIC_BUCK_A_UNDERVOLTAGE = 0,  // Fault code for Buck A rail
    DTC_PMIC_BUCK_B_UNDERVOLTAGE = 1,  // Fault code for Buck B rail
    DTC_PMIC_BUCK_C_UNDERVOLTAGE = 2,  // Fault code for Buck C rail
    DTC_PMIC_BUCK_D_UNDERVOLTAGE = 3,  // Fault code for Buck D rail

    // Add other DTCs for the system here...
    // e.g., DTC_PMIC_OVER_TEMPERATURE = 4,

    DTC_CODE_COUNT // Total number of DTCs, must be last
} DTC_Code_t;

/**
 * @brief Initializes the DTC manager.
 */
void DTC_Init(void);

/**
 * @brief Sets a specific DTC to indicate a fault has occurred.
 * @param code The DTC to set.
 */
void DTC_Set(DTC_Code_t code);

/**
 * @brief Clears a specific DTC, indicating a fault is resolved.
 * @param code The DTC to clear.
 */
void DTC_Clear(DTC_Code_t code);

/**
 * @brief Clears all DTCs.
 */
void DTC_ClearAll(void);

/**
 * @brief Checks if a specific DTC is currently set.
 * @param code The DTC to check.
 * @return true if the DTC is set, false otherwise.
 */
bool DTC_IsSet(DTC_Code_t code);

/**
 * @brief Gets the entire DTC status bitmask.
 * @return The 32-bit bitmask representing all DTC statuses.
 */
uint32_t DTC_GetStatusBitmask(void);

/**
 * @brief Sets the entire DTC status bitmask.
 * @param bitmask The 32-bit bitmask to restore DTC statuses from.
 */
void DTC_SetStatusBitmask(uint32_t bitmask);

#endif /* __DTC_MANAGER_H */
