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
    CU_ASSERT_STRING_EQUAL(dstr_to_cstr_const(str), "something");
    dstr_decref(str);
}

void new_dstr_from_dstr()
{
    dstr *src = dstr_with_initial("something else");
    dstr *cpy;
}

int main()
{
   CU_pSuite pSuite = NULL;

   if (CU_initialize_registry() != CUE_SUCCESS)
      return CU_get_error();

   pSuite = CU_add_suite("Suite_1", 0,0);
   if (!pSuite){
      CU_cleanup_registry();
      return CU_get_error();
   }

   if (!CU_add_test(pSuite, "dstr_new", new_dstr_test) ||
       !CU_add_test(pSuite, "dstr_with_initial", new_dstr_initial_test)){
      CU_cleanup_registry();
      return CU_get_error();
   }

   CU_basic_set_mode(CU_BRM_VERBOSE);
   CU_basic_run_tests();
   CU_cleanup_registry();
   return CU_get_error();
}
