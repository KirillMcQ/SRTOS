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
static void prvConfigureMpuRegionForCurTaskStack ();

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

void
SVC_Handler ()
{
  TCB *tcbToStart = curTask->taskTCB;
  uint32_t spToStart = (uint32_t)tcbToStart->sp;
  
  /* Ensure Thread Mode is executed in unprivileged mode */
  __asm volatile ("mrs r0, CONTROL\n"
    "orr r0, r0, #1\n"
    "msr CONTROL, r0\n"
    "isb\n");
    
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
  prvConfigureMpuRegionForCurTaskStack ();
  __asm volatile ("svc #0");
}

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

/* Maybe make this return a STATUS? */
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

static void
idleTask ()
{
  for (;;)
    {
      __asm volatile ("wfi");
    }
}

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

static void
prvConfigureMpuRegionForCurTaskStack ()
{
  /* Region 3 protects the current task's stack */
  MPU_RNR = 3U;

  uint32_t stackSizeInBytes = STACK_SIZE * 4;

  uint32_t mpuSizeField;

  for (uint32_t i = 0; i < 32; i++)
    {
      if ((1U << i) >= stackSizeInBytes)
        {
          mpuSizeField = i - 1;
          break;
        }
    }
  
    uint32_t curBaseAddr = (uint32_t)(curTask->taskTCB->stackFrameLowerBoundAddr);

    uint32_t alignedBaseAddr = curBaseAddr & ~((1U << (mpuSizeField + 1)) - 1);

    MPU_RBAR = alignedBaseAddr;

    uint32_t rasr = 0;

    rasr |= (1U << MPU_RASR_ENABLE_BIT);
    rasr |= (mpuSizeField << MPU_RASR_SIZE_START_BIT);

    /* SCB = 0b110 TEX = 0b000
    * recommended by https://interrupt.memfault.com/blog/fix-bugs-and-secure-firmware-with-the-mpu
    * */

    rasr &= ~(1U << MPU_RASR_ATTRS_B_BIT);
    rasr |= (1U << MPU_RASR_ATTRS_C_BIT);
    rasr |= (1U << MPU_RASR_ATTRS_S_BIT);
    rasr &= ~(0x7U << MPU_RASR_ATTRS_TEX_START_BIT);

    /* AP = 0b011 = rw for both privilege levels*/
    rasr |= (0x3U << MPU_RASR_ATTRS_AP_START_BIT);
    rasr &= ~(1U << (MPU_RASR_ATTRS_AP_START_BIT + 2));

    /* XN = 1 = Nothing should be executed from a task's stack */
    rasr |= (1U << MPU_RASR_ATTRS_XN_BIT);

    MPU_RASR = rasr;

    __asm volatile("dsb\n");
    __asm volatile("isb\n");
}
