#include "config.h"

// Configuration functions
static void
configureClock()
{
  // Clock Configuration - SYSCLK sourced from HSE - 8 Mhz
  RCC_CR |= (1 << 16); // HSEON -> 1
  while (!(RCC_CR & (1 << 17)))
    ;
  RCC_CFGR &= ~(1U << 0);
  RCC_CFGR &= ~(1U << 1);
  RCC_CFGR |= (1 << 0);
  // 0x3 -> 0b11
  // 0x1 -> 0b01
  while ((RCC_CFGR & (0x3 << 2)) != (0x1 << 2))
    ;
}

static void
configureBlueLED()
{
  // Enable the RCC Peripheral Clock for GPIOD
  RCC_AHB1ENR |= (1 << 3);
  // Set the PD15 GPIO Pin (Blue LED) to Output
  GPIOD_MODER &= ~(1U << 31);
  GPIOD_MODER &= ~(1U << 30);
  GPIOD_MODER |= (1 << 30);
}

static void
configureGreenLED()
{
  // Clock already enabled, since blue LED is also on GPIOD
  // Green LED: PD12
  GPIOD_MODER &= ~(1U << 24);
  GPIOD_MODER &= ~(1U << 25);
  GPIOD_MODER |= (1 << 24);
}

static void
configureOrangeLED()
{
  // Orange LED: PD13
  GPIOD_MODER &= ~(1U << 26);
  GPIOD_MODER &= ~(1U << 27);
  GPIOD_MODER |= (1 << 26);
}

static void
configureSystickInterrupts()
{
  SYSTICK_CSR &= ~(1U << 0);    // Disable timer
  SYSTICK_CSR |= (1 << 2);      // Use SYSCLK
  SYSTICK_CSR |= (1 << 1);      // Enable interrupt requests
  SYSTICK_RELOAD = 7999;        // 1 ms per interval
  SYSTICK_CURRENT = 0x00000000; // Reset current count
  SYSTICK_CSR |= (1 << 0);      // Enable Timer
}

static void
configureInterruptPriorities()
{
  // PendSV to the lowest priority (240 because only top 4 bits are used)
  SHPR3 &= ~(0xFFU << PENDSV_PRIORITY_START_BIT);
  SHPR3 |= (0xF0U << PENDSV_PRIORITY_START_BIT);

  // SysTick to right above PendSV (224 because only top 4 bits are used)
  SHPR3 &= ~(0xFFU << SYSTICK_PRIORITY_START_BIT);
  SHPR3 |= (0xE0U << SYSTICK_PRIORITY_START_BIT);
}

static void
configureMPU()
{
  uint32_t flashTextStartAddr = (uint32_t)&flash_text_start;
  uint32_t flashTextEndAddr = (uint32_t)&flash_text_end;

  uint32_t codeMemorySizeInBytes = flashTextEndAddr - flashTextStartAddr;

  int mpuRegionSizeExponent;

  for (int i = 0; i < 32; i++)
  {
    if ((1U << i) >= codeMemorySizeInBytes)
    {
      mpuRegionSizeExponent = i - 1;
      break;
    }
  }

  uint32_t alignedBase = flashTextStartAddr & ~((1U << (mpuRegionSizeExponent + 1)) - 1);

  
  MPU_RNR = 0U; /* Region 0 protects system code (.text in flash) */
  
  MPU_RBAR = alignedBase;

  uint32_t rasr = 0;
  rasr |= ((uint32_t) mpuRegionSizeExponent << MPU_RASR_SIZE_START_BIT);
  
  /* SBC = 0b001 TEX = 0b000
  * recommended by https://interrupt.memfault.com/blog/fix-bugs-and-secure-firmware-with-the-mpu
  * */
  rasr &= ~(1U << MPU_RASR_ATTRS_B_BIT);
  rasr |= (1U << MPU_RASR_ATTRS_C_BIT);
  rasr &= ~(1U << MPU_RASR_ATTRS_S_BIT);
  rasr &= ~(0x7U << MPU_RASR_ATTRS_TEX_START_BIT);
  
  /* AP = 0b110 = read only for both privileged and unprivileged code */
  rasr &= ~(1U << MPU_RASR_ATTRS_AP_START_BIT);
  rasr |= (0x3U << (MPU_RASR_ATTRS_AP_START_BIT + 1));
  
  /* XN = 0 = this section can and should be executed */
  rasr &= ~(1U << MPU_RASR_ATTRS_XN_BIT);

  /* Enable MPU and the current region */
  rasr |= (1U << MPU_RASR_ENABLE_BIT);

  MPU_RASR = rasr;

  MPU_CTRL = (1U << MPU_CTRL_ENABLE_BIT) | (1U << MPU_CTRL_PRIVDEFENA_BIT);

  __asm volatile("dsb\n");
  __asm volatile("isb\n");
}

void
configureAll()
{
  configureClock();
  configureSystickInterrupts();
  configureBlueLED();
  configureGreenLED();
  configureOrangeLED();
  configureInterruptPriorities();
  configureMPU();
}
