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
 * Gets stack frame and pushes to non-volatile flash memory.
 * */

void systemHandle_Fault(uint32_t *faultSP)
{
	FLASH_KEYR = FLASH_UNLOCK_KEY1;
	FLASH_KEYR = FLASH_UNLOCK_KEY2;

	while (FLASH_SR & (1U << FLASH_SR_BSY_BIT));

	// PSIZE -> 0b10 which is a 32 bit parallelism size
	FLASH_CR &= ~(1U << FLASH_CR_PSIZE_BIT_START);
	FLASH_CR &= ~(1U << (FLASH_CR_PSIZE_BIT_START + 1));
	FLASH_CR |= (1U << (FLASH_CR_PSIZE_BIT_START + 1));

	while (FLASH_SR & (1U << FLASH_SR_BSY_BIT));

	// Activate Sector Erase
	FLASH_CR |= (1U << FLASH_CR_SER_BIT);

	// Erase Sector 7 (FAULT_DATA)
	FLASH_CR &= ~(0xFU << FLASH_CR_SNB_BIT_START);
	FLASH_CR |= (0x7U << FLASH_CR_SNB_BIT_START);

	// Start erasing
	FLASH_CR |= (1U << FLASH_CR_STRT_BIT);

	while (FLASH_SR & (1U << FLASH_SR_BSY_BIT));

	// Program data to flash
	FLASH_CR |= (1U << FLASH_CR_PG_BIT);

	/*
	 * Push order:
	 * r0
	 * r1
	 * r2
	 * r3
	 * r12
	 * lr
	 * pc
	 * psr
	 * */

	volatile uint32_t *curWriteAddr = (volatile uint32_t *) FAULT_DATA_FLASH_START_ADDR;
	for (int i = 0; i < 8; i++) {
		*(curWriteAddr) = faultSP[i];
		curWriteAddr += 1;
	}

	while (FLASH_SR & (1U << FLASH_SR_BSY_BIT));

	while (1);
}

__attribute__((naked)) void HardFault_Handler()
{
	__asm volatile(
			"MOV r0, lr\n"
			"BL systemGet_Fault_SP\n"
			"LDR r1, =systemHandle_Fault\n"
			"BX r1\n");
}
