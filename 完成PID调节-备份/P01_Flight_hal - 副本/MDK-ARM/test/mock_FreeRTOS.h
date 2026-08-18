#ifndef _MOCK_FREERTOS_H_
#define _MOCK_FREERTOS_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ---------- FreeRTOS type definitions ---------- */
typedef void *          TaskHandle_t;
typedef uint32_t        TickType_t;
typedef int32_t         BaseType_t;
typedef uint32_t        UBaseType_t;
typedef uint16_t        configSTACK_DEPTH_TYPE;
typedef void (*TaskFunction_t)(void *);

#define pdPASS          ((BaseType_t)1)
#define pdFAIL          ((BaseType_t)0)
#define pdTRUE          ((BaseType_t)1)
#define pdFALSE         ((BaseType_t)0)

/* ---------- mock control globals ---------- */
/* 测试可设置这些变量来控制 mock 行为 */
extern BaseType_t       mock_xTaskCreate_return;    /* xTaskCreate 返回值，默认 pdPASS */
extern int              mock_xTaskCreate_called;    /* xTaskCreate 被调用次数 */
extern int              mock_vTaskStartScheduler_called; /* vTaskStartScheduler 被调用次数 */

/* 记录最近一次 xTaskCreate 的参数 */
extern TaskFunction_t   mock_xTaskCreate_pxTaskCode;
extern const char *     mock_xTaskCreate_pcName;
extern configSTACK_DEPTH_TYPE mock_xTaskCreate_usStackDepth;
extern void *           mock_xTaskCreate_pvParameters;
extern UBaseType_t      mock_xTaskCreate_uxPriority;
extern TaskHandle_t *   mock_xTaskCreate_pxCreatedTask;

/* ---------- mock FreeRTOS API ---------- */
BaseType_t xTaskCreate(
    TaskFunction_t pxTaskCode,
    const char * const pcName,
    configSTACK_DEPTH_TYPE usStackDepth,
    void *pvParameters,
    UBaseType_t uxPriority,
    TaskHandle_t *pxCreatedTask);

void vTaskStartScheduler(void);

TickType_t xTaskGetTickCount(void);

void vTaskDelayUntil(TickType_t *pxPreviousWakeTime, TickType_t xTimeIncrement);

/* ---------- mock reset helper ---------- */
void mock_freertos_reset(void);

#ifdef __cplusplus
}
#endif

#endif /* _MOCK_FREERTOS_H_ */
