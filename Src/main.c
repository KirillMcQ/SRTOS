#include "config.h"
#include "task.h"
#include <stdint.h>

// Static allocation, user must provide these memory buffers
uint32_t task1Stack[STACK_SIZE];
uint32_t task2Stack[STACK_SIZE];
TCB task1TCB;
TCB task2TCB;
TaskNode task1Node;
TaskNode task2Node;

static void
task1_blueLED ()
{
  while (1)
    {
      GPIOD_ODR ^= (1 << 15);
      taskDelay (1000);
    }
}

static void
task2_greenLED ()
{
  while (1)
    {
      GPIOD_ODR ^= (1 << 12);
      taskDelay (500);
    }
}

int
main (void)
{
  configureAll ();

  createTask (task1Stack, &task1_blueLED, 1, &task1TCB, &task1Node);
  createTask (task2Stack, &task2_greenLED, 1, &task2TCB, &task2Node);

  startScheduler ();
  while (1)
    {
    }
}
