#include "config.h"

static void
configureClock ()
{
  /* Clock Configuration - SYSCLK sourced from HSE - 8 Mhz */
  RCC_CR |= (1 << 16); /* HSEON -> 1 */
  while (!(RCC_CR & (1 << 17)))
    ;
  RCC_CFGR &= ~(1U << 0);
  RCC_CFGR &= ~(1U << 1);
  RCC_CFGR |= (1 << 0);
  while ((RCC_CFGR & (0x3 << 2)) != (0x1 << 2))
    ;
}

static void
configureBlueLED ()
{
  /* Enable the RCC Peripheral Clock for GPIOD */
  RCC_AHB1ENR |= (1 << 3);
  /* Set the PD15 GPIO Pin (Blue LED) to Output */
  GPIOD_MODER &= ~(1U << 31);
  GPIOD_MODER &= ~(1U << 30);
  GPIOD_MODER |= (1 << 30);
}

static void
configureGreenLED ()
{
  /* Clock already enabled, since blue LED is also on GPIOD */
  /* Green LED: PD12 */
  GPIOD_MODER &= ~(1U << 24);
  GPIOD_MODER &= ~(1U << 25);
  GPIOD_MODER |= (1 << 24);
}

static void
configureOrangeLED ()
{
  /* Orange LED: PD13 */
  GPIOD_MODER &= ~(1U << 26);
  GPIOD_MODER &= ~(1U << 27);
  GPIOD_MODER |= (1 << 26);
}

static void
configureSystickInterrupts ()
{
  SYSTICK_CSR &= ~(1U << 0);    /* Disable timer */
  SYSTICK_CSR |= (1 << 2);      /* Use SYSCLK */
  SYSTICK_CSR |= (1 << 1);      /* Enable interrupt requests */
  SYSTICK_RELOAD = 7999;        /* 1 ms per interval */
  SYSTICK_CURRENT = 0x00000000; /* Reset current count */
  SYSTICK_CSR |= (1 << 0);      /* Enable Timer */
}

static void
configureInterruptPriorities ()
{
  /* PendSV to the lowest priority (240 because only top 4 bits are used) */
  SHPR3 &= ~(0xFFU << PENDSV_PRIORITY_START_BIT);
  SHPR3 |= (0xF0U << PENDSV_PRIORITY_START_BIT);

  /* SysTick to right above PendSV (224 because only top 4 bits are used) */
  SHPR3 &= ~(0xFFU << SYSTICK_PRIORITY_START_BIT);
  SHPR3 |= (0xE0U << SYSTICK_PRIORITY_START_BIT);
}

void
configureAll ()
{
  configureClock ();
  configureSystickInterrupts ();
  configureBlueLED ();
  configureGreenLED ();
  configureOrangeLED ();
  configureInterruptPriorities ();
}
