#ifndef CONFIG_H_
#define CONFIG_H_

#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include "mcu_registers.h"

void configureClock();
void configureBlueLED();
void configureGreenLED();
void configureOrangeLED();
void configureSystickInterrupts();
void configureAll();

#endif
