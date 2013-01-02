/*  Reference counted dynamic string and string list.
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

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include <malloc.h>

#include "dstr.h"

dstr *dstr_version()
{
    dstr *ver = dstr_with_prealloc(4);
    if (!ver)
        return 0;
#ifdef DSTR_LESSER_VERSION
    if (!dstr_sprintf(ver, "%d.%d.%d", DSTR_MAJOR_VERSION,
                 DSTR_MINOR_VERSION,
                 DSTR_LESSER_VERSION)){
        dstr_decref(ver);
        return 0;
    }
#else
    if (!dstr_sprintf(ver, "%d.%d", DSTR_MAJOR_VERSION, DSTR_MINOR_VERSION)){
        dstr_decref(ver);
        return 0;
    }
#endif
    return ver;
}

/*                            DYNAMIC STRING                                 */

#ifdef DSTR_MEM_CLEAR
/* Memset that will not be optimized away by compilers.   */
static void __dstr_safe_memset(void *ptr, int c, size_t sz)
{
    volatile unsigned char *p = ptr;

    for (; sz > 0; sz--)
        *p++ = (unsigned char)c;
}

/* Realloc that will zero byte the old allocated space.    */
static void *__dstr_safe_realloc(void *ptr, size_t new_sz, size_t old_sz)
{
    void * tmp_ptr;

    tmp_ptr = malloc(new_sz);
    if (!tmp_ptr)
        return 0;
    if (ptr){
        if (old_sz <= new_sz) // increase buffer case
            memcpy(tmp_ptr, ptr, old_sz);
        else if (new_sz < old_sz)
            memcpy(tmp_ptr, ptr, new_sz); // decrease buffer case
        __dstr_safe_memset(ptr, 0, old_sz);
        free(ptr);
    }
    return tmp_ptr;
}
#endif

/* Allocate memory for dstr. Uses realloc if not DSTR_MEM_CLEAR is defined,
   else it uses __dstr_safe_realloc.   */
static int __dstr_alloc(dstr* str, size_t sz)
{
    size_t more_mem;
    void *tmp_ptr;

    more_mem = (str->mem + (sz * sizeof(char))) * DSTR_MEM_EXPAND_RATE;
#ifdef DSTR_MEM_CLEAR
    tmp_ptr = __dstr_safe_realloc(str->data, more_mem, str->mem);
#else
    tmp_ptr = realloc(str->data, more_mem);
#endif
    if (tmp_ptr)
        str->data = tmp_ptr;
    else
        return 0;
    str->mem = more_mem;
    return 1;
}

/* Check if a dstr can hold n bytes.   */
static int __dstr_can_hold(const dstr *str, size_t sz)
{
    if (!str->data)
        return 0;
    else
        return (sz < str->mem);
}

/* strdup implementation for those who are not so fortunate.   */
static char *__dstr_strdup(const char* str)
{
    char *cpy;
    size_t len = strlen(str);

    cpy = malloc(len + 1);
    if (!cpy)
        return 0;
    cpy[len] = '\0';
    return memcpy(cpy, str, len);
}

/* strndup implementation.   */
static char *__dstr_strndup(const char *str, size_t sz)
{
    char *cpy;
    size_t len = strlen(str);

    if (sz < len)
        len = sz;
    cpy = malloc(len + 1);
    if (!cpy)
        return 0;
    cpy[len] = '\0';
    return memcpy(cpy, str, len);
}

void dstr_decref(dstr *str)
{
    str->ref--;
    if (!str->ref){
#ifdef DSTR_MEM_CLEAR
        __dstr_safe_memset(str->data, 0, str->mem);
        free(str->data);
        __dstr_safe_memset(str, 0, sizeof(dstr));
        free(str);
#else
        free(str->data);
        free(str);
#endif
    }
}

const char *dstr_to_cstr_const(const dstr* str)
{
    return str->data;
}

dstr *dstr_new()
{
    dstr *str = malloc(sizeof(dstr));

    if (!str)
        return 0;
    str->sz = 0;
    str->data = 0;
    str->mem = 0;
    str->ref = 1;
    return str;
}

