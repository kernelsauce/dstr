#include <CUnit/CUnit.h>
#include "CUnit/Basic.h"
#include "dstr.h"

/****************************** DYNAMIC STRING ********************************/

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
    CU_ASSERT_EQUAL(str->allocd_mem,
                    (alloc_before + (8 * sizeof(char))) * str->growth_rate);
    dstr_decref(str);
}

/**************************** DYNAMIC STRING LIST  ****************************/

void test_dstr_list_new()
{
    dstr_list *list = dstr_list_new();
    CU_ASSERT_PTR_NOT_NULL(list);
    dstr_list_decref(list);
}

void test_dstr_list_decref()
{
    dstr_list *list = dstr_list_new();
    dstr_list_incref(list);
    dstr_list_decref(list);
    CU_ASSERT_EQUAL(list->ref, 1);
    dstr_list_decref(list);
}

void test_dstr_list_incref()
{
    dstr_list *list = dstr_list_new();
    dstr_list_incref(list);
    CU_ASSERT_EQUAL(list->ref, 2);
    dstr_list_decref(list);
    dstr_list_decref(list);
}

void test_dstr_list_append()
{
    dstr_list *list = dstr_list_new();
    dstr *str1, *str2, *str3;
    dstr *combined;

    str1 = dstr_with_initial("str1");
    str2 = dstr_with_initial("str2");
    str3 = dstr_with_initial("str3");

    CU_ASSERT_PTR_NOT_NULL(list);

    dstr_list_add(list, str1);
    dstr_list_add(list, str2);
    dstr_list_add(list, str3);

    dstr_decref(str1);
    dstr_decref(str2);
    dstr_decref(str3);


    combined = dstr_list_to_dstr(0, list);
    CU_ASSERT_STRING_EQUAL(dstr_to_cstr_const(combined), "str1str2str3");

    dstr_decref(combined);
    dstr_list_decref(list);
}

void test_dstr_list_append_decref()
{
    dstr_list *list = dstr_list_new();
    dstr *combined;

    CU_ASSERT_PTR_NOT_NULL(list);

    dstr_list_add_decref(list, dstr_with_initial("str1"));
    dstr_list_add_decref(list, dstr_with_initial("str2"));
    dstr_list_add_decref(list, dstr_with_initial("str3"));

    combined = dstr_list_to_dstr(0, list);
    CU_ASSERT_STRING_EQUAL(dstr_to_cstr_const(combined), "str1str2str3");

    dstr_decref(combined);
    dstr_list_decref(list);
}

void __traverse_callback(dstr *str, void *append)
{
    dstr_append(append, str);
}

void test_dstr_list_traverse()
{
    dstr *append_to = dstr_new();
    dstr_list *list = dstr_list_new();
    dstr_list_add_decref(list, dstr_with_initial("str1"));
    dstr_list_add_decref(list, dstr_with_initial("str2"));
    dstr_list_add_decref(list, dstr_with_initial("str3"));
    // concat some strings...
    dstr_list_traverse(list, __traverse_callback, append_to);

    CU_ASSERT_STRING_EQUAL(dstr_to_cstr_const(append_to), "str1str2str3");
    dstr_decref(append_to);
    dstr_list_decref(list);
}

void __traverse_reverse_callback(dstr *str, void *append)
{
    dstr_append(append, str);
}

void test_dstr_list_traverse_reverse()
{
    dstr *append_to = dstr_new();
    dstr_list *list = dstr_list_new();
    dstr_list_add_decref(list, dstr_with_initial("str1"));
    dstr_list_add_decref(list, dstr_with_initial("str2"));
    dstr_list_add_decref(list, dstr_with_initial("str3"));
    // concat some strings in reverse...
    dstr_list_traverse_reverse(list, __traverse_reverse_callback, append_to);

    CU_ASSERT_STRING_EQUAL(dstr_to_cstr_const(append_to), "str3str2str1");
    dstr_decref(append_to);
    dstr_list_decref(list);
}

void test_dstr_list_size()
{
    dstr_list *list = dstr_list_new();
    dstr_list_add_decref(list, dstr_with_initial("str1"));
    dstr_list_add_decref(list, dstr_with_initial("str2"));
    dstr_list_add_decref(list, dstr_with_initial("str3"));
    CU_ASSERT_EQUAL(dstr_list_size(list), 3);
    dstr_list_decref(list);
}

/************************** DYNAMIC STRING VECTOR  ****************************/

void test_dstr_vector_new()
{
    dstr_vector *vec = dstr_vector_new();
    CU_ASSERT_PTR_NOT_NULL(vec);
    dstr_vector_decref(vec);
}

