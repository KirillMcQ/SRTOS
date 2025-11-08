/**
 * @file    config.h
 * @brief   System configuration for SRTOS.
 * @details
 * Declares configuration routines executed before the scheduler starts.
 * Users can define hardware and peripheral initialization functions in
 * `config.c` and call them through `configureAll()`.
 */

#ifndef CONFIG_H_
#define CONFIG_H_

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

#include "mcu_macros.h"

/**
 * @brief This function will be called in the main() function of the user's code before the scheulder is started.
 * It will call every configuration function defined in config.c
 * 
 * @note SRTOS is designed with configuration in mind, so users are able to create
 * configuration functions in config.c, and call them in this function. Users can add and delete functions calls from this function as needed.
 */
void configureAll ();

#endif