dstr *dstr_with_initial(const char *initial)
{
    dstr *str = malloc(sizeof(dstr));

    if (!str)
        return 0;
    str->sz = strlen(initial);
    str->mem = (str->sz + 1) * sizeof(char);
    str->data = __dstr_strdup(initial);
    if (!str->data)
        return 0;
    str->ref = 1;
    return str;
}

dstr *dstr_with_initialn(const char *initial, size_t n)
{
    dstr *str = malloc(sizeof(dstr));

    if (!str)
        return 0;
    str->sz = n;
    str->data = __dstr_strndup(initial, n);
    if (!str->data)
        return 0;
    str->mem = (str->sz + 1) * sizeof(char);
    str->ref = 1;
    return str;
}

dstr *dstr_with_prealloc(size_t sz)
{
    dstr *str = malloc(sizeof(dstr));
    size_t pre_alloc_mem = sizeof(char) * sz;

    if (!str)
        return 0;
    str->sz = 0;
    str->data = malloc(pre_alloc_mem);
    if (!str->data)
        return 0;
    str->mem = pre_alloc_mem;
    str->data[0] = '\0';
    str->ref = 1;
    return str;
}

char *dstr_copy_to_cstr(const dstr* str)
{
    return __dstr_strndup(str->data, str->sz + 1);
}

int dstr_compact(dstr *str)
{
    int alloc;
    void *tmp_ptr;

    if (str->mem > str->sz){
        alloc = (sizeof(char) * str->sz + sizeof(char)) ;
#ifdef DSTR_MEM_CLEAR
        tmp_ptr = __dstr_safe_realloc(str->data, alloc, str->mem);
#else
        tmp_ptr = realloc(str->data, alloc);
#endif
        if (tmp_ptr)
            str->data = tmp_ptr;
        else
            return 0;
        str->mem = alloc;
        if (str->data)
            return 1;
    }
    return 0;
}

int dstr_contains(const dstr *haystack, const char *needle)
{
    const char* ptr = haystack->data;
    size_t n = 0;

    while ((ptr = strstr(ptr, needle))){
        n++;
        ptr++;
    }
    return n;
}

int dstr_contains_dstr(const dstr *haystack, const dstr *needle)
{
    return dstr_contains(haystack, dstr_to_cstr_const(needle));
}

int dstr_starts_with_dstr(const dstr *str, const dstr *starts_with)
{
    return dstr_starts_with(str, dstr_to_cstr_const(starts_with));
}

dstr_vector *dstr_split_to_vector(const dstr *str, const char *sep)
{
    dstr_vector *vec;
    dstr *dstr_ptr;
    size_t count, sep_len, occ_len;
    const char *cstr, *occ_start, *occ_end;

    cstr = dstr_to_cstr_const(str);
    occ_start = cstr;
    count = 0;
    sep_len = strlen(sep);

    while (*cstr != '\0'){
        if (strncmp(cstr++, sep, sep_len))
            continue;
        count++;
    }

    vec = dstr_vector_prealloc(count);
    cstr = dstr_to_cstr_const(str);
    for (;;){
        occ_end = strstr(occ_start, sep);
        if (!occ_end){
            occ_len = strlen(occ_start);
            dstr_ptr = dstr_with_initialn(occ_start, occ_len);
            if (!dstr_ptr || !dstr_vector_push_back_decref(vec, dstr_ptr)){
                dstr_vector_decref(vec);
                return 0;
            }
            return vec;
        } else {
            occ_len = occ_end - occ_start;
            dstr_ptr = dstr_with_initialn(occ_start, occ_len);
            if (!dstr_ptr || !dstr_vector_push_back_decref(vec, dstr_ptr)){
                dstr_vector_decref(vec);
                return 0;
            }
            occ_start = occ_end + 1;
        }
    }

    return vec;
}

dstr_list *dstr_split_to_list(const dstr *str, const char *sep)
{
    dstr_list *list;
    dstr *dstr_ptr;
    size_t occ_len;
    const char *cstr, *occ_start, *occ_end;

    cstr = dstr_to_cstr_const(str);
    occ_start = cstr;

    list = dstr_list_new();
    if (!list)
        return 0;
    cstr = dstr_to_cstr_const(str);
    for (;;){
        occ_end = strstr(occ_start, sep);
        if (!occ_end){
            occ_len = strlen(occ_start);
            dstr_ptr = dstr_with_initialn(occ_start, occ_len);
            if (!dstr_ptr || !dstr_list_add_decref(list, dstr_ptr)){
                dstr_list_decref(list);
                return 0;
            }
            return list;
        } else {
            occ_len = occ_end - occ_start;
            dstr_ptr = dstr_with_initialn(occ_start, occ_len);
            if (!dstr_ptr || !dstr_list_add_decref(list, dstr_ptr)){
                dstr_list_decref(list);
                return 0;
            }
            occ_start = occ_end + 1;
        }
    }

    return list;
}

