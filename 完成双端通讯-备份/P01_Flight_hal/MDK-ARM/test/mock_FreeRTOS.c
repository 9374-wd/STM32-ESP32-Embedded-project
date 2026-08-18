#include "mock_FreeRTOS.h"

/* ---------- mock control variables ---------- */
BaseType_t       mock_xTaskCreate_return    = pdPASS;
int              mock_xTaskCreate_called     = 0;
int              mock_vTaskStartScheduler_called = 0;

TaskFunction_t   mock_xTaskCreate_pxTaskCode     = NULL;
const char *     mock_xTaskCreate_pcName         = NULL;
configSTACK_DEPTH_TYPE mock_xTaskCreate_usStackDepth = 0;
void *           mock_xTaskCreate_pvParameters   = NULL;
UBaseType_t      mock_xTaskCreate_uxPriority     = 0;
TaskHandle_t *   mock_xTaskCreate_pxCreatedTask  = NULL;

/* ---------- mock implementations ---------- */
BaseType_t xTaskCreate(
    TaskFunction_t pxTaskCode,
    const char * const pcName,
    configSTACK_DEPTH_TYPE usStackDepth,
    void *pvParameters,
    UBaseType_t uxPriority,
    TaskHandle_t *pxCreatedTask)
{
    mock_xTaskCreate_called++;
    mock_xTaskCreate_pxTaskCode      = pxTaskCode;
    mock_xTaskCreate_pcName          = pcName;
    mock_xTaskCreate_usStackDepth    = usStackDepth;
    mock_xTaskCreate_pvParameters    = pvParameters;
    mock_xTaskCreate_uxPriority      = uxPriority;
    mock_xTaskCreate_pxCreatedTask   = pxCreatedTask;
    return mock_xTaskCreate_return;
}

void vTaskStartScheduler(void)
{
    mock_vTaskStartScheduler_called++;
}

TickType_t xTaskGetTickCount(void)
{
    return 0;
}

void vTaskDelayUntil(TickType_t *pxPreviousWakeTime, TickType_t xTimeIncrement)
{
    (void)pxPreviousWakeTime;
    (void)xTimeIncrement;
}

void mock_freertos_reset(void)
{
    mock_xTaskCreate_return           = pdPASS;
    mock_xTaskCreate_called           = 0;
    mock_vTaskStartScheduler_called   = 0;

    mock_xTaskCreate_pxTaskCode       = NULL;
    mock_xTaskCreate_pcName           = NULL;
    mock_xTaskCreate_usStackDepth     = 0;
    mock_xTaskCreate_pvParameters     = NULL;
    mock_xTaskCreate_uxPriority       = 0;
    mock_xTaskCreate_pxCreatedTask    = NULL;
}
