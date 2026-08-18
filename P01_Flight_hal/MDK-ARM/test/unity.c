#include <stdio.h>
#include "unity.h"

int unity_total  = 0;
int unity_passed = 0;
int unity_failed = 0;

void unity_begin(const char *test_file)
{
    unity_total  = 0;
    unity_passed = 0;
    unity_failed = 0;
    printf("\n==== Running tests in: %s ====\n", test_file);
}

int unity_end(void)
{
    printf("\n---- Results: %d total, %d passed, %d failed ----\n",
           unity_total, unity_passed, unity_failed);
    return (unity_failed > 0) ? 1 : 0;
}

void unity_check(int condition, const char *file, int line, const char *msg)
{
    unity_total++;
    if (condition) {
        unity_passed++;
        printf("  [PASS] %s:%d - %s\n", file, line, msg);
    } else {
        unity_failed++;
        printf("  [FAIL] %s:%d - %s (condition false)\n", file, line, msg);
    }
}
