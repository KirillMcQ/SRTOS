/**
 * @file    task.h
 * @brief   Task management and scheduler API for SRTOS.
 * @details
 * Provides user-level functions to manage tasks.
 */

#include "task.h"

volatile uint32_t msTicks = 0;
TaskNode *curTask = NULL;
TaskNode *readyTasksList[MAX_PRIORITIES] = { NULL };

static uint32_t prvCurTaskIDNum = 0;
static TaskNode *prvNextTask = NULL;
static TaskNode *prvBlockedTasks = NULL;
static uint32_t idleTaskStack[STACK_SIZE];
static TCB idleTaskTCB;
static TCB *idleTaskTCBptr = &idleTaskTCB;
static TaskNode idleTaskNode;
static TaskNode *prvIdleTask;
static TaskNode *idleTaskNodePtr = &idleTaskNode;

static STATUS prvAddTaskNodeToReadyList (TaskNode *task);
static TaskNode *prvGetHighestTaskReadyToExecute ();
static void prvAddTaskToBlockedList (TaskNode *task);
static void prvUnblockDelayedTasksReadyToUnblock ();
static TaskNode *createIdleTask ();
static void idleTask ();
static void prvCheckCurTaskForStackOverflow ();

/**
 * @brief Initializes a task's stack frame.
 * @details This will populate the stack with a dummy xPSR and LR value to allow the task to begin execution.
 * These values will be overwritten once the task starts executing. Also, this will fill the bottom of the stack with canary values
 * so that stack overflows can be detected. The stack is also populated with usage watermarks.
 * 
 * @param taskStack The array created by the user
 * @param taskFunc The address of the task function
 * 
 * @warning This function should not be called by user code.
 * 
 * @return Returns a pointer to the top of the stack.
 */
uint32_t *
initTaskStackFrame (uint32_t taskStack[], void (*taskFunc) (void))
{
  for (int i = 0; i < STACK_SIZE; ++i)
    {
      taskStack[i] = STACK_USAGE_WATERMARK;
    }

  taskStack[0] = STACK_OVERFLOW_CANARY_VALUE;
  taskStack[1] = STACK_OVERFLOW_CANARY_VALUE;

  taskStack[STACK_SIZE - 1] = 0x01000000;                 /* xPSR */
  taskStack[STACK_SIZE - 2] = ((uint32_t)taskFunc) | 0x1; /* PC */
  taskStack[STACK_SIZE - 3] = 0xFFFFFFFD;                 /* LR */
  taskStack[STACK_SIZE - 4] = 0x00000000;                 /* R12 */
  taskStack[STACK_SIZE - 5] = 0x00000000;                 /* R3 */
  taskStack[STACK_SIZE - 6] = 0x00000000;                 /* R2 */
  taskStack[STACK_SIZE - 7] = 0x00000000;                 /* R1 */
  taskStack[STACK_SIZE - 8] = 0x00000000;                 /* R0 */
  taskStack[STACK_SIZE - 9] = 0x00000000;                 /* R11 */
  taskStack[STACK_SIZE - 10] = 0x00000000;                /* R10 */
  taskStack[STACK_SIZE - 11] = 0x00000000;                /* R9 */
  taskStack[STACK_SIZE - 12] = 0x00000000;                /* R8 */
  taskStack[STACK_SIZE - 13] = 0x00000000;                /* R7 */
  taskStack[STACK_SIZE - 14] = 0x00000000;                /* R6 */
  taskStack[STACK_SIZE - 15] = 0x00000000;                /* R5 */
  taskStack[STACK_SIZE - 16] = 0x00000000;                /* R4 */

  return &taskStack[STACK_SIZE - 16];
}

