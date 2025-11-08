/**
 * @file    system_funcs.h
 * @brief   This header files contains kernel functions that help control system behvaior.
 * 
 * @warning Only low-level system components and startup code should include this file.
 */

#ifndef SYSTEM_FUNCS_H_
#define SYSTEM_FUNCS_H_

#include <stdint.h>

void systemENTER_CRITICAL ();
void systemEXIT_CRITICAL ();

#endif
