#ifndef TASK_H_
#define TASK_H_

#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>

#define GPIOD_START_ADDR 0x40020C00
#define GPIOD_ODR *((volatile uint32_t *)(GPIOD_START_ADDR + 0x14))
#define ICSR *((volatile uint32_t *)(0xE000ED04))
#define STACK_SIZE 128
#define MAX_PRIORITIES 2 // For now, only 2 priorities: 0 and 1

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

typedef struct
{
	TCB *taskTCB;
	struct TaskNode *next;
} TaskNode;

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