STATUS
createTask (uint32_t taskStack[], void (*taskFunc) (void),
            unsigned int priority, TCB *userAllocatedTCB,
            TaskNode *userAllocatedTaskNode)
{
  if (!taskFunc || !userAllocatedTCB || !userAllocatedTaskNode)
    return STATUS_FAILURE;
  if (priority >= MAX_PRIORITIES)
    return STATUS_FAILURE;
  if (STACK_SIZE < 18)
    return STATUS_FAILURE;

  userAllocatedTCB->sp = initTaskStackFrame (taskStack, taskFunc);
  userAllocatedTCB->priority = priority;
  userAllocatedTCB->id = prvCurTaskIDNum;
  prvCurTaskIDNum++;
  userAllocatedTCB->stackFrameLowerBoundAddr = &taskStack[0];

  /* Insert at end of tasks linked list */
  userAllocatedTaskNode->taskTCB = userAllocatedTCB;
  userAllocatedTaskNode->next = NULL;

  STATUS resStatus;
  systemENTER_CRITICAL ();
  {
    resStatus = prvAddTaskNodeToReadyList (userAllocatedTaskNode);
  }
  systemEXIT_CRITICAL ();

  return resStatus;
}

/**
 * @brief This interrupt handler will be called every 1 ms, checking if any tasks need to be unblocked or blocked,
 * and whether a context switch is needed.
 * 
 * @warning This function should not be called from user code.
 */
void
SysTick_Handler ()
{
  msTicks++;

  prvUnblockDelayedTasksReadyToUnblock ();

  if (curTask == NULL)
    {
      return;
    }

  uint32_t curExecutingPriority = curTask->taskTCB->priority;

  TaskNode *highestPriorityPossibleExecute
      = prvGetHighestTaskReadyToExecute ();

  /* Check if a higher priority task is ready to execute */
  if (curExecutingPriority < highestPriorityPossibleExecute->taskTCB->priority)
    {
      prvNextTask = highestPriorityPossibleExecute;
      setPendSVPending ();
      return;
    }

  if (curTask->next == NULL)
    {
      if (highestPriorityPossibleExecute->taskTCB->id != curTask->taskTCB->id)
        {
          /* There is another task of equal priority, time to switch. */
          prvNextTask = highestPriorityPossibleExecute;
          setPendSVPending ();
          return;
        }
    }
  else
    {
      /* There is another task of equal priority, time to switch. */
      prvNextTask = curTask->next;
      setPendSVPending ();
      return;
    }
}

/**
 * @brief This interrupt will be pended when a context switch is needed.
 * 
 * @note This interrupt will run only after all other pending interupts have finished executing.
 * @note prvNextTask will be set when this interrupt is pended.
 * @warning This function should never be called by user code.
 */
void
PendSV_Handler ()
{
  prvCheckCurTaskForStackOverflow ();

  uint32_t spToSave;
  __asm volatile ("mrs r0, PSP\n"
                  "stmdb r0!, {r4-r11}\n"
                  "mov %[spToSave], r0\n"
                  : [spToSave] "=r"(spToSave));

  curTask->taskTCB->sp = (uint32_t *)spToSave;

  uint32_t nextSP;
  systemENTER_CRITICAL ();
  {
    nextSP = (uint32_t)prvNextTask->taskTCB->sp;
    curTask = prvNextTask;
  }
  systemEXIT_CRITICAL ();

  __asm volatile ("mov r2, %[nextSP]\n"
                  "ldmia r2!, {r4-r11}\n"
                  "msr PSP, r2\n"
                  :
                  : [nextSP] "r"(nextSP));

  __asm volatile ("ldr lr, =0xFFFFFFFD\n"
                  "bx lr\n");
}

/**
 * @brief This interrupt will be executed when the scheduler is started.
 * 
 * @note This is where the first task is started.
 * @warning This function should not be called from user code.
 */
void
SVC_Handler ()
{
  TCB *tcbToStart = curTask->taskTCB;
  uint32_t spToStart = (uint32_t)tcbToStart->sp;

  __asm volatile ("ldr r0, %[sp]\n"
                  "ldmia r0!, {r4-r11}\n"
                  "msr PSP, r0\n"
                  "ldr lr, =0xFFFFFFFD\n"
                  "bx lr\n"
                  :
                  : [sp] "m"(spToStart));
}

