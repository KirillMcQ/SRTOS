#include "fault.h"

__attribute((naked)) uint32_t *systemGet_Fault_SP(__attribute((unused)) uint32_t faultLR)
{
	__asm volatile(
			"TST r0, #4\n"
			"ITE eq\n"
			"MRSEQ r0, msp\n"
			"MRSNE r0, psp\n"
			"BX lr\n");
}

/*
 * This must be called in every fault handler.
 * This may be implemented however is needed.
 * */

void systemHandle_Fault(uint32_t *faultSP)
{
	(void) faultSP;
	while (1);
}


/*
 * ****USER CODE SECTION START****
 * Implement your fault handlers here.
 * */

__attribute__((naked)) void HardFault_Handler() {
	__asm volatile(
			"MOV r0, lr\n"
			"BL systemGet_Fault_SP\n"
			"LDR r1, =systemHandle_Fault\n"
			"BX r1\n");
}

/*
 * ****USER CODE SECTION END****
 * */
