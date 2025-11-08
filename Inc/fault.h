/**
 * @file    fault.h
 * @brief   Fault handling interface for SRTOS.
 * @details
 * Declares fault handling routines for capturing and processing
 * system exceptions.
 */

#ifndef FAULT_H_
#define FAULT_H_

#include "mcu_macros.h"
#include <stdint.h>

/**
 * @brief Returns the Stack Pointer after a fault.
 * @param faultLR This is the link register, which will be used to return to the function's caller. It is marked unused to suppress the unused paramter compiler warning.
 * 
 * 
 * @note This function will only be called by Inline Assembly.
 * @warning This function should not be called by user code.
 */
__attribute ((naked)) uint32_t *
systemGet_Fault_SP (__attribute__ ((unused)) uint32_t faultLR);

/**
 * @brief This function will handle a system fault. The default behavior is to log the fault SP to non-volatile memory. 
 * @details This function can be editted as the user needs. There is not one specific action that needs to be done here, it just depends
 * on how the user wants to handle faults.
 * 
 * @param faultSP The fault Stack Pointer that, by default, will be written to non-volatile memory. This will be the best representation of why the system faulted.
 * 
 * @warning This function must not return.
 * @warning This function should not be called by user code.
 * */
void systemHandle_Fault (uint32_t *faultSP);

/**
 * @brief This function will get the fault Stack Pointer and call the fault handler.
 * 
 * @note This function is merely an intermediary step to the fault handler. The user should not need to change this function.
 * @warning This function should not be called by user code.
 */
__attribute__ ((naked)) void HardFault_Handler ();

#endif