int dstr_starts_with(const dstr *str, const char *starts_with)
{
    const char *cstr;
    int cstr_len, start_len;

    cstr = dstr_to_cstr_const(str);
    cstr_len = strlen(cstr);
    start_len = strlen(starts_with);

    if (start_len > cstr_len)
        return 0;
    while (start_len--){
        if (cstr[start_len] != starts_with[start_len])
            return 0;
    }
    return 1;
}

int dstr_ends_with(const dstr *str, const char *ends_with)
{
    const char *cstr;
    int cstr_len, ends_len;

    cstr = dstr_to_cstr_const(str);
    cstr_len = strlen(cstr);
    ends_len = strlen(ends_with);

    if (ends_len > cstr_len)
        return 0;
    while (ends_len){
        if (cstr[cstr_len] != ends_with[ends_len--]){
            return 0;
        }
        cstr_len--;
    }
    return 1;
}

int dstr_ends_with_dstr(const dstr *str, dstr *ends_with)
{
    return dstr_ends_with(str, dstr_to_cstr_const(ends_with));
}

int dstr_matches(const dstr *haystack, const char *needle)
{
    int rc = strcmp(dstr_to_cstr_const(haystack), needle);
    if (!rc)
        return 1;
    return 0;
}

size_t dstr_len(const dstr *str)
{
    return str->sz;
}

int dstr_append(dstr* dest, const dstr* src)
{
    size_t total = src->sz + dest->sz;
    if (!__dstr_can_hold(dest, total + 1)){
        if (!__dstr_alloc(dest, total + 1))
            return 0;
    }
    memcpy(dest->data+dest->sz, src->data, src->sz + 1);
    dest->sz = total;
    return 1;
}

int dstr_append_cstr(dstr* dest, const char *src)
{
    size_t total = strlen(src) + dest->sz;
    if (!__dstr_can_hold(dest, total + 1)){
        if (!__dstr_alloc(dest, total + 1))
            return 0;
    }
    strcpy(dest->data+dest->sz, src);
    dest->sz = total;
    return 1;
}

int dstr_append_cstrn(dstr* dest, const char *src, size_t n)
{
    size_t total = n + dest->sz;
    if (!__dstr_can_hold(dest, total + 1)){
        if (!__dstr_alloc(dest, total + 1))
            return 0;
    }
    memcpy(dest->data+dest->sz, src, n);
    dest->sz = total;
    return 1;
}

int dstr_sprintf(dstr *str, const char *fmt, ...)
{
    int len, new_sz;
    size_t space = str->mem - (str->sz + 1);
    char *start_ptr = str->data + str->sz;
    va_list ap, ap_c;

    va_copy(ap_c, ap);
    va_start(ap, fmt);
    len = vsnprintf(start_ptr, space, fmt, ap);
    va_end(ap);
    new_sz = str->sz + len + 1;
    if (space <= len){
        if (!__dstr_alloc(str, new_sz))
            return 0;
        start_ptr = str->data + str->sz;
        va_start(ap_c, fmt);
        vsnprintf(start_ptr, new_sz, fmt, ap_c);
        va_end(ap_c);
    }
    str->sz = new_sz - 1;
    return 1;
}

void dstr_to_upper(dstr *str)
{
    int sz = str->sz;
    while(sz--){
        str->data[sz] = toupper(str->data[sz]);
    }
}

void dstr_to_lower(dstr *str)
{
    int sz = str->sz;
    while(sz--){
        str->data[sz] = tolower(str->data[sz]);
    }
}

void dstr_capitalize(dstr *str)
{
    if (!str->sz)
        return;
    str->data[0] = toupper(str->data[0]);
}

int dstr_append_decref(dstr* dest, dstr* src)
{
    if (dstr_append(dest, src)){
        dstr_decref(src);
        return 1;
    }
    return 0;
}

