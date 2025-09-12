#ifndef TASK_H_
#define TASK_H_

#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include "kernel_config.h"
#include "mcu_registers.h"


typedef enum
{
	STATUS_SUCCESS = 0,
	STATUS_FAILURE = 1
} STATUS;

typedef struct
{
	uint32_t *sp;
	uint32_t priority;
	uint32_t id;
	uint32_t delayedUntil;
} TCB;

typedef struct TaskNode TaskNode;

struct TaskNode
{
	TCB *taskTCB;
	TaskNode *next;
};

extern volatile uint32_t msTicks;
extern TaskNode *readyTasksList[MAX_PRIORITIES];

uint32_t *initTaskStackFrame(uint32_t taskStack[], void (*taskFunc)(void));
STATUS createTask(uint32_t taskStack[], void (*taskFunc)(void), unsigned int priority, TCB *userAllocatedTCB, TaskNode *userAllocatedTaskNode);

void SysTick_Handler();
void PendSV_Handler();
void SVC_Handler();

void setPendSVPending();
void startScheduler();

void taskYield(); // TODO: This shouldn't work right now. Fix later.
void taskDelay(uint32_t ticksToDelay);

#endif
