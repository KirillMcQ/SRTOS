#include "task.h"

volatile uint32_t msTicks = 0;
TaskNode *readyTasks = NULL;
TaskNode *tasks = NULL;
TaskNode *curTask = NULL;
static uint32_t curTaskIDNum = 0;

static void prvAddTaskNodeToReadyList(TaskNode *task);

uint32_t *initTaskStackFrame(uint32_t taskStack[], void (*taskFunc)(void))
{
	taskStack[STACK_SIZE - 1] = 0x01000000;									// xPSR
	taskStack[STACK_SIZE - 2] = ((uint32_t)taskFunc) | 0x1; // PC
	taskStack[STACK_SIZE - 3] = 0xFFFFFFFD;									// LR
	taskStack[STACK_SIZE - 4] = 0x00000000;									// R12
	taskStack[STACK_SIZE - 5] = 0x00000000;									// R3
	taskStack[STACK_SIZE - 6] = 0x00000000;									// R2
	taskStack[STACK_SIZE - 7] = 0x00000000;									// R1
	taskStack[STACK_SIZE - 8] = 0x00000000;									// R0
	taskStack[STACK_SIZE - 9] = 0x00000000;									// R11
	taskStack[STACK_SIZE - 10] = 0x00000000;								// R10
	taskStack[STACK_SIZE - 11] = 0x00000000;								// R9
	taskStack[STACK_SIZE - 12] = 0x00000000;								// R8
	taskStack[STACK_SIZE - 13] = 0x00000000;								// R7
	taskStack[STACK_SIZE - 14] = 0x00000000;								// R6
	taskStack[STACK_SIZE - 15] = 0x00000000;								// R5
	taskStack[STACK_SIZE - 16] = 0x00000000;								// R4

	return &taskStack[STACK_SIZE - 16];
}

void createTask(uint32_t taskStack[], void (*taskFunc)(void), unsigned int priority)
{
	TCB *taskTCB = (TCB *)malloc(sizeof(TCB));

	taskTCB->sp = initTaskStackFrame(taskStack, taskFunc);
	taskTCB->priority = priority;
	taskTCB->id = curTaskIDNum;
	curTaskIDNum++;

	// Check if the linked list has been initialized yet
	if (tasks == NULL)
	{
		// This is the first task, so this is where we will start
		tasks->taskTCB = taskTCB;
		tasks->next = NULL;
		curTask = tasks;
		return;
	}

	// Get the tail of the tasks linked list
	TaskNode *cur = tasks;
	while (cur->next != NULL)
	{
		cur = (cur->next);
	}

	// Insert at end of tasks linked list
	TaskNode *new = (TaskNode *)malloc(sizeof(TaskNode));
	new->taskTCB = taskTCB;
	new->next = NULL;
	cur->next = new;
	prvAddTaskNodeToReadyList(new);
}

void SysTick_Handler()
{
	msTicks++;
}

void PendSV_Handler()
{
	GPIOD_ODR ^= (1 << 13);

	uint32_t spToSave;

	__asm volatile(
			"mrs r0, PSP\n"
			"stmdb r0!, {r4-r11}\n"
			"mov %[spToSave], r0\n"
			: [spToSave] "=r"(spToSave));

	curTask->taskTCB->sp = (uint32_t *)spToSave;

	TaskNode *nextTask = (TaskNode *)(curTask->next);

	uint32_t nextSP;

	if (nextTask != NULL)
	{
		nextSP = (uint32_t)nextTask->taskTCB->sp;
	}
	else
	{
		nextTask = &tasks;
		nextSP = (uint32_t)nextTask->taskTCB->sp;
	}

	curTask = nextTask;

	__asm volatile(
			"mov r2, %[nextSP]\n"
			"ldmia r2!, {r4-r11}\n"
			"msr PSP, r2\n"
			:
			: [nextSP] "r"(nextSP));

	__asm volatile(
			"ldr lr, =0xFFFFFFFD\n"
			"bx lr\n");
}

void SVC_Handler()
{
	TCB *tcbToStart = curTask->taskTCB;
	uint32_t spToStart = tcbToStart->sp;

	__asm volatile(
			"ldr r0, %[sp]\n"
			"ldmia r0!, {r4-r11}\n"
			"msr PSP, r0\n"
			"ldr lr, =0xFFFFFFFD\n"
			"bx lr\n"
			:
			: [sp] "m"(spToStart));
}

void startScheduler()
{
	__asm volatile("svc #0");
}

void setPendSVPending()
{
	ICSR |= (1 << 28);
}

void taskYield()
{
	setPendSVPending();
}

// TODO: Does changing task->next corrupt the tasks list?
static void prvAddTaskNodeToReadyList(TaskNode *task)
{
	if (readyTasks == NULL)
	{
		task->next = NULL;
		readyTasks = task;
		return;
	}

	if (task->taskTCB->priority >= readyTasks->taskTCB->priority)
	{
		task->next = readyTasks;
		readyTasks = task;
		return;
	}

	TaskNode *cur = readyTasks;
	TaskNode *next = cur->next;

	while (next != NULL && task->taskTCB->priority < next->taskTCB->priority)
	{
		cur = cur->next;
		next = next->next;
	}

	task->next = next;
	cur->next = task;
}
