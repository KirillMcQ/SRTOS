#include "task.h"

volatile uint32_t msTicks = 0; // Overflows in ~49 days of concurrent running.
TaskNode *curTask = NULL;
static uint32_t prvCurTaskIDNum = 0;
TaskNode *readyTasksList[MAX_PRIORITIES] = {NULL}; // All NULL initially
static TaskNode *prvNextTask = NULL;
static TaskNode *prvBlockedTasks = NULL; // Head of blocked tasks list (Currently, the only way to block is to be delayed)

// File-scoped private function headers
static STATUS prvAddTaskNodeToReadyList(TaskNode *task);
static TaskNode *prvGetHighestTaskReadyToExecute();
static void prvAddTaskToBlockedList(TaskNode *task);
static void prvUnblockDelayedTasksReadyToUnblock();

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

STATUS createTask(uint32_t taskStack[], void (*taskFunc)(void), unsigned int priority)
{
	TCB *taskTCB = (TCB *)malloc(sizeof(TCB));

	taskTCB->sp = initTaskStackFrame(taskStack, taskFunc);
	taskTCB->priority = priority;
	taskTCB->id = prvCurTaskIDNum;
	prvCurTaskIDNum++;

	// Insert at end of tasks linked list
	TaskNode *new = (TaskNode *)malloc(sizeof(TaskNode));
	new->taskTCB = taskTCB;
	new->next = NULL;

	return prvAddTaskNodeToReadyList(new);
}

// TODO: Make tasks able to block
void SysTick_Handler()
{
	if (!curTask)
	{
		msTicks++;
		return;
	}

	uint32_t curExecutingPriority = curTask->taskTCB->priority;

	TaskNode *highestPriorityPossibleExecute = prvGetHighestTaskReadyToExecute();

	// Check if a higher priority task is ready to execute
	if (curExecutingPriority < highestPriorityPossibleExecute->taskTCB->priority)
	{
		prvNextTask = highestPriorityPossibleExecute;
		setPendSVPending();
		msTicks++;
		return;
	}

	if (curTask->next == NULL)
	{
		if (highestPriorityPossibleExecute->taskTCB->id != curTask->taskTCB->id)
		{
			// There is another task of equal priority, time to switch.
			prvNextTask = highestPriorityPossibleExecute;
			setPendSVPending();
			msTicks++;
			return;
		}
	}
	else
	{
		// There is another task of equal priority, time to switch.
		prvNextTask = curTask->next;
		setPendSVPending();
		msTicks++;
		return;
	}

	msTicks++;
}

void PendSV_Handler()
{
	uint32_t spToSave;

	__asm volatile(
			"mrs r0, PSP\n"
			"stmdb r0!, {r4-r11}\n"
			"mov %[spToSave], r0\n"
			: [spToSave] "=r"(spToSave));

	curTask->taskTCB->sp = (uint32_t *)spToSave;

	uint32_t nextSP = prvNextTask->taskTCB->sp;

	curTask = prvNextTask;

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
	curTask = prvGetHighestTaskReadyToExecute();
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

void taskDelay(uint32_t ticksToDelay)
{
	uint32_t curTaskID = curTask->taskTCB->id;
	uint32_t curTaskPriority = curTask->taskTCB->priority;

	curTask->taskTCB->delayedUntil = msTicks + ticksToDelay;

	// Remove the task from the ready list
	TaskNode *cur = readyTasksList[curTaskPriority];
	TaskNode *prev = NULL;

	if (cur->next == NULL) {
		// This is the only task for this priority, and it must be curTask
		readyTasksList[curTaskPriority] = NULL;
		prvNextTask = prvGetHighestTaskReadyToExecute();
		prvAddTaskToBlockedList(curTask);
		setPendSVPending();
		return;
	}

	// Check if curTask is the head of the priority
	if (cur->taskTCB->id == curTaskID) {
		// We know there is more than one task, just make the new head the next task
		readyTasksList[curTaskPriority] = curTask->next;

		prvNextTask = prvGetHighestTaskReadyToExecute();
		prvAddTaskToBlockedList(curTask);
		setPendSVPending();
		return;
	}


	// There is more than one task for the current priority
	while (cur->taskTCB->id != curTaskID) {
		prev = cur;
		cur = cur->next;
	}

	TaskNode *afterCur = cur->next;
	prev->next = afterCur;

	prvNextTask = prvGetHighestTaskReadyToExecute();
	prvAddTaskToBlockedList(curTask);
	setPendSVPending();
	return;
}

static STATUS prvAddTaskNodeToReadyList(TaskNode *task)
{
	// Safeguards
	if (task->taskTCB->priority >= MAX_PRIORITIES)
	{
		return STATUS_FAILURE;
	}

	task->next = NULL;

	uint32_t curPriority = task->taskTCB->priority;

	TaskNode *curHead = readyTasksList[curPriority];

	if (curHead == NULL)
	{
		// This is the first node for this priority
		readyTasksList[curPriority] = task;
		return STATUS_SUCCESS;
	}

	// Get to the end of the LL
	while (curHead->next != NULL)
	{
		curHead = curHead->next;
	}

	curHead->next = task;
	return STATUS_SUCCESS;
}

static TaskNode *prvGetHighestTaskReadyToExecute()
{
	int idx = MAX_PRIORITIES - 1; // Highest Priority Possible

	while (idx >= 0)
	{
		if (readyTasksList[idx] != NULL)
		{
			return readyTasksList[idx];
		}
		--idx;
	}

	return NULL;
}

// Maybe make this return a STATUS?
static void prvAddTaskToBlockedList(TaskNode *task) {
	task->next = NULL;

	if (prvBlockedTasks == NULL) {
		prvBlockedTasks = task;
		return;
	}

	TaskNode *cur = prvBlockedTasks;

	while (cur->next != NULL) {
		cur = cur->next;
	}

	cur->next = task;

	return;
}

// Maybe make this return a STATUS?
// TODO: Finish implementing
static void prvUnblockDelayedTasksReadyToUnblock() {
	TaskNode *cur = prvBlockedTasks;
	TaskNode *prev = NULL;

	while (cur != NULL) {
		if (cur->taskTCB->delayedUntil == msTicks) {
			// Add it to the ready list and remove from this list
		}
	}
}
