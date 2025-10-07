# SRTOS - Guides

## Prerequisites

- SRTOS is cloned and ready to be uploaded to your board. For more information, check `README.md`.
- You have read `DESIGN.md` and understand the basic operating principles of SRTOS.

## Getting Started

SRTOS runs from any file that contains `int main()`, but preferably should be defined in a file called `main.c` to maintain good practice and readability.

The first step in any RTOS program is to define your task functions and their relavent memory buffers/structs. Task functions should **never** return, and should always implement their functionality inside an infinite loop. Here is an example task taken from the LED blink example (written for STM32F411E):

Your main file must include `task.h` and `config.h`. _This will be updated as SRTOS changes._

```
#include "config.h"
#include "task.h"

uint32_t task1Stack[STACK_SIZE];
TCB task1TCB;
TaskNode task1Node;

static void
task1_blueLED ()
{
  while (1)
    {
      GPIOD_ODR ^= (1 << 15);
      taskDelay (1000);
    }
}
```

This part imports the necessary header files. System-defined imports like `stdint.h` are included in `config.h`, so the user does not need to worry about including these.

```
#include "config.h"
#include "task.h"
```

This part defines the memory needed for the task. Every task has 3 statically-defined components: the stack, TCB, and TaskNode. These are explained in `DESIGN.md`.

```
uint32_t task1Stack[STACK_SIZE];
TCB task1TCB;
TaskNode task1Node;
```

This is the actual task definition. It doesn't _need_ to be marked static, but it is good practice as it is only used in one file. The task uses an infinite loop to complete its operations then delay for 1 second. With `taskDelay ()`, you specify the delay in ms. Tasks don't need to delay, but that is a choice the user must make. This is the basic way to create a task function. Now, you must create the task and start the scheduler.

```
static void
task1_blueLED ()
{
  while (1)
    {
      GPIOD_ODR ^= (1 << 15);
      taskDelay (1000);
    }
}
```

Here is a code snippet that creates `task1_blueLED ()` and starts the scheduler.

```
int
main ()
{
  createTask (task1Stack, &task1_blueLED,
              1, &task1TCB, &task1Node);

  startScheduler ();
  while (1)
    {
    }
}

```

This will create the task and add it to the ready list. `createTask ()` is defined in much more detail in the documentation, but this will give you a high-level overview.

You must first pass in the stack. This just requires referencing the name you gave to the stack above. Next, you must pass the address of the function. After that, you must pass in the priority. Priorites span from [0 ... `MAX_PRIORITIES` - 1]. `MAX_PRIORITIES` is the maximum number of priorities allowed, starting at 0. This, along with `STACK_SIZE`, is configurable in `kernel_config.h`. Passing in an invalid priority will result in a `STATUS_FAILURE` being returned. Once asserts are added to SRTOS, you can use these status returns to ensure everything is working. Then, you just need to pass in the address of the task's TCB and TaskNode.

```
  createTask (task1Stack, &task1_blueLED,
              1, &task1TCB, &task1Node);
```

This will start the scheduler. After creating all tasks, you just need to call `startScheduler ()` to begin execution of tasks. Tasks may **not** be created after the scheduler is started.

```
startScheduler ();
while (1)
  {
  }
```