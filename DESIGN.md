# Design

SRTOS is designed with safety and ease-of-learning in mind. The goal of the project is not only to be an introductory RTOS, but also be memory-safe and reliable. This file will give information on the scheduler design and behavior, as well as reasons behind the implementation.

## Useful Terminology

- **Kernel**: The central system code that manages all RTOS features. This controls the scheduler, context switching, etc...
- **Scheduler**: Core kernel component that determines which task is and should be executing.
- **Preemptive Scheduler**: Scheduler type that switches out the current executing task if another higher priority task becomes ready. Even if the currently executing task doesn't yield, it may still be switched out.
- **Context Switch**: The operation of saving the currently executing task's state and restoring another's. This is the operation where tasks swap in and out.

## Scheduler Overview

SRTOS uses a preemptive scheduling algorithm that selects the highest priority task ready to execute. Tasks can be in one of 2 states: ready or blocked. In the current phase of SRTOS (this will change as more features are added), tasks are only in the blocked list if they are delayed by calling `taskDelay ()`. If a task doesn't call `taskDelay ()`, they will be in the ready tasks list. The scheduler is driven by a 1ms `SysTick`, which means that every 1ms the kernel code will check if a context switch is needed.

As an example, imagine 2 tasks `task1` and `task2`, with the priorities 1 and 2, respectively. They are implemented like so (**Pseudocode, do not attempt to execute**):

```
void
task1 ()
{
  while (1)
    {
      makeLedBlink ();
      taskDelay (1000);
    }
}

void
task2 ()
{
  while (1)
    {
      makeAnotherLedBlink ();
      taskDelay (1000);
    }
}
```

When the scheduler is started, `task2` will begin executing, and after it calls `taskDelay (1000)`, it will be placed in the blocked list until it unblocks 1 second later. Once `task2` is in the blocked list, the scheduler will switch to `task1`, as that is the next highest priority task available to execute. After `task1` finishes its operations and calls `taskDelay (1000)`, there are no more user-defined tasks ready to execute, so the scheduler executes `idleTask` until another task with an equal or higher priority unblocks. `idleTask` is simply an infinite loop that executes the `WFI` assembly instruction, which stands for "Wait For Interrupt." This implementation of `idleTask` helps save power and system resources. Once the next task unblocks, it will be switched in and the process will continue.

If two or more tasks of equal priority are ready to execute, the scheduler will switch between the tasks on every tick, giving each task a 1ms chunk of time to execute.

## Scheduler Safety

Stack overflow detection is implemented and non-optional. Before a task is switched out, a check is made to ensure two canary values at the lower bound of the task's stack are not overwritten. If they are, the `handleStackOverflow ()` function is called. This program must not exit, unless the user tries to implement system recovery. `handleStackOverflow ()` is weakly defined in `task.c`, so any other implementation that is non-weakly defined will be used. The function `getCurTaskWordsAvailable ()` will return the minimum number of words still available on a task's stack. This is useful for tasks when determining how much space is left on a task's stack, which can aid in responding to potential stack overflows before they happen.

A default hardfault handler is provided in `fault.c`. Other handlers may be provided, but they must be naked, or in other words, the compiler **must not** add a prologue or epilogue to the function. If it were to add either of these, the stack pointer would no longer point to the exception frame that gives key information into why the system faulted. Custom handler functions must call `systemGet_Fault_SP ()`, and `systemHandle_Fault ()` right after. `systemHandle_Fault ()` is the function where you can handle the fault. Recovery options are slim in this situation. The best thing you can do is write to non-volatile memory and go into an infinite loop. This is the default behavior of the STM32F411E-DISCOVERY implementation of SRTOS.

The user **must** define all data needed for tasks. You must define the task's stack, TCB, and TaskNode. The stack is the section of memory that the task will use to store all runtime information. For more information, look into stacks. You can configure `STACK_SIZE` in `kernel_config.h`, which is the size of the user stack in words (`uint32_t`). You can calculate the size of the stack in bytes by multiplying `STACK_SIZE` by 4: `stackSizeInBytes = STACK_SIZE * 4`. You can experiment with values, or stick with the default value, but you should try to tailor the value to your tasks, to conserve memory. The TCB is the structure that contains all information about the task. The TaskNode is a node in a linked list that contains the task's TCB and its `next` pointer. You don't need to initialize these, just pass in the memory address. Please refer to `GUIDES.md` for getting started guides and information on how to create tasks. The documentation also contains detailed descriptions of everything you need to know.
