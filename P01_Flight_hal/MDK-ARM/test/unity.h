/*
 * Unity - 极简测试框架 (嵌入式C用)
 * 仅实现 assert 所需的最小 API
 */
#ifndef UNITY_H_
#define UNITY_H_

#ifdef __cplusplus
extern "C" {
#endif

void unity_begin(const char *test_file);
int  unity_end(void);

void unity_pass(void);
void unity_fail(const char *file, int line, const char *msg);
void unity_check(int condition, const char *file, int line, const char *msg);

#define UNITY_BEGIN()             unity_begin(__FILE__)
#define UNITY_END()               return unity_end()

#define TEST_ASSERT(cond)         unity_check((cond) ? 1 : 0, __FILE__, __LINE__, #cond)
#define TEST_ASSERT_TRUE(cond)    unity_check((cond) ? 1 : 0, __FILE__, __LINE__, #cond)
#define TEST_ASSERT_FALSE(cond)   unity_check((cond) ? 0 : 1, __FILE__, __LINE__, #cond)
#define TEST_ASSERT_NULL(p)       unity_check(((p) == NULL) ? 1 : 0, __FILE__, __LINE__, #p " is NULL")
#define TEST_ASSERT_NOT_NULL(p)   unity_check(((p) != NULL) ? 1 : 0, __FILE__, __LINE__, #p " is NOT NULL")
#define TEST_ASSERT_EQUAL(e, a)   unity_check(((e) == (a)) ? 1 : 0, __FILE__, __LINE__, "EQUAL")
#define TEST_ASSERT_EQUAL_INT(e,a) unity_check(((e) == (a)) ? 1 : 0, __FILE__, __LINE__, "EQUAL_INT")
#define TEST_ASSERT_EQUAL_PTR(e,a) unity_check(((e) == (a)) ? 1 : 0, __FILE__, __LINE__, "EQUAL_PTR")
#define TEST_ASSERT_EQUAL_STRING(e,a) unity_check((e != NULL && a != NULL && strcmp(e,a)==0) ? 1 : 0, __FILE__, __LINE__, "EQUAL_STRING")

extern int unity_total;
extern int unity_passed;
extern int unity_failed;

#ifdef __cplusplus
}
#endif

#endif /* UNITY_H_ */
