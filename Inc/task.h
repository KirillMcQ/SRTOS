/**
 * @file    task.h
 * @brief   This header files provides all task control functions for SRTOS users.
 */

#ifndef TASK_H_
#define TASK_H_

#include "fault.h"
#include "kernel_config.h"
#include "mcu_macros.h"
#include "system_funcs.h"
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>


/**
 * @brief This enum is used to indicate whether an operation was sucessful or not.
 */
typedef enum
{
  STATUS_SUCCESS = 0,
  STATUS_FAILURE = 1
} STATUS;

/**
 * @brief This struct is the Task Control Block (TCB), which is what stores a task's properties.
 */
typedef struct
{
  uint32_t *sp;
  uint32_t priority;
  uint32_t id;
  uint32_t delayedUntil;
  uint32_t *stackFrameLowerBoundAddr;
} TCB;

/**
 * @brief This struct is used to represent a task in a linked list.
 */
typedef struct TaskNode TaskNode;

struct TaskNode
{
  TCB *taskTCB;
  TaskNode *next;
};

/**
 * @brief msTicks contains the amount of ticks that have occured since the scheduler started.
 * 
 * @warning User code should rarely, if ever, access this variable.
 */
extern volatile uint32_t msTicks;

/**
 * @brief This is a list of linked lists that contain the ready tasks.
 * 
 * @note Each index represents the linked list of ready tasks for that priority.
 * 
 * @warning This variable should never be accessed in user code.
 */
extern TaskNode *readyTasksList[MAX_PRIORITIES];


uint32_t *initTaskStackFrame (uint32_t taskStack[], void (*taskFunc) (void));

/**
 * @brief Add a task to the scheduler's ready list.
 * 
 * @param taskStack The user-defined array of size STACK_SIZE
 * @param taskAddress The address of the task function
 * @param priority The task's priority which must be between 0 and MAX_PRIORITIES - 1, inclusive
 * @param userAllocatedTCB The address of the task's TCB allocated by the user
 * @param userAllocatedTaskNode The address of the task's TaskNode allocated by the user
 * 
 * @return Returns either STATUS_SUCCESS or STATUS_FAILURE, depending on whether the task was successfully created or not.
 * 
 * @note Must be called before the scheduler is started.
 */
STATUS
createTask (uint32_t taskStack[], void (*taskFunc) (void),
            unsigned int priority, TCB *userAllocatedTCB,
            TaskNode *userAllocatedTaskNode);

void SysTick_Handler ();
void PendSV_Handler ();
void SVC_Handler ();
void setPendSVPending ();

/**
 * @brief Start the scheduler.
 * 
 * @note No tasks should be created after this function is called.
 * @note There should be no code directly under this function call that is expected to execute. This function will initiate the transfer of execution to the first task.
 */
void startScheduler ();

/**
 * @brief This function will delay a task's execution for ticksToDelay ms.
 * @details This function will move the current task to the blocked list for ticksToDelay ms.
 * If there are no other tasks available to execute, the idleTask will execute.
 * 
 * @param ticksToDelay The number of milliseconds to delay a task's execution.
 */
void taskDelay (uint32_t ticksToDelay);

/**
 * @brief This function will return the minimum number of words left on the stack.
 * 
 * @return This function returns the minimum amount of words left on the stack.
 * 
 * @note A word is 32 bits in size on the ARM-Cortex M4/M4f.
 */
uint32_t getCurTaskWordsAvailable ();

/**
 * @brief This function will be called when a stack overflow is detected.
 * This function will only be called if no other definitions are found.
 * The user is recommended to define this themselves.
 */
void handleStackOverflow ();

#endif
