#include "config.h"
#include "task.h"

// Static allocation, user must provide these memory buffers
uint32_t task1Stack[STACK_SIZE];
uint32_t task2Stack[STACK_SIZE];
TCB task1TCB;
TCB *task1TCBPtr = &task1TCB;
TCB task2TCB;
TCB *task2TCBPtr = &task2TCB;
TaskNode task1Node;
TaskNode *task1NodePtr = &task1Node;
TaskNode task2Node;
TaskNode *task2NodePtr = &task2Node;

// Blinking LED tasks
void task1_blueLED()
{
	for (;;)
	{
		GPIOD_ODR ^= (1 << 15);
		taskDelay(500);
	}
}

void task2_greenLED()
{
	for (;;)
	{
		GPIOD_ODR ^= (1 << 12);
		taskDelay(500);
	}
}

int main(void)
{
	configureAll();

	createTask(task1Stack, &task1_blueLED, 1, task1TCBPtr, task1NodePtr);
	createTask(task2Stack, &task2_greenLED, 1, task2TCBPtr, task2NodePtr);

	startScheduler();
	while (1)
	{
	}
}