int dstr_prepend(dstr* dest, const dstr *src)
{
    size_t total = src->sz + dest->sz;
    if (!__dstr_can_hold(dest, total + 1)){
        if (!__dstr_alloc(dest, total + 1))
            return 0;
    }
    if (!memmove(dest->data + src->sz, dest->data,
            (src->sz * sizeof(char)) + sizeof(char)))
        return 0;
    if (!memcpy(dest->data, src->data, src->sz))
        return 0;
    dest->sz = total;
    return 1;
}

int dstr_prepend_decref(dstr* dest, dstr* src)
{
    if (dstr_prepend(dest, src)){
        dstr_decref(src);
        return 1;
    }
    return 0;
}

int dstr_prepend_cstr(dstr* dest, const char *src)
{
    size_t src_len = strlen(src);
    size_t total = src_len + dest->sz;
    if (!__dstr_can_hold(dest, total + 1)){
        if (!__dstr_alloc(dest, total + 1))
            return 0;
    }
    if (!memmove(dest->data + src_len, dest->data,
            (src_len + 1) * sizeof(char)))
        return 0;
    if (!memcpy(dest->data, src, src_len))
        return 0;
    dest->sz = total;
    return 1;
}

int dstr_prepend_cstrn(dstr* dest, const char *src, size_t n)
{
    size_t total = n + dest->sz;
    if (!__dstr_can_hold(dest, total + 1)){
        if (!__dstr_alloc(dest, total + 1))
            return 0;
    }
    if (!memmove(dest->data + n, dest->data,
            (n + 1 * sizeof(char))))
        return 0;
    if (!memcpy(dest->data, src, n))
        return 0;
    dest->sz = total;
    return 1;
}

dstr *dstr_copy(const dstr *copy)
{
    int rc;
    dstr* str = dstr_with_prealloc(copy->sz + 1);
    if (!str)
        return 0;
    rc = dstr_append(str, copy);
    if (!rc)
        return 0;
    return str;
}

void dstr_clear(dstr *str)
{
    int i = str->mem;
    while (i){
        i--;
        str->data[i] = 0;
    }
    str->sz = 0;
}

int dstr_print(const dstr *src)
{
    return printf("%s", src->data);
}



/*                          DYNAMIC STRING LIST                             */

dstr_list *dstr_list_new()
{
    dstr_list *list = malloc(sizeof(dstr_list));
    if (!list)
        return 0;
    list->head = 0;
    list->tail = 0;
    list->ref = 1;
    return list;
}

int dstr_list_add(dstr_list *list, dstr *str)
{
    dstr_link *link;

    link = calloc(1, sizeof(dstr_link));
    if (!link)
        return 0;

    link->str = str;
    dstr_incref(str);

    if (list->tail){
        list->tail->next = link;
        link->prev = list->tail;
        list->tail = link;
    } else {
        list->head = link;
        list->tail = link;
    }
    return 1;
}

int dstr_list_add_decref(dstr_list *dest, dstr *str)
{
    int rc = dstr_list_add(dest, str);
    if (rc)
        dstr_decref(str);
    return rc;
}

void dstr_list_remove(dstr_list *list, dstr_link *link)
{
    dstr_link * prev;
    dstr_link * next;

    prev = link->prev;
    next = link->next;
    if (prev){
        if (next){
            prev->next = next;
            next->prev = prev;
        } else {
            prev->next = 0;
            list->tail = prev;
        }
    } else {
        if (next){
            next->prev = 0;
            list->head = next;
        } else {
            list->head = 0;
            list->tail = 0;
        }
    }

    dstr_decref(link->str);
    free(link);
}

size_t dstr_list_size(const dstr_list *list)
{
    dstr_link *link;
    size_t sz = 0;
    DSTR_LIST_FOREACH(list, link){
        sz++;
    }
    return sz;
}

void dstr_list_traverse(dstr_list * list,
                        void (*callback)(dstr *, void *),
                        void *user_data)
{
    dstr_link *link;

    for (link = list->head; link; link = link->next){
        callback((void *) link->str, user_data);
    }
}

void dstr_list_traverse_reverse (dstr_list *list,
                                 void (*callback)(dstr *, void *),
                                 void *userdata)
{
    dstr_link * link;

    for (link = list->tail; link; link = link->prev) {
        callback ((void *) link->str, userdata);
    }
}

