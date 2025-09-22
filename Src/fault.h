#ifndef FAULT_H_
#define FAULT_H_

#include <stdint.h>

/*
 * These helper functions are provided to help troubleshoot
 * faults. Only HardFault is enabled by default, but this can
 * be changed by the user if needed.
 * */

__attribute((naked)) uint32_t *systemGet_Fault_SP(__attribute__((unused)) uint32_t faultLR);
void systemHandle_Fault(uint32_t *faultSP);

__attribute__((naked)) void HardFault_Handler();

#endif
