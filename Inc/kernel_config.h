/**
 * @file    kernel_config.h
 * @brief   Core configuration parameters for SRTOS.
 * @details
 * This file defines system-level constants that control kernel behavior.
 */

#ifndef KERNEL_CONFIG_H_
#define KERNEL_CONFIG_H_

/**
 * @brief Default stack size for each task, in 32-bit words.
 * @details
 * Each task’s stack is statically allocated at creation time.
 * The total size in bytes is `STACK_SIZE * 4`.
 * Adjust this value based on task complexity and available RAM.
 */
#define STACK_SIZE 128U

/**
 * @brief Number of unique task priority levels supported by the scheduler.
 * @details
 * Priorities range from `0` (lowest) to `MAX_PRIORITIES - 1` (highest).
 * Increasing this value allows for increased scheduler clarity
 * but may increase context-switching overhead and decrease performance.
 */
#define MAX_PRIORITIES 2U

#endif
