#ifndef FAULT_H_
#define FAULT_H_

#include <stdint.h>

/*
 * Users are highly recommended to implement fault handlers.
 * These helper functions are provided to help recover and troubleshoot
 * faults.
 * */

__attribute((naked)) uint32_t *systemGet_Fault_SP(__attribute__((unused)) uint32_t faultLR);
void systemHandle_Fault(uint32_t *faultSP);

/*
 * ****USER CODE SECTION START****
 * Place your fault handler definitions here
 * */

__attribute__((naked)) void HardFault_Handler();

/*
 * ****USER CODE SECTION END****
 * */

#endif