void test_dstr_vector_insert()
{
    dstr_vector *vec = dstr_vector_new();
    CU_ASSERT_PTR_NOT_NULL(vec);
    CU_ASSERT(dstr_vector_insert_decref(vec, DSTR_VECTOR_END, dstr_with_initial("lol1")));
    CU_ASSERT(dstr_vector_insert_decref(vec, DSTR_VECTOR_END, dstr_with_initial("lol2")));
    CU_ASSERT(dstr_vector_insert_decref(vec, DSTR_VECTOR_END, dstr_with_initial("lol3")));
    CU_ASSERT(dstr_vector_insert_decref(vec, DSTR_VECTOR_END, dstr_with_initial("lol4")));
    CU_ASSERT(dstr_vector_insert_decref(vec, DSTR_VECTOR_END, dstr_with_initial("lol5")));
    CU_ASSERT(dstr_vector_insert_decref(vec, 3, dstr_with_initial("liksom")));
    CU_ASSERT(dstr_vector_insert_decref(vec, 3, dstr_with_initial("hei")));
    CU_ASSERT(dstr_vector_insert_decref(vec, 3, dstr_with_initial("hei")));

    CU_ASSERT_STRING_EQUAL(dstr_to_cstr_const(vec->arr[0]), "lol1");
    CU_ASSERT_STRING_EQUAL(dstr_to_cstr_const(vec->arr[1]), "lol2");
    CU_ASSERT_STRING_EQUAL(dstr_to_cstr_const(vec->arr[2]), "lol3");
    CU_ASSERT_STRING_EQUAL(dstr_to_cstr_const(vec->arr[3]), "hei");
    CU_ASSERT_STRING_EQUAL(dstr_to_cstr_const(vec->arr[4]), "hei");
    CU_ASSERT_STRING_EQUAL(dstr_to_cstr_const(vec->arr[5]), "liksom");
    CU_ASSERT_STRING_EQUAL(dstr_to_cstr_const(vec->arr[6]), "lol4");
    CU_ASSERT_STRING_EQUAL(dstr_to_cstr_const(vec->arr[7]), "lol5");

    dstr_vector_decref(vec);
}

void test_dstr_vector_remove()
{
    dstr_vector *vec = dstr_vector_new();
    CU_ASSERT_PTR_NOT_NULL(vec);
    CU_ASSERT(dstr_vector_insert_decref(vec, DSTR_VECTOR_END, dstr_with_initial("lol1")));
    CU_ASSERT(dstr_vector_insert_decref(vec, DSTR_VECTOR_END, dstr_with_initial("lol2")));
    CU_ASSERT(dstr_vector_insert_decref(vec, DSTR_VECTOR_END, dstr_with_initial("lol3")));
    CU_ASSERT(dstr_vector_insert_decref(vec, DSTR_VECTOR_END, dstr_with_initial("lol4")));
    CU_ASSERT(dstr_vector_insert_decref(vec, DSTR_VECTOR_END, dstr_with_initial("lol5")));
    CU_ASSERT(dstr_vector_insert_decref(vec, 3, dstr_with_initial("liksom")));
    CU_ASSERT(dstr_vector_insert_decref(vec, 3, dstr_with_initial("hei")));
    CU_ASSERT(dstr_vector_insert_decref(vec, 3, dstr_with_initial("hei")));

    dstr_vector_remove(vec, 1);

    CU_ASSERT_STRING_EQUAL(dstr_to_cstr_const(vec->arr[0]), "lol1");
    CU_ASSERT_STRING_EQUAL(dstr_to_cstr_const(vec->arr[1]), "lol3");
    CU_ASSERT_STRING_EQUAL(dstr_to_cstr_const(vec->arr[2]), "hei");
    CU_ASSERT_STRING_EQUAL(dstr_to_cstr_const(vec->arr[3]), "hei");
    CU_ASSERT_STRING_EQUAL(dstr_to_cstr_const(vec->arr[4]), "liksom");
    CU_ASSERT_STRING_EQUAL(dstr_to_cstr_const(vec->arr[5]), "lol4");
    CU_ASSERT_STRING_EQUAL(dstr_to_cstr_const(vec->arr[6]), "lol5");

    dstr_vector_decref(vec);
}

void test_dstr_vector_push_front()
{
    dstr *str = dstr_with_initial("some data");
    dstr *str2 = dstr_with_initial("some more data");
    dstr_vector *vec = dstr_vector_new();
    dstr_vector_push_front(vec, str);
    dstr_vector_push_front(vec, str2);
    CU_ASSERT_STRING_EQUAL(dstr_to_cstr_const(vec->arr[0]), "some more data");
    CU_ASSERT_STRING_EQUAL(dstr_to_cstr_const(vec->arr[1]), "some data");
    dstr_vector_decref(vec);
    dstr_decref(str);
    dstr_decref(str2);
}

