#include "utils.h"

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

// Called if malloc fails
void vApplicationMallocFailedHook() {
    xassert(!"Malloc failed. Increase heap size.");
}

// Called if a stack overflow is detected if 
// configCHECK_FOR_STACK_OVERFLOW is defined to 1 or 2
void vApplicationStackOverflowHook(TaskHandle_t pxTask, char *pcTaskName) {
    xassert(!"Stack overflowed.");
}

void vApplicationIdleHook() { }

void vApplicationTickHook() { }
