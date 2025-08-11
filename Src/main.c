#include "config.h"
#include "task.h"

uint32_t task1Stack[STACK_SIZE];
uint32_t task2Stack[STACK_SIZE];

// Tasks
void task1_blueLED() {
	static uint32_t nextBlink = 0;
	for (;;) {
		if (msTicks >= nextBlink) {
			GPIOD_ODR ^= (1 << 15);
			nextBlink = msTicks + 500;
		}
	}
}


void task2_greenLED() {
	static uint32_t nextBlink = 0;
	for(;;) {
		if (msTicks >= nextBlink) {
			GPIOD_ODR ^= (1 << 12);
			nextBlink = msTicks + 500;
		}
	}
}

int main(void)
{
	configureAll();

	createTask(task1Stack, &task1_blueLED, 1);
	createTask(task2Stack, &task2_greenLED, 1);

	startScheduler();
	while (1) {}
}