void test_dstr_vector_push_front_decref()
{
    dstr_vector *vec = dstr_vector_new();
    dstr_vector_push_front_decref(vec, dstr_with_initial("some data"));
    dstr_vector_push_front_decref(vec, dstr_with_initial("some more data"));
    dstr_vector_push_front_decref(vec, dstr_with_initial("random data"));
    CU_ASSERT_STRING_EQUAL(dstr_to_cstr_const(vec->arr[0]), "random data");
    CU_ASSERT_STRING_EQUAL(dstr_to_cstr_const(vec->arr[1]), "some more data");
    CU_ASSERT_STRING_EQUAL(dstr_to_cstr_const(vec->arr[2]), "some data");
    dstr_vector_decref(vec);
}

void test_dstr_vector_push_back()
{
    dstr *str = dstr_with_initial("some data");
    dstr *str2 = dstr_with_initial("some more data");
    dstr_vector *vec = dstr_vector_new();
    dstr_vector_push_back(vec, str);
    dstr_vector_push_back(vec, str2);
    CU_ASSERT_STRING_EQUAL(dstr_to_cstr_const(vec->arr[0]), "some data");
    CU_ASSERT_STRING_EQUAL(dstr_to_cstr_const(vec->arr[1]), "some more data");
    dstr_vector_decref(vec);
    dstr_decref(str);
    dstr_decref(str2);
}

void test_dstr_vector_push_back_decref()
{
    dstr_vector *vec = dstr_vector_new();
    dstr_vector_push_back_decref(vec, dstr_with_initial("some data"));
    dstr_vector_push_back_decref(vec, dstr_with_initial("some more data"));
    dstr_vector_push_back_decref(vec, dstr_with_initial("random data"));
    CU_ASSERT_STRING_EQUAL(dstr_to_cstr_const(vec->arr[0]), "some data");
    CU_ASSERT_STRING_EQUAL(dstr_to_cstr_const(vec->arr[1]), "some more data");
    CU_ASSERT_STRING_EQUAL(dstr_to_cstr_const(vec->arr[2]), "random data");
    dstr_vector_decref(vec);
}

void test_dstr_vector_back()
{
    dstr_vector *vec = dstr_vector_new();
    dstr_vector_push_back_decref(vec, dstr_with_initial("some data"));
    dstr_vector_push_back_decref(vec, dstr_with_initial("some more data"));

    CU_ASSERT_STRING_EQUAL(dstr_to_cstr_const(dstr_vector_back(vec)), "some more data");
    dstr_vector_pop_back(vec);
    CU_ASSERT_STRING_EQUAL(dstr_to_cstr_const(dstr_vector_back(vec)), "some data");
    dstr_vector_pop_back(vec);

    dstr_vector_decref(vec);
}

void test_dstr_vector_front()
{
    dstr_vector *vec = dstr_vector_new();
    dstr_vector_push_front_decref(vec, dstr_with_initial("some data"));
    dstr_vector_push_front_decref(vec, dstr_with_initial("some more data"));
    dstr_vector_push_front_decref(vec, dstr_with_initial("even more data"));

    CU_ASSERT_STRING_EQUAL(dstr_to_cstr_const(dstr_vector_front(vec)), "even more data");
    dstr_vector_pop_front(vec);
    CU_ASSERT_STRING_EQUAL(dstr_to_cstr_const(dstr_vector_front(vec)), "some more data");
    dstr_vector_pop_front(vec);
    CU_ASSERT_STRING_EQUAL(dstr_to_cstr_const(dstr_vector_front(vec)), "some data");
    dstr_vector_pop_front(vec);

    dstr_vector_decref(vec);
}

void test_dstr_vector_is_empty()
{
    dstr_vector *vec = dstr_vector_new();
    CU_ASSERT(dstr_vector_is_empty(vec));
    dstr_vector_push_front_decref(vec, dstr_with_initial("some more data"));
    CU_ASSERT(!dstr_vector_is_empty(vec));
    dstr_vector_decref(vec);
}

void test_dstr_vector_size()
{
    dstr_vector *vec = dstr_vector_prealloc(10000);
    int i;

    CU_ASSERT_EQUAL(dstr_vector_size(vec), 0);
    dstr_vector_push_front_decref(vec, dstr_with_initial("some more data"));
    CU_ASSERT_EQUAL(dstr_vector_size(vec), 1);
    for (i = 0; i < 10000; i++){
        dstr_vector_push_back_decref(vec, dstr_with_initial("some more data"));
    }
    CU_ASSERT_EQUAL(dstr_vector_size(vec), 10001);

    dstr_vector_decref(vec);
}

