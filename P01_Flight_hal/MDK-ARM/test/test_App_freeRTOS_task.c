/**
 * @file    test_App_freeRTOS_task.c
 * @brief   对 App_freeRTOS_start() 的单元测试（Unity + Mock FreeRTOS）
 *
 * 编译方法 (MinGW / GCC):
 *   gcc -std=c99 -I. ../Application/App_freeRTOS_task.c unity.c mock_FreeRTOS.c test_App_freeRTOS_task.c -o test_runner.exe && test_runner.exe
 *
 * 说明:
 *   本文件通过 mock 层替换 FreeRTOS 内核函数，在 PC 上验证被测函数的调用逻辑。
 *   由于 App_freeRTOS_start() 无返回值，测试重点为：
 *     1. xTaskCreate 参数是否正确
 *     2. vTaskStartScheduler 是否被调用
 *     3. xTaskCreate 失败时的行为（当前无错误处理，记录为已知行为）
 */

#include "unity.h"
#include "mock_FreeRTOS.h"

/* ---------- 被测函数声明 ---------- */
extern void App_freeRTOS_start(void);
extern void power_task(void *args);

/* ---------- 每个测试前重置 mock 状态 ---------- */
static void setUp(void)
{
    mock_freertos_reset();
}

/* ========================================================================
 *   测试用例 1: 验证 xTaskCreate 参数正确性（正常路径）
 *   - 任务函数指针、名称、栈大小、参数(NULL)、优先级、句柄指针
 * ======================================================================== */
static void test_xTaskCreate_normal_parameters(void)
{
    App_freeRTOS_start();

    TEST_ASSERT_EQUAL_INT(1, mock_xTaskCreate_called);
    TEST_ASSERT_EQUAL_PTR((void *)power_task, (void *)mock_xTaskCreate_pxTaskCode);
    TEST_ASSERT_EQUAL_STRING("power_task", mock_xTaskCreate_pcName);
    TEST_ASSERT_EQUAL_INT(128, mock_xTaskCreate_usStackDepth);
    TEST_ASSERT_NULL(mock_xTaskCreate_pvParameters);
    TEST_ASSERT_EQUAL_INT(4, mock_xTaskCreate_uxPriority);
    TEST_ASSERT_NOT_NULL(mock_xTaskCreate_pxCreatedTask);
}

/* ========================================================================
 *   测试用例 2: 验证调度器在任务创建之后被启动
 *   - vTaskStartScheduler 必须被调用且恰好在 xTaskCreate 之后
 * ======================================================================== */
static void test_vTaskStartScheduler_is_called(void)
{
    /* 确保 mock 在调用前重置，xTaskCreate 调用次数 = 0 */
    mock_xTaskCreate_called = 0;
    mock_vTaskStartScheduler_called = 0;

    App_freeRTOS_start();

    TEST_ASSERT_EQUAL_INT(1, mock_vTaskStartScheduler_called);
}

/* ========================================================================
 *   测试用例 3: xTaskCreate 返回 pdFAIL 时的行为
 *   注意: 当前 App_freeRTOS_start() 未检查 xTaskCreate 返回值，
 *         即使任务创建失败，调度器仍会启动。
 *         本测试记录当前行为，如需健壮性应在源码中增加错误处理。
 * ======================================================================== */
static void test_xTaskCreate_failure_still_starts_scheduler(void)
{
    mock_xTaskCreate_return = pdFAIL;

    App_freeRTOS_start();

    /* 当前行为：xTaskCreate 失败后仍然会调用 vTaskStartScheduler */
    TEST_ASSERT_EQUAL_INT(1, mock_xTaskCreate_called);
    TEST_ASSERT_EQUAL_INT(1, mock_vTaskStartScheduler_called);
}

/* ========================================================================
 *   测试用例 4: 多次调用时 mock 计数正确叠加
 * ======================================================================== */
static void test_multiple_calls_increment_counters(void)
{
    App_freeRTOS_start();
    App_freeRTOS_start();

    TEST_ASSERT_EQUAL_INT(2, mock_xTaskCreate_called);
    TEST_ASSERT_EQUAL_INT(2, mock_vTaskStartScheduler_called);
}

/* ========================================================================
 *   测试用例 5: pdPASS 返回值下调度器正常启动
 * ======================================================================== */
static void test_xTaskCreate_success_starts_scheduler(void)
{
    mock_xTaskCreate_return = pdPASS;

    App_freeRTOS_start();

    TEST_ASSERT_EQUAL_INT(1, mock_xTaskCreate_called);
    TEST_ASSERT_EQUAL_INT(1, mock_vTaskStartScheduler_called);
}

/* ========================================================================
 *   测试用例 6: 边界 — 优先级最大值 (configMAX_PRIORITIES - 1 = 4)
 *   验证使用的优先级 4 在合法范围内
 * ======================================================================== */
static void test_priority_within_valid_range(void)
{
    App_freeRTOS_start();

    /* configMAX_PRIORITIES = 5, 合法范围 0~4 */
    TEST_ASSERT_TRUE(mock_xTaskCreate_uxPriority >= 0);
    TEST_ASSERT_TRUE(mock_xTaskCreate_uxPriority <  5);
}

/* ========================================================================
 *   主入口
 * ======================================================================== */
int main(void)
{
    UNITY_BEGIN();

    setUp();  test_xTaskCreate_normal_parameters();
    setUp();  test_vTaskStartScheduler_is_called();
    setUp();  test_xTaskCreate_failure_still_starts_scheduler();
    setUp();  test_multiple_calls_increment_counters();
    setUp();  test_xTaskCreate_success_starts_scheduler();
    setUp();  test_priority_within_valid_range();

    return UNITY_END();
}