void dstr_list_traverse_delete (dstr_list * list, int (*callback)(dstr *))
{
    dstr_link *link;

    for (link = list->head; link; link = link->next) {
        if (callback((void *) link->str)) {
            dstr_list_remove(list, link);
        }
    }
}

void dstr_list_decref (dstr_list *list)
{
    dstr_link *link;
    dstr_link *next;

    list->ref--;
    if (!list->ref){
        for (link = list->head; link; link = next){
            next = link->next;
            dstr_decref(link->str);
            free(link);
        }
        free(list);
    }
}

dstr *dstr_list_to_dstr(const char *sep, dstr_list *list)
{
    dstr *str = dstr_new();
    dstr_link *link;

    if (!str)
        return 0;

    if (sep){
        DSTR_LIST_FOREACH(list, link){
            if (!dstr_append(str, link->str)){
                dstr_decref(str);
                return 0;
            }
            if (link->next)
                dstr_append_cstr(str, sep);
        }
    } else {
        DSTR_LIST_FOREACH(list, link){
            if (!dstr_append(str, link->str)){
                dstr_decref(str);
                return 0;
            }
        }
    }

    return str;
}

dstr_list *dstr_list_search_contains(dstr_list *search, const char * substr)
{
    dstr_list *found = dstr_list_new();
    dstr_link *link;

    if (!found)
        return 0;

    DSTR_LIST_FOREACH(search, link){
        if (dstr_contains(link->str, substr)){
            if (!dstr_list_add(found, link->str)){
                dstr_list_decref(found);
                return 0;
            }
        }
    }

    return found;
}

dstr_list *dstr_list_search_contains_dstr(dstr_list *search, const dstr *substr)
{
    return dstr_list_search_contains(search, dstr_to_cstr_const(substr));
}

dstr_list *dstr_list_bdecode(const char *str)
{
    size_t str_sz;
    dstr_list *list;

    if (str[0] != 'l') // Formatting might be sane.
        return 0;
    list = dstr_list_new();
    if (!list)
        return 0;

    str++;
    for (;;){
        str_sz = 0;
        while (isdigit(*str)){
            str_sz = str_sz * 10 + (*str - '0');
            str++;
        }
        str++; // Consume 'l'
        if (!dstr_list_add_decref(list, dstr_with_initialn(str, str_sz))){
            dstr_list_decref(list);
            return 0;
        }
        str += str_sz;
        if (*str == 'e')
            break; // End of list.
    }
    return list;

}

dstr *dstr_list_bencode(const dstr_list *list)
{
    dstr *byte_arr = dstr_with_initial("l");
    dstr_link *link;

    DSTR_LIST_FOREACH(list, link){
        if (!dstr_sprintf(byte_arr, "%d:", dstr_len(link->str)) ||
                !dstr_append(byte_arr, link->str)){
            dstr_decref(byte_arr);
            return 0;
        }
    }
    if (!dstr_append_cstr(byte_arr, "e")){
        dstr_decref(byte_arr);
        return 0;
    }
    return byte_arr;
}



/*                          DYNAMIC STRING VECTOR                           */

dstr_vector *dstr_vector_new()
{
    dstr_vector *vec = malloc(sizeof(dstr_vector));
    if (!vec)
        return 0;
    vec->ref = 1;
    vec->space = 0;
    vec->arr = 0;
    vec->sz = 0;
    return vec;
}

dstr_vector *dstr_vector_prealloc(size_t elements)
{
    dstr_vector *vec = malloc(sizeof(dstr_vector));
    if (!vec)
        return 0;
    vec->ref = 1;
    vec->arr = malloc(elements * sizeof(dstr*));
    if (!vec->arr){
        free(vec);
        return 0;
    }
    vec->space = elements;
    vec->sz = 0;
    return vec;
}

void dstr_vector_decref(dstr_vector *vec)
{
    size_t i;
    vec->ref--;
    if (!vec->ref){
        for (i = 0; i < vec->sz; i++){
            dstr_decref(vec->arr[i]);
        }
        free(vec->arr);
        free(vec);
    }
}