int main()
{
   CU_pSuite dstr_suite, dstr_list_suite, dstr_vector_suite;

   if (CU_initialize_registry() != CUE_SUCCESS)
      return CU_get_error();

   dstr_suite = CU_add_suite("dstr", 0,0);
   if (!dstr_suite){
      CU_cleanup_registry();
      return CU_get_error();
   }
   dstr_list_suite = CU_add_suite("dstr_list", 0,0);
   if (!dstr_list_suite){
      CU_cleanup_registry();
      return CU_get_error();
   }
   dstr_vector_suite = CU_add_suite("dstr_vector", 0,0);
   if (!dstr_list_suite){
      CU_cleanup_registry();
      return CU_get_error();
   }

   if (!CU_add_test(dstr_suite, "dstr_new", new_dstr_test) ||
           !CU_add_test(dstr_suite, "dstr_with_initial", new_dstr_initial_test) ||
           !CU_add_test(dstr_suite, "dstr_copy", new_dstr_from_dstr) ||
           !CU_add_test(dstr_suite, "dstr_prealloc", new_dstr_prealloc) ||
           !CU_add_test(dstr_suite, "dstr_decref", test_decref) ||
           !CU_add_test(dstr_suite, "dstr_incref", test_incref) ||
           !CU_add_test(dstr_suite, "dstr_append", test_dstr_append) ||
           !CU_add_test(dstr_suite, "dstr_append_decref", test_dstr_append_decref) ||
           !CU_add_test(dstr_suite, "dstr_append_cstr", test_dstr_append_cstr) ||
           !CU_add_test(dstr_suite, "dstr_clear", test_dstr_clear) ||
           !CU_add_test(dstr_suite, "dstr_compact", test_dstr_compact) ||
           !CU_add_test(dstr_suite, "dstr_growth_rate", test_dstr_growth_rate) ||
           !CU_add_test(dstr_suite, "dstr_dstr_to_cstr", test_dstr_to_cstr)){
      CU_cleanup_registry();
      return CU_get_error();
   }

   if (!CU_add_test(dstr_list_suite, "dstr_list_new", test_dstr_list_new) ||
           !CU_add_test(dstr_list_suite, "dstr_list_append", test_dstr_list_append) ||
           !CU_add_test(dstr_list_suite, "dstr_decref", test_dstr_list_decref) ||
           !CU_add_test(dstr_list_suite, "dstr_incref", test_dstr_list_incref) ||
           !CU_add_test(dstr_list_suite, "dstr_list_traverse", test_dstr_list_traverse) ||
           !CU_add_test(dstr_list_suite, "dstr_list_traverse_reverse", test_dstr_list_traverse_reverse) ||
           !CU_add_test(dstr_list_suite, "dstr_list_traverse_size", test_dstr_list_size) ||
           !CU_add_test(dstr_list_suite, "dstr_list_append_decref", test_dstr_list_append_decref)){
      CU_cleanup_registry();
      return CU_get_error();
   }

   if (!CU_add_test(dstr_vector_suite, "dstr_vector_new", test_dstr_vector_new) ||
           !CU_add_test(dstr_vector_suite, "dstr_vector_insert", test_dstr_vector_insert) ||
           !CU_add_test(dstr_vector_suite, "dstr_vector_push_front", test_dstr_vector_push_front) ||
           !CU_add_test(dstr_vector_suite, "dstr_vector_push_front_decref", test_dstr_vector_push_front_decref) ||
           !CU_add_test(dstr_vector_suite, "dstr_vector_push_back", test_dstr_vector_push_back) ||
           !CU_add_test(dstr_vector_suite, "dstr_vector_push_back_decref", test_dstr_vector_push_back_decref) ||
           !CU_add_test(dstr_vector_suite, "dstr_vector_back", test_dstr_vector_back) ||
           !CU_add_test(dstr_vector_suite, "dstr_vector_front", test_dstr_vector_front) ||
           !CU_add_test(dstr_vector_suite, "dstr_vector_is_empty", test_dstr_vector_is_empty) ||
           !CU_add_test(dstr_vector_suite, "dstr_vector_size", test_dstr_vector_size) ||
           !CU_add_test(dstr_vector_suite, "dstr_vector_remove", test_dstr_vector_remove)){
      CU_cleanup_registry();
      return CU_get_error();
   }

   CU_basic_set_mode(CU_BRM_VERBOSE);
   CU_basic_run_tests();
   CU_cleanup_registry();
   return CU_get_error();
}
