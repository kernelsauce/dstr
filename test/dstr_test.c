#include <CUnit/CUnit.h>
#include "CUnit/Basic.h"
#include "dstr.h"

void new_dstr_test()
{
    dstr *new_dstr = dstr_new();
    CU_ASSERT_PTR_NOT_NULL(new_dstr);
    dstr_decref(new_dstr);
}

void new_dstr_initial_test()
{
    dstr *str = dstr_with_initial("something");
    CU_ASSERT_PTR_NOT_NULL(str);
    CU_ASSERT_STRING_EQUAL(dstr_to_cstr_const(str), "something");
    dstr_decref(str);
}

void new_dstr_from_dstr()
{
    dstr *src = dstr_with_initial("something else");
    CU_ASSERT_PTR_NOT_NULL(src);
    dstr *cpy = dstr_copy(src);
    CU_ASSERT_PTR_NOT_NULL(cpy);
    CU_ASSERT_STRING_EQUAL(dstr_to_cstr_const(cpy), "something else");
    dstr_decref(cpy);
    dstr_decref(src);
}

void new_dstr_prealloc()
{
    dstr *str = dstr_with_prealloc(100);
    CU_ASSERT_PTR_NOT_NULL(str);
    CU_ASSERT_EQUAL(str->allocd_mem, 100);
    dstr_decref(str);
}

void test_dstr_to_cstr()
{
    const char *cstr;
    dstr *str;

    str = dstr_with_initial("something");
    CU_ASSERT_PTR_NOT_NULL(str);

    cstr = dstr_to_cstr_const(str);
    CU_ASSERT_PTR_NOT_NULL(cstr);
    CU_ASSERT_STRING_EQUAL(cstr, "something");
    dstr_decref(str);
}

void test_decref()
{
    dstr *str = dstr_new();
    dstr_incref(str);
    CU_ASSERT_EQUAL(str->ref, 2);
    dstr_decref(str);
    CU_ASSERT_EQUAL(str->ref, 1);
    dstr_decref(str);
}

void test_incref()
{
    dstr *str = dstr_new();
    CU_ASSERT_EQUAL(str->ref, 1);
    dstr_incref(str);
    CU_ASSERT_EQUAL(str->ref, 2);
    dstr_decref(str);
    dstr_decref(str);
}

void test_dstr_append()
{
    dstr *str = dstr_with_initial("concat me");
    dstr *str2 = dstr_with_initial(" to me");
    dstr_append(str, str2);
    CU_ASSERT_STRING_EQUAL(dstr_to_cstr_const(str), "concat me to me");
    dstr_decref(str);
    dstr_decref(str2);
}

void test_dstr_append_decref()
{
    dstr *str = dstr_with_initial("concat me");
    dstr *str2 = dstr_with_initial(" to me");
    dstr_incref(str2);

    dstr_append_decref(str, str2);
    CU_ASSERT_EQUAL(str->ref, 1);
    CU_ASSERT_STRING_EQUAL(dstr_to_cstr_const(str), "concat me to me");
    dstr_decref(str);
    dstr_decref(str2);
}

void test_dstr_append_cstr()
{
    dstr *str = dstr_with_initial("concat me");
    dstr_append_cstr(str, " to me");
    CU_ASSERT_STRING_EQUAL(dstr_to_cstr_const(str), "concat me to me");
    dstr_decref(str);
}

void test_dstr_clear()
{
    dstr *str = dstr_with_initial("some data");
    CU_ASSERT_STRING_EQUAL(dstr_to_cstr_const(str), "some data");
    dstr_clear(str);
    CU_ASSERT_STRING_EQUAL(dstr_to_cstr_const(str), "");
    dstr_decref(str);
}

void test_dstr_compact()
{
    dstr *str = dstr_with_initial("some data");
    CU_ASSERT_STRING_EQUAL(dstr_to_cstr_const(str), "some data");
    CU_ASSERT_EQUAL(str->allocd_mem, 10);
    dstr_clear(str);
    dstr_append_cstr(str, "hi!");
    dstr_compact(str);
    CU_ASSERT_EQUAL(str->allocd_mem, 4);
    dstr_decref(str);
}

void test_dstr_growth_rate()
{
    dstr *str = dstr_with_initial("data");
    int alloc_before = str->allocd_mem;

    dstr_growth_rate(str, 5);
    dstr_append_cstr(str, "data");
    CU_ASSERT_EQUAL(str->allocd_mem, (alloc_before + (8 * sizeof(char))) * str->growth_rate);
    dstr_decref(str);
}

int main()
{
   CU_pSuite pSuite;

   if (CU_initialize_registry() != CUE_SUCCESS)
      return CU_get_error();

   pSuite = CU_add_suite("dstr", 0,0);
   if (!pSuite){
      CU_cleanup_registry();
      return CU_get_error();
   }

   if (!CU_add_test(pSuite, "dstr_new", new_dstr_test) ||
           !CU_add_test(pSuite, "dstr_with_initial", new_dstr_initial_test) ||
           !CU_add_test(pSuite, "dstr_copy", new_dstr_from_dstr) ||
           !CU_add_test(pSuite, "dstr_prealloc", new_dstr_prealloc) ||
           !CU_add_test(pSuite, "dstr_decref", test_decref) ||
           !CU_add_test(pSuite, "dstr_incref", test_incref) ||
           !CU_add_test(pSuite, "dstr_append", test_dstr_append) ||
           !CU_add_test(pSuite, "dstr_append_decref", test_dstr_append_decref) ||
           !CU_add_test(pSuite, "dstr_append_cstr", test_dstr_append_cstr) ||
           !CU_add_test(pSuite, "dstr_clear", test_dstr_clear) ||
           !CU_add_test(pSuite, "dstr_compact", test_dstr_compact) ||
           !CU_add_test(pSuite, "dstr_growth_rate", test_dstr_growth_rate) ||
           !CU_add_test(pSuite, "dstr_dstr_to_cstr", test_dstr_to_cstr)){
      CU_cleanup_registry();
      return CU_get_error();
   }

   CU_basic_set_mode(CU_BRM_VERBOSE);
   CU_basic_run_tests();
   CU_cleanup_registry();
   return CU_get_error();
}