void
startScheduler ()
{
  prvIdleTask = createIdleTask ();
  curTask = prvGetHighestTaskReadyToExecute ();
  __asm volatile ("svc #0");
}

/**
 * @brief This function will be called when a context-switch is needed.
 * 
 * @warning This function should not be called from user code.
 */
void
setPendSVPending ()
{
  ICSR |= (1 << 28);
}

void
taskDelay (uint32_t ticksToDelay)
{
  systemENTER_CRITICAL ();
  {
    uint32_t curTaskID = curTask->taskTCB->id;
    uint32_t curTaskPriority = curTask->taskTCB->priority;

    curTask->taskTCB->delayedUntil = msTicks + ticksToDelay;

    /* Remove the task from the ready list */
    TaskNode *cur = readyTasksList[curTaskPriority];
    TaskNode *prev = NULL;

    if (cur->next == NULL)
      {
        /* This is the only task for this priority, and it must be curTask */
        readyTasksList[curTaskPriority] = NULL;
        prvNextTask = prvGetHighestTaskReadyToExecute ();
        prvAddTaskToBlockedList (curTask);
        systemEXIT_CRITICAL ();
        setPendSVPending ();
        return;
      }

    /* Check if curTask is the head of the priority */
    if (cur->taskTCB->id == curTaskID)
      {
        readyTasksList[curTaskPriority] = curTask->next;

        prvNextTask = prvGetHighestTaskReadyToExecute ();
        prvAddTaskToBlockedList (curTask);
        systemEXIT_CRITICAL ();
        setPendSVPending ();
        return;
      }

    /* There is more than one task for the current priority */
    while (cur->taskTCB->id != curTaskID)
      {
        prev = cur;
        cur = cur->next;
      }

    TaskNode *afterCur = cur->next;
    prev->next = afterCur;

    prvNextTask = prvGetHighestTaskReadyToExecute ();
    prvAddTaskToBlockedList (curTask);
  }
  systemEXIT_CRITICAL ();
  setPendSVPending ();
}

/**
 * @brief Add a task to the ready tasks list.
 * 
 * @param task The task's TaskNode to move to the ready list.
 * 
 * @return Returns STATUS_SUCCESS if the task was successfully added to the ready list, and STATUS_FAILURE if it was failed to be added.
 * 
 * @warning This function should not be called from user code.
 */
static STATUS
prvAddTaskNodeToReadyList (TaskNode *task)
{
  /* Safeguards */
  if (task->taskTCB->priority >= MAX_PRIORITIES)
    {
      return STATUS_FAILURE;
    }

  task->next = NULL;

  uint32_t curPriority = task->taskTCB->priority;

  TaskNode *curHead = readyTasksList[curPriority];

  if (curHead == NULL)
    {
      /* This is the first node for this priority */
      readyTasksList[curPriority] = task;
      return STATUS_SUCCESS;
    }

  /* Get to the end of the LL */
  while (curHead->next != NULL)
    {
      curHead = curHead->next;
    }

  curHead->next = task;
  return STATUS_SUCCESS;
}

/**
 * @brief This function will get the highest priority task ready to execute.
 * 
 * @return Returns the TaskNode of the highest priority task ready to execute.
 * 
 * @warning This function should not be called by user code.
 */
static TaskNode *
prvGetHighestTaskReadyToExecute ()
{
  int idx = MAX_PRIORITIES - 1;

  while (idx >= 0)
    {
      if (readyTasksList[idx] != NULL)
        {
          return readyTasksList[idx];
        }
      --idx;
    }

  return prvIdleTask;
}

/**
 * @brief This function will add a task to the blocked list.
 * 
 * @param task The TaskNode of the task to be added to the blocked list
 * 
 * @warning This function should not be called by user code.
 */
