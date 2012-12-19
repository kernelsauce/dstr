/*  Reference counted dynamic string and string containers.
    Copyright 2012 John Abrahamsen <jhnabrhmsn@gmail.com>

    Permission is hereby granted, free of charge, to any person obtaining
    a copy of this software and associated documentation files (the
    "Software"), to deal in the Software without restriction, including
    without limitation the rights to use, copy, modify, merge, publish,
    distribute, sublicense, and/or sell copies of the Software, and to
    permit persons to whom the Software is furnished to do so, subject to
    the following conditions:

    The above copyright notice and this permission notice shall be
    included in all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
    EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
    MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
    NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
    LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
    OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
    WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#include <string.h>
#include <time.h>
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
    CU_ASSERT_EQUAL(str->mem, 100);
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

void test_dstr_copy_to_cstr()
{
    dstr *str = dstr_with_initial("something here");
    char *cstr = dstr_copy_to_cstr(str);
    CU_ASSERT_STRING_EQUAL(cstr, dstr_to_cstr_const(str));
    dstr_decref(str);
    free(cstr);
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

void test_dstr_sprintf()
{
    dstr *str = dstr_with_initial("I am this old: ");
    dstr_sprintf(str, "%d and in 2 years I am: %d", 30, 32);
    CU_ASSERT_STRING_EQUAL(dstr_to_cstr_const(str), "I am this old: 30 and in 2 years I am: 32");
    dstr_decref(str);
}

void test_dstr_prepend()
{
    dstr *str1 = dstr_with_initial("str1");
    dstr *str2 = dstr_with_initial("str2");
    dstr_prepend(str1, str2);
    CU_ASSERT_STRING_EQUAL(dstr_to_cstr_const(str1), "str2str1");
    dstr_decref(str1);
    dstr_decref(str2);
}

void test_dstr_prepend_cstr()
{
    dstr *str1 = dstr_with_initial("str1");
    dstr_prepend_cstr(str1, "str2");
    CU_ASSERT_STRING_EQUAL(dstr_to_cstr_const(str1), "str2str1");
    dstr_decref(str1);
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
    CU_ASSERT_EQUAL(str->mem, 10);
    dstr_clear(str);
    dstr_append_cstr(str, "hi!");
    dstr_compact(str);
    CU_ASSERT_EQUAL(str->mem, 4);
    dstr_decref(str);
}

void test_dstr_growth_rate()
{
    dstr *str = dstr_with_initial("data");
    int alloc_before = str->mem;

    dstr_growth_rate(str, 5);
    dstr_append_cstr(str, "data");
    CU_ASSERT_EQUAL(str->mem,
                    (alloc_before + (8 * sizeof(char))) * str->grow_r);
    dstr_decref(str);
}

void test_dstr_starts_with_dstr()
{
    dstr *match = dstr_with_initial("something to match");
    dstr *no_match = dstr_with_initial("not to match");

    dstr *match_word = dstr_with_initial("something");
    CU_ASSERT(dstr_starts_with_dstr(match, match_word));
    CU_ASSERT(!dstr_starts_with_dstr(no_match, match_word));
    dstr_decref(match);
    dstr_decref(no_match);
    dstr_decref(match_word);
}

void test_dstr_starts_with()
{
    dstr *match = dstr_with_initial("something to match");
    dstr *no_match = dstr_with_initial("not to match");

    CU_ASSERT(dstr_starts_with(match, "something"));
    CU_ASSERT(!dstr_starts_with(no_match, "something"));
    dstr_decref(match);
    dstr_decref(no_match);
}

void test_dstr_ends_with()
{
    dstr *match = dstr_with_initial("something to match");
    dstr *no_match = dstr_with_initial("match not");

    CU_ASSERT(dstr_ends_with(match, "match"));
    CU_ASSERT(!dstr_ends_with(no_match, "match"));
    dstr_decref(match);
    dstr_decref(no_match);
}

void test_dstr_ends_with_dstr()
{
    dstr *match = dstr_with_initial("something to match");
    dstr *no_match = dstr_with_initial("match not");
    dstr *to_match = dstr_with_initial("match");

    CU_ASSERT(dstr_ends_with_dstr(match, to_match));
    CU_ASSERT(!dstr_ends_with_dstr(no_match, to_match));
    dstr_decref(match);
    dstr_decref(no_match);
    dstr_decref(to_match);
}

void test_dstr_contains()
{
    dstr *str = dstr_with_initial("a big string with a small word in it to find");
    CU_ASSERT(dstr_contains(str, "small"));
    CU_ASSERT(!dstr_contains(str, "large"));
    dstr_decref(str);
}

void test_dstr_contains_dstr()
{
    dstr *str = dstr_with_initial("a big string with a small word in it to find");
    dstr *contains = dstr_with_initial("small");
    dstr *not_contains = dstr_with_initial("large");

    CU_ASSERT(dstr_contains_dstr(str, contains));
    CU_ASSERT(!dstr_contains_dstr(str, not_contains));
    dstr_decref(str);
    dstr_decref(contains);
    dstr_decref(not_contains);
}

void test_dstr_split_to_vector()
{
    dstr *str = dstr_with_initial("word1,word2,word3,word4,word5,word6");
    dstr_vector *vec = dstr_split_to_vector(str, ",");

    dstr_decref(str);
    CU_ASSERT_STRING_EQUAL(dstr_to_cstr_const(dstr_vector_at(vec, 0)), "word1");
    CU_ASSERT_STRING_EQUAL(dstr_to_cstr_const(dstr_vector_at(vec, 1)), "word2");
    CU_ASSERT_STRING_EQUAL(dstr_to_cstr_const(dstr_vector_at(vec, 2)), "word3");
    CU_ASSERT_STRING_EQUAL(dstr_to_cstr_const(dstr_vector_at(vec, 3)), "word4");
    CU_ASSERT_STRING_EQUAL(dstr_to_cstr_const(dstr_vector_at(vec, 4)), "word5");
    CU_ASSERT_STRING_EQUAL(dstr_to_cstr_const(dstr_vector_at(vec, 5)), "word6");

    dstr_vector_decref(vec);
}

void test_dstr_split_to_list()
{
    dstr *str = dstr_with_initial("word1,word2,word3,word4,word5,word6");
    dstr_list *list = dstr_split_to_list(str, ",");
    dstr *combined_again = dstr_list_to_dstr(",", list);

    CU_ASSERT_PTR_NOT_NULL_FATAL(combined_again);
    CU_ASSERT(dstr_starts_with_dstr(str, combined_again));

    dstr_decref(str);
    dstr_decref(combined_again);
    dstr_list_decref(list);
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

void test_dstr_vector_at()
{
    dstr_vector *vec = dstr_vector_new();
    dstr_vector_push_front_decref(vec, dstr_with_initial("some data"));
    dstr_vector_push_front_decref(vec, dstr_with_initial("some more data"));
    dstr_vector_push_front_decref(vec, dstr_with_initial("even more data"));

    CU_ASSERT_STRING_EQUAL(dstr_to_cstr_const(dstr_vector_at(vec, 0)), "even more data");
    CU_ASSERT_STRING_EQUAL(dstr_to_cstr_const(dstr_vector_at(vec, 1)), "some more data");
    CU_ASSERT_STRING_EQUAL(dstr_to_cstr_const(dstr_vector_at(vec, 2)), "some data");

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

void test_some_concat()
{
    dstr *str = dstr_with_prealloc(1000);
    clock_t start = clock(), diff;
    int i;

    for (i = 0; i < 1000000; i++){
        dstr_append_cstr(str, "concat me onto something...");
    }

    dstr_decref(str);
    diff = clock() - start;

    int msec = diff * 1000 / CLOCKS_PER_SEC;
    printf("time used for 1000000 appends: %d seconds %d milliseconds. ", msec/1000, msec%1000);
}

void __append_to_str_callback(dstr *src, void *dest)
{
    dstr_append(dest, src);
    if (!dstr_starts_with(src, "word6"))
        dstr_append_cstr(dest, ",");
}

void test_diverse_things()
{
    dstr_vector *vec1;
    dstr_list *list = dstr_list_new();
    dstr *str = dstr_with_initial("word1,word2,word3,word4,word5,word6");
    dstr *str2 = dstr_new();

    vec1 = dstr_split_to_vector(str, ",");
    while (!dstr_vector_is_empty(vec1)){
        dstr_list_add(list, dstr_vector_back(vec1));
        dstr_vector_pop_back(vec1);
    }

    dstr_list_traverse_reverse(list, __append_to_str_callback, str2);

    CU_ASSERT(dstr_starts_with_dstr(str, str2));
    dstr_decref(str);
    dstr_decref(str2);
    dstr_vector_decref(vec1);
    dstr_list_decref(list);
}

void test_vector_append_speed()
{
    dstr *str = dstr_with_initial("append me");
    dstr_vector *vec = dstr_vector_prealloc(1000000);
    clock_t start = clock(), diff;
    int i;

    for (i = 0; i < 1000000; i++){
        dstr_vector_push_back(vec, str);
    }

    dstr_vector_decref(vec);
    diff = clock() - start;
    dstr_decref(str);
    int msec = diff * 1000 / CLOCKS_PER_SEC;
    printf("time used for 1000000 push_back to vector: %d seconds %d milliseconds. ", msec/1000, msec%1000);
}

void test_vector_append_speed_no_prealloc()
{
    dstr *str = dstr_with_initial("append me");
    dstr_vector *vec = dstr_vector_new();
    clock_t start = clock(), diff;
    int i;

    for (i = 0; i < 1000000; i++){
        dstr_vector_push_back(vec, str);
    }

    dstr_vector_decref(vec);
    diff = clock() - start;
    dstr_decref(str);
    int msec = diff * 1000 / CLOCKS_PER_SEC;
    printf("time used for 1000000 push_back to vector: %d seconds %d milliseconds. ", msec/1000, msec%1000);
}

void test_vector_append_front_speed()
{
    dstr *str = dstr_with_initial("append me");
    dstr_vector *vec = dstr_vector_prealloc(20000);
    clock_t start = clock(), diff;
    int i;

    for (i = 0; i < 20000; i++){
        dstr_vector_push_front(vec, str);
    }

    dstr_vector_decref(vec);
    diff = clock() - start;
    dstr_decref(str);
    int msec = diff * 1000 / CLOCKS_PER_SEC;
    printf("time used for 20000 push_front to vector: %d seconds %d milliseconds. ", msec/1000, msec%1000);
}

void test_list_append_speed()
{
    dstr *str = dstr_with_initial("append me");
    dstr_list *list = dstr_list_new();
    clock_t start = clock(), diff;
    int i;

    for (i = 0; i < 1000000; i++){
        dstr_list_add(list, str);
    }

    dstr_list_decref(list);
    diff = clock() - start;
    dstr_decref(str);
    int msec = diff * 1000 / CLOCKS_PER_SEC;
    printf("time used for 1000000 insertion to list: %d seconds %d milliseconds. ", msec/1000, msec%1000);
}

int main()
{
   CU_pSuite dstr_suite, dstr_list_suite, dstr_vector_suite, typical;

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
   if (!dstr_vector_suite){
      CU_cleanup_registry();
      return CU_get_error();
   }
   typical = CU_add_suite("typical usage", 0,0);
   if (!typical){
      CU_cleanup_registry();
      return CU_get_error();
   }

   printf("\n========== sizes ==========\n");
   printf("size of size_t is: %lu bytes, %lu bits\n", sizeof(size_t), sizeof(size_t)*4);
   printf("size of dstr is: %lu bytes, %lu bits\n", sizeof(dstr), sizeof(dstr)*4);
   printf("size of dstr_vector is: %lu bytes, %lu bits\n", sizeof(dstr_vector), sizeof(dstr_vector)*4);
   printf("size of dstr_list is: %lu bytes, %lu bits\n", sizeof(dstr_list), sizeof(dstr_list)*4);

   if (!CU_add_test(dstr_suite, "dstr_new", new_dstr_test) ||
           !CU_add_test(dstr_suite, "dstr_with_initial", new_dstr_initial_test) ||
           !CU_add_test(dstr_suite, "dstr_copy", new_dstr_from_dstr) ||
           !CU_add_test(dstr_suite, "dstr_prealloc", new_dstr_prealloc) ||
           !CU_add_test(dstr_suite, "dstr_decref", test_decref) ||
           !CU_add_test(dstr_suite, "dstr_incref", test_incref) ||
           !CU_add_test(dstr_suite, "dstr_dstr_copy_to_cstr", test_dstr_copy_to_cstr) ||
           !CU_add_test(dstr_suite, "dstr_append", test_dstr_append) ||
           !CU_add_test(dstr_suite, "dstr_append_decref", test_dstr_append_decref) ||
           !CU_add_test(dstr_suite, "dstr_append_cstr", test_dstr_append_cstr) ||
           !CU_add_test(dstr_suite, "dstr_sprintf", test_dstr_sprintf) ||
           !CU_add_test(dstr_suite, "dstr_prepend", test_dstr_prepend) ||
           !CU_add_test(dstr_suite, "dstr_prepend_cstr", test_dstr_prepend_cstr) ||
           !CU_add_test(dstr_suite, "dstr_clear", test_dstr_clear) ||
           !CU_add_test(dstr_suite, "dstr_compact", test_dstr_compact) ||
           !CU_add_test(dstr_suite, "dstr_growth_rate", test_dstr_growth_rate) ||
           !CU_add_test(dstr_suite, "dstr_starts_with_dstr", test_dstr_starts_with_dstr) ||
           !CU_add_test(dstr_suite, "dstr_starts_with", test_dstr_starts_with) ||
           !CU_add_test(dstr_suite, "dstr_ends_with", test_dstr_ends_with) ||
           !CU_add_test(dstr_suite, "dstr_ends_with_dstr", test_dstr_ends_with_dstr) ||
           !CU_add_test(dstr_suite, "dstr_contains", test_dstr_contains) ||
           !CU_add_test(dstr_suite, "dstr_contains_dstr", test_dstr_contains_dstr) ||
           !CU_add_test(dstr_suite, "dstr_split_to_vector", test_dstr_split_to_vector) ||
           !CU_add_test(dstr_suite, "dstr_split_to_list", test_dstr_split_to_list) ||
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
           !CU_add_test(dstr_vector_suite, "dstr_vector_at", test_dstr_vector_at) ||
           !CU_add_test(dstr_vector_suite, "dstr_vector_remove", test_dstr_vector_remove)){
      CU_cleanup_registry();
      return CU_get_error();
   }


   if (!CU_add_test(typical, "test_some_concating", test_some_concat) ||
           !CU_add_test(typical, "test_vector_append_speed", test_vector_append_speed) ||
           !CU_add_test(typical, "test_vector_append_speed_no_prealloc", test_vector_append_speed_no_prealloc) ||
           !CU_add_test(typical, "test_vector_append_front_speed", test_vector_append_front_speed) ||
           !CU_add_test(typical, "test_vector_list_speed", test_list_append_speed) ||
           !CU_add_test(typical, "test_diverse_things", test_diverse_things)){
      CU_cleanup_registry();
      return CU_get_error();
   }

   CU_basic_set_mode(CU_BRM_VERBOSE);
   CU_basic_run_tests();
   CU_cleanup_registry();
   return CU_get_error();
}

