#include "config.h"
#include "task.h"

// TODO: Define these in another file, probably task.c, so the user doesn't need to.
uint32_t task1Stack[STACK_SIZE];
uint32_t task2Stack[STACK_SIZE];

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

	createTask(task1Stack, &task1_blueLED, 1);
	createTask(task2Stack, &task2_greenLED, 1);

	startScheduler();
	while (1)
	{
	}
}
