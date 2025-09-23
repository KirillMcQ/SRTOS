#include "fault.h"

__attribute((naked)) uint32_t *systemGet_Fault_SP(__attribute__((unused)) uint32_t faultLR)
{
	__asm volatile(
			"TST r0, #4\n"
			"ITE eq\n"
			"MRSEQ r0, msp\n"
			"MRSNE r0, psp\n"
			"BX lr\n");
}

/*
 * Gets stack frame and pushes to non-volatile flash memory
 * then waits for Watchdog to reset.
 * */

void systemHandle_Fault(uint32_t *faultSP)
{
	uint32_t r0;
	uint32_t r1;
	uint32_t r2;
	uint32_t r3;
	uint32_t r12;
	uint32_t lr;
	uint32_t pc;
	uint32_t psr;

	r0 = faultSP[0];
	r1 = faultSP[1];
	r2 = faultSP[2];
	r3 = faultSP[3];
	r12 = faultSP[4];
	lr = faultSp[5];
	pc = faultSP[6];
	psr = faultSP[7];

	FLASH_KEYR = FLASH_UNLOCK_KEY1;
	FLASH_KEYR = FLASH_UNLOCK_KEY2;

	while (FLASH_SR & (1 << FLASH_SR_BSY_BIT))
		;

	while (1)
		;
}

__attribute__((naked)) void HardFault_Handler()
{
	__asm volatile(
			"MOV r0, lr\n"
			"BL systemGet_Fault_SP\n"
			"LDR r1, =systemHandle_Fault\n"
			"BX r1\n");
}
