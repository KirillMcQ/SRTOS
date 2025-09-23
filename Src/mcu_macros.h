#ifndef MCU_MACROS_H_
#define MCU_MACROS_H_

#define GPIOD_START_ADDR 0x40020C00
#define RCC_START_ADDR 0x40023800
#define RCC_AHB1ENR *((volatile uint32_t *)(RCC_START_ADDR + 0x30))
#define GPIOD_MODER *((volatile uint32_t *)GPIOD_START_ADDR)
#define GPIOD_ODR *((volatile uint32_t *)(GPIOD_START_ADDR + 0x14))
#define RCC_CR *((volatile uint32_t *)(RCC_START_ADDR))
#define RCC_CFGR *((volatile uint32_t *)(RCC_START_ADDR + 0x08))
#define SYSTICK_CSR *((volatile uint32_t *)(0xE000E010))
#define SYSTICK_RELOAD *((volatile uint32_t *)(0xE000E014))
#define SYSTICK_CURRENT *((volatile uint32_t *)(0xE000E018))
#define ICSR *((volatile uint32_t *)(0xE000ED04))
#define SHPR3 *((volatile uint32_t *)(0xE000ED20))
#define PENDSV_PRIORITY_START_BIT 16
#define SYSTICK_PRIORITY_START_BIT 24
#define STACK_OVERFLOW_CANARY_VALUE 0xDEADBEEF
#define STACK_USAGE_WATERMARK 0xBAADF00D
#define FLASH_REGISTERS_START_ADDR 0x40023C00
#define FLASH_KEYR *((volatile uint32_t *)(FLASH_REGISTERS_START_ADDR + 0x04))
#define FLASH_UNLOCK_KEY1 0x45670123
#define FLASH_UNLOCK_KEY2 0xCDEF89AB
#define FLASH_SR *((volatile uint32_t *)(FLASH_REGISTERS_START_ADDR + 0x0C))
#define FLASH_SR_BSY_BIT 16

#endif
