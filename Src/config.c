#include "config.h"

// Configuration functions
void configureClock()
{
	// Clock Configuration - SYSCLK sourced from HSE - 8 Mhz
	RCC_CR |= (1 << 16); // HSEON -> 1
	while (!(RCC_CR & (1 << 17)))
		;
	RCC_CFGR &= ~(1U << 0);
	RCC_CFGR &= ~(1U << 1);
	RCC_CFGR |= (1 << 0);
	while ((RCC_CFGR & (0b11 << 2)) != (0b01 << 2))
		;
}

void configureBlueLED()
{
	// Enable the RCC Peripheral Clock for GPIOD
	RCC_AHB1ENR |= (1 << 3);
	// Set the PD15 GPIO Pin (Blue LED) to Output
	GPIOD_MODER &= ~(1 << 31);
	GPIOD_MODER &= ~(1U << 30);
	GPIOD_MODER |= (1 << 30);
}

void configureGreenLED()
{
	// Clock already enabled, since blue LED is also on GPIOD
	// Green LED: PD12
	GPIOD_MODER &= ~(1U << 24);
	GPIOD_MODER &= ~(1U << 25);
	GPIOD_MODER |= (1 << 24);
}

void configureOrangeLED()
{
	// Orange LED: PD13
	GPIOD_MODER &= ~(1U << 26);
	GPIOD_MODER &= ~(1U << 27);
	GPIOD_MODER |= (1 << 26);
}

void configureSystickInterrupts()
{
	SYSTICK_CSR &= ~(1U << 0);			// Disable timer
	SYSTICK_CSR |= (1 << 2);			// Use SYSCLK
	SYSTICK_CSR |= (1 << 1);			// Enable interrupt requests
	SYSTICK_RELOAD = 7999;				// 1 ms per interval
	SYSTICK_CURRENT = 0x00000000; // Reset current count
	SYSTICK_CSR |= (1 << 0);			// Enable Timer
}

void configureAll()
{
	configureClock();
	configureSystickInterrupts();
	configureBlueLED();
	configureGreenLED();
	configureOrangeLED();
}
