#ifndef CONFIG_H_
#define CONFIG_H_

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

#include "mcu_macros.h"

extern uint32_t const flash_text_start;
extern uint32_t const flash_text_end;

extern uint32_t const sram_start;
extern uint32_t const sram_end;
extern uint32_t const sram_size;

void configureAll ();

#endif
