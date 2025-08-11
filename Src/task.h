#ifndef TASK_H_
#define TASK_H_

#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>

#define GPIOD_START_ADDR 0x40020C00
#define GPIOD_ODR *((volatile uint32_t *)(GPIOD_START_ADDR + 0x14))
#define ICSR *((volatile uint32_t *)(0xE000ED04))
#define STACK_SIZE 128

typedef struct {
	uint32_t *sp;
	unsigned int priority;
	uint32_t id;
} TCB;

typedef struct {
	TCB *taskTCB;
	struct TaskNode *next;
} TaskNode;


extern volatile uint32_t msTicks;
extern TaskNode *readyTasks;
extern TaskNode tasks;
extern TaskNode *curTask;

uint32_t *initTaskStackFrame(uint32_t taskStack[], void (*taskFunc)(void));
void createTask(uint32_t taskStack[], void (*taskFunc)(void), unsigned int);

void SysTick_Handler();
void PendSV_Handler();
void SVC_Handler();

void setPendSVPending();
void startScheduler();

void taskYield();

#endif