static int __dstr_vector_alloc(dstr_vector *vec, unsigned int elements)
{
    size_t alloc = elements * sizeof(dstr *) * DSTR_VECTOR_MEM_EXPAND_RATE;
    vec->arr = (dstr **)realloc(vec->arr, alloc);
    if (!vec->arr)
        return 0;
    vec->space = elements * DSTR_VECTOR_MEM_EXPAND_RATE;
    return 1;
}

static int __dstr_vector_can_hold(const dstr_vector *vec, unsigned int elements)
{
    if (vec->space >= elements)
        return 1;
    return 0;
}

int dstr_vector_insert(dstr_vector *vec, size_t pos, dstr *str)
{
    size_t new_sz = vec->sz + 1;

#ifdef DSTR_MEM_SECURITY
    if (pos != DSTR_VECTOR_END && vec->sz - 1 < pos)
        return 0;
#endif

    if (!__dstr_vector_can_hold(vec, new_sz)){
        if (!__dstr_vector_alloc(vec, new_sz))
            return 0;
    }
    if (pos == DSTR_VECTOR_END){
        vec->arr[vec->sz] = str;
        vec->sz++;
        dstr_incref(str);
        return 1;
    } else {
        memmove(vec->arr + pos + 1,
                vec->arr + pos,
                sizeof(dstr*) * (vec->sz - pos));
        vec->arr[pos] = str;
        vec->sz++;
        dstr_incref(str);
        return 1;
    }
    return 0;
}

int dstr_vector_insert_decref(dstr_vector *vec, size_t pos, dstr *str)
{
    int rc = dstr_vector_insert(vec, pos, str);
    if (rc)
        dstr_decref(str);
    return rc;
}

int dstr_vector_push_front(dstr_vector *vec, dstr *str)
{
    return dstr_vector_insert(vec, DSTR_VECTOR_BEGIN, str);
}

int dstr_vector_push_front_decref(dstr_vector *vec, dstr *str)
{
    return dstr_vector_insert_decref(vec, DSTR_VECTOR_BEGIN, str);
}

int dstr_vector_push_back(dstr_vector *vec, dstr *str)
{
    return dstr_vector_insert(vec, DSTR_VECTOR_END, str);
}

int dstr_vector_push_back_decref(dstr_vector *vec, dstr *str)
{
    return dstr_vector_insert_decref(vec, DSTR_VECTOR_END, str);
}

void dstr_vector_pop_back(dstr_vector *vec)
{
    dstr_vector_remove(vec, DSTR_VECTOR_END);
}

void dstr_vector_pop_front(dstr_vector *vec)
{
    dstr_vector_remove(vec, DSTR_VECTOR_BEGIN);
}

dstr *dstr_vector_back(dstr_vector *vec)
{
#ifdef DSTR_MEM_SECURITY
    if (!vec->sz)
        return 0;
#endif
    return vec->arr[vec->sz - 1];
}

dstr *dstr_vector_front(dstr_vector *vec)
{
#ifdef DSTR_MEM_SECURITY
    if (!vec->sz)
        return 0;
#endif
    return vec->arr[DSTR_VECTOR_BEGIN];
}

dstr *dstr_vector_at(dstr_vector *vec, size_t pos)
{
#ifdef DSTR_MEM_SECURITY
    if (!vec->sz || (pos != DSTR_VECTOR_END && vec->sz - 1 < pos))
        return 0;
#endif
    return vec->arr[pos];
}

int dstr_vector_is_empty(const dstr_vector *vec)
{
    return !vec->sz;
}

size_t dstr_vector_size(const dstr_vector *vec)
{
    return vec->sz;
}

int dstr_vector_remove(dstr_vector *vec, size_t pos)
{
    size_t sz = vec->sz - 1;
#ifdef DSTR_MEM_SECURITY
    if (!vec->sz || (pos != DSTR_VECTOR_END && vec->sz - 1 < pos))
        return 0;
#endif

    if (pos == DSTR_VECTOR_END){
        dstr_decref(vec->arr[sz]);
        vec->arr[sz] = 0;
        vec->sz--;
        return 1;
    } else {
        dstr_decref(vec->arr[pos]);
        memmove(vec->arr + pos,
                vec->arr + pos + 1,
                sizeof(dstr*) * (vec->sz - pos - 1));
        vec->sz--;
        return 1;
    }
    return 0;
}
