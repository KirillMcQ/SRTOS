#ifndef CONFIG_H_
#define CONFIG_H_

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

#include "mcu_macros.h"

extern uint32_t const flash_text_start;
extern uint32_t const flash_text_end;

void configureAll ();

#endif
