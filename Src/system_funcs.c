/**
 * @file    system_funcs.c
 * @brief   System-level control functions for SRTOS.
 * @details
 * Implements low-level system functions used by the kernel.
 */

#include "system_funcs.h"

/**
 * @brief Enter a critical section by disabling all interrupts.
 * 
 * @warning This function should not be called by user code.
 */
void
systemENTER_CRITICAL ()
{
  /* Mask priorities 0xE0 to 0xFF, which includes
   * SysTick_Handler and PendSV_Handler
   */
  __asm volatile ("MOV r0, #0xE0\n"
                  "MSR BASEPRI, r0\n"
                  "ISB\n");
}

/**
 * @brief Exit a critical section by enabling all interrupts.
 * 
 * @warning This function should not be called by user code.
 */
void
systemEXIT_CRITICAL ()
{
  __asm volatile ("MOV r0, #0x00\n"
                  "MSR BASEPRI, r0\n"
                  "ISB\n");
}
