#include "system_funcs.h"

void systemENTER_CRITICAL() {
	// Mask priorities 0xE0 to 0xFF, which includes SysTick_Handler and PendSV_Handler
	__asm volatile(
			"MOV r0, #0xE0\n"
			"MSR BASEPRI, r0\n"
			"ISB\n");
}

void systemEXIT_CRITICAL() {
	__asm volatile(
			"MOV r0, #0x00\n"
			"MSR BASEPRI, r0\n"
			"ISB\n");
}

