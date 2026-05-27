#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

#define configUSE_PREEMPTION                    1
#define configUSE_IDLE_HOOK                     0
#define configUSE_TICK_HOOK                     0
#define configCPU_CLOCK_HZ                      ((unsigned long)240000000)
#define configTICK_RATE_HZ                      ((TickType_t)100)
#define configMAX_PRIORITIES                    5
#define configMINIMAL_STACK_SIZE                512
#define configTOTAL_HEAP_SIZE                   (40 * 1024)
#define configMAX_TASK_NAME_LEN                 16
#define configUSE_TRACE_FACILITY                0
#define configUSE_16_BIT_TICKS                  0
#define configIDLE_SHOULD_YIELD                 1
#define configUSE_MUTEXES                       0
#define configUSE_COUNTING_SEMAPHORES           0
#define configQUEUE_REGISTRY_SIZE               0
#define configUSE_APPLICATION_TASK_TAG          0
#define configUSE_RECURSIVE_MUTEXES             0
#define configUSE_EVENT_GROUPS                  0
#define configUSE_STREAM_BUFFERS                0
#define configUSE_CO_ROUTINES                   0
#define configUSE_TIMERS                        0
#define configSUPPORT_DYNAMIC_ALLOCATION        1
#define configSUPPORT_STATIC_ALLOCATION         0

#define configUSE_TICKLESS_IDLE                 0
#define configEXPECTED_IDLE_TIME_BEFORE_SLEEP   2
#define configUSE_MALLOC_FAILED_HOOK            0
#define configCHECK_FOR_STACK_OVERFLOW          0
#define configUSE_DAEMON_TASK_STARTUP_HOOK      0
#define configUSE_CORE_AFFINITY                 0
#define configNUMBER_OF_CORES                   1

#define INCLUDE_vTaskPrioritySet                0
#define INCLUDE_uxTaskPriorityGet               0
#define INCLUDE_vTaskDelete                     0
#define INCLUDE_vTaskSuspend                    0
#define INCLUDE_vTaskDelay                      1
#define INCLUDE_xTaskDelayUntil                 0
#define INCLUDE_xTaskGetIdleTaskHandle          0
#define INCLUDE_xTaskAbortDelay                 0
#define INCLUDE_uxTaskGetStackHighWaterMark     0
#define INCLUDE_eTaskGetState                   0
#define INCLUDE_xTaskResumeFromISR              1
#define INCLUDE_xTaskGetSchedulerState          0
#define INCLUDE_xTaskGetCurrentTaskHandle       0

#define configUSE_NEWLIB_REENTRANT              0
#define configUSE_PICOLIBC_TLS                  0
#define configUSE_C_RUNTIME_TLS_SUPPORT         0

#define configUSE_LIST_DATA_INTEGRITY_CHECK_BYTES  0
#define configUSE_MINI_LIST_ITEM                1

#ifndef __ASSEMBLER__
extern void vPortAssertFailed(const char *file, int line);
#endif
#define configASSERT(x) if (!(x)) { vPortAssertFailed(__FILE__, __LINE__); }

#endif