static void
prvAddTaskToBlockedList (TaskNode *task)
{
  task->next = NULL;

  if (prvBlockedTasks == NULL)
    {
      prvBlockedTasks = task;
      return;
    }

  TaskNode *cur = prvBlockedTasks;

  while (cur->next != NULL)
    {
      cur = cur->next;
    }

  cur->next = task;
}

/**
 * @brief This function will unblock every task that is delayed and ready to be unblocked.
 * 
 * @note This function is called at the frequency of 1ms (the frequency of SysTick).
 * 
 * @warning This function should not be called by user code.
 */
static void
prvUnblockDelayedTasksReadyToUnblock ()
{
  TaskNode *cur = prvBlockedTasks;
  TaskNode *prev = NULL;

  if (cur == NULL)
    {
      return;
    }

  if (cur->next == NULL)
    {
      if (cur->taskTCB->delayedUntil == msTicks)
        {
          prvBlockedTasks = NULL;
          prvAddTaskNodeToReadyList (cur);
          return;
        }
    }

  while (cur != NULL)
    {
      TaskNode *tempNext = cur->next;
      if (cur->taskTCB->delayedUntil == msTicks)
        {
          if (prev == NULL)
            {
              prvBlockedTasks = cur->next;
              prvAddTaskNodeToReadyList (cur);
            }
          else
            {
              prev->next = cur->next;
              prvAddTaskNodeToReadyList (cur);
            }
        }
      else
        {
          prev = cur;
        }
      cur = tempNext;
    }
}

/**
 * @brief This function will create the idle task.
 * 
 * @return This function returns the pointer to the IdleTask's TaskNode
 * 
 * @warning This function should not be called by user code.
 */
static TaskNode *
createIdleTask ()
{
  idleTaskTCBptr->sp = initTaskStackFrame (idleTaskStack, &idleTask);
  idleTaskTCBptr->priority = 0;
  idleTaskTCBptr->id = prvCurTaskIDNum;
  idleTaskTCBptr->stackFrameLowerBoundAddr = &idleTaskStack[0];
  idleTaskNodePtr->taskTCB = idleTaskTCBptr;
  idleTaskNodePtr->next = NULL;
  prvCurTaskIDNum++;

  return idleTaskNodePtr;
}

/**
 * @brief This function is the Idle Task. It will run when no other task is ready to run.
 * 
 * @warning This function should not be called by user code.
 */
static void
idleTask ()
{
  for (;;)
    {
      __asm volatile ("wfi");
    }
}

/**
 * @brief This task will check the current running task for a stack overflow.
 * 
 * @note This function will be called when the current task is ready to be switched out.
 * 
 * @warning This function should not be called by user code.
 */
static void
prvCheckCurTaskForStackOverflow ()
{
  uint32_t *curTaskStackFrameLowerBound;
  systemENTER_CRITICAL ();
  {
    curTaskStackFrameLowerBound = curTask->taskTCB->stackFrameLowerBoundAddr;
  }
  systemEXIT_CRITICAL ();

  if ((*curTaskStackFrameLowerBound != STACK_OVERFLOW_CANARY_VALUE)
      || (*(curTaskStackFrameLowerBound + 1) != STACK_OVERFLOW_CANARY_VALUE))
    {
      handleStackOverflow ();
    }
}

void __attribute__ ((weak))
handleStackOverflow ()
{
  for (;;)
    {
    }
}

uint32_t
getCurTaskWordsAvailable ()
{
  uint32_t *curTaskStackFrameLowerBound;
  systemENTER_CRITICAL ();
  {
    curTaskStackFrameLowerBound = curTask->taskTCB->stackFrameLowerBoundAddr;
  }
  systemEXIT_CRITICAL ();

  curTaskStackFrameLowerBound
      += 2; /* Skip the 2 canary values (assumes no stack overflow) */

  uint32_t amtWordsAvailable = 0;

  while (*(curTaskStackFrameLowerBound) == STACK_USAGE_WATERMARK)
    {
      curTaskStackFrameLowerBound++;
      amtWordsAvailable++;
    }

  return amtWordsAvailable;
}
