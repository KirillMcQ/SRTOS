#ifndef MCU_REGISTERS_H_
#define MCU_REGISTERS_H_

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

#endif
