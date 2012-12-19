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
#include <malloc.h>

#include "dstr.h"

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
        memcpy(tmp_ptr, ptr, old_sz);
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

    more_mem = (str->mem + (sz * sizeof(char))) * str->grow_r;
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
    str->grow_r = DSTR_MEM_EXPAND_RATE;
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
    str->grow_r = DSTR_MEM_EXPAND_RATE;
    str->mem = (str->sz * sizeof(char)) + sizeof(char) ;
    str->data = strdup(initial);
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
    str->grow_r = DSTR_MEM_EXPAND_RATE;
    str->data = strndup(initial, n);
    if (!str->data)
        return 0;
    str->mem = (str->sz * sizeof(char)) + sizeof(char);
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
    str->grow_r = DSTR_MEM_EXPAND_RATE;
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
    return strndup(str->data, str->sz + 1);
}

int dstr_compact(dstr *str)
{
    int alloc;
    void *tmp_ptr;

    if (str->mem > str->sz){
        alloc = (sizeof(char) * str->sz) + sizeof(char);
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
    if (strstr(haystack->data, needle))
        return 1;
    return 0;
}

int dstr_contains_dstr(const dstr *haystack, const dstr *needle)
{
    if (strstr(haystack->data, needle->data))
        return 1;
    return 0;
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
            if (!dstr_ptr)
                return 0;
            if (!dstr_vector_push_back_decref(vec, dstr_ptr))
                return 0;
            return vec;
        } else {
            occ_len = occ_end - occ_start;
            dstr_ptr = dstr_with_initialn(occ_start, occ_len);
            if (!dstr_ptr)
                return 0;
            if (!dstr_vector_push_back_decref(vec, dstr_ptr))
                return 0;
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
    cstr = dstr_to_cstr_const(str);
    for (;;){
        occ_end = strstr(occ_start, sep);
        if (!occ_end){
            occ_len = strlen(occ_start);
            dstr_ptr = dstr_with_initialn(occ_start, occ_len);
            if (!dstr_ptr)
                return 0;
            if (!dstr_list_add_decref(list, dstr_ptr))
                return 0;
            return list;
        } else {
            occ_len = occ_end - occ_start;
            dstr_ptr = dstr_with_initialn(occ_start, occ_len);
            if (!dstr_ptr)
                return 0;
            if (!dstr_list_add_decref(list, dstr_ptr))
                return 0;
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

int dstr_append(dstr* dest, const dstr* src)
{
    size_t total = src->sz + dest->sz;
    if (!__dstr_can_hold(dest, total + 1)){
        if (!__dstr_alloc(dest, total))
            return 0;
    }
    strcpy(dest->data+dest->sz, src->data);
    dest->sz = total;
    return 1;
}

int dstr_append_cstr(dstr* dest, const char *src)
{
    size_t total = strlen(src) + dest->sz;
    if (!__dstr_can_hold(dest, total + 1)){
        if (!__dstr_alloc(dest, total))
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
    strncpy(dest->data+dest->sz, src, n);
    dest->sz = total;
    return 1;
}

int dstr_sprintf(dstr *str, const char *fmt, ...)
{
    char *tmp;
    int rc;

    va_list ap;
    va_start(ap, fmt);
    vasprintf(&tmp, fmt, ap);
    va_end(ap);
    if (!tmp)
        return 0;
    rc = dstr_append_cstr(str, tmp);
    free(tmp);
    return rc;
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
        if (!__dstr_alloc(dest, total))
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
        if (!__dstr_alloc(dest, total))
            return 0;
    }
    if (!memmove(dest->data + src_len, dest->data,
            (src_len * sizeof(char)) + sizeof(char)))
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
        if (!__dstr_alloc(dest, total))
            return 0;
    }
    if (!memmove(dest->data + n, dest->data,
            (n * sizeof(char)) + sizeof(char)))
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

void dstr_growth_rate(dstr *dest, int rate)
{
    dest->grow_r = rate;
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
    dstr_link_t *link;

    link = calloc(1, sizeof(dstr_link_t));
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

void dstr_list_remove(dstr_list *list, dstr_link_t *link)
{
    dstr_link_t * prev;
    dstr_link_t * next;

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
    dstr_link_t *link;
    size_t sz = 0;
    for (link = list->head; link; link = link->next){
        sz++;
    }
    return sz;
}

void dstr_list_traverse(dstr_list * list,
                        void (*callback)(dstr *, void *),
                        void *user_data)
{
    dstr_link_t *link;

    for (link = list->head; link; link = link->next){
        callback((void *) link->str, user_data);
    }
}

void dstr_list_traverse_reverse (dstr_list *list,
                                 void (*callback)(dstr *, void *),
                                 void *userdata)
{
    dstr_link_t * link;

    for (link = list->tail; link; link = link->prev) {
        callback ((void *) link->str, userdata);
    }
}

void dstr_list_traverse_delete (dstr_list * list, int (*callback)(dstr *))
{
    dstr_link_t *link;

    for (link = list->head; link; link = link->next) {
        if (callback((void *) link->str)) {
            dstr_list_remove(list, link);
        }
    }
}

void dstr_list_decref (dstr_list *list)
{
    dstr_link_t *link;
    dstr_link_t *next;

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
    dstr_link_t *link;

    if (sep){
        for (link = list->head; link; link = link->next){
            dstr_append(str, link->str);
            if (link->next)
                dstr_append_cstr(str, sep);
        }
    } else {
        for (link = list->head; link; link = link->next){
            dstr_append(str, link->str);
        }
    }

    return str;
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
    int i;
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
    int alloc = elements * sizeof(dstr *) * DSTR_VECTOR_MEM_EXPAND_RATE;
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
    int new_sz = vec->sz + 1, move_n, move_ptr;
#ifdef DSTR_MEM_SECURITY
    if (vec->sz < pos && pos != DSTR_VECTOR_END)
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
        move_n = vec->sz - pos;
        move_ptr = vec->sz;
        while(move_n){
            vec->arr[move_ptr] = vec->arr[move_ptr-1];
            move_ptr--;
            move_n--;
        }

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
    dstr *str = vec->arr[vec->sz - 1];
    return str;
}

dstr *dstr_vector_front(dstr_vector *vec)
{
    dstr *str = vec->arr[DSTR_VECTOR_BEGIN];
    return str;
}

dstr *dstr_vector_at(dstr_vector *vec, int pos)
{
#ifdef DSTR_MEM_SECURITY
    if (vec->sz - 1 < pos && pos != DSTR_VECTOR_END)
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
    size_t x, sz = vec->sz - 1;
#ifdef DSTR_MEM_SECURITY
    if (vec->sz - 1 < pos && pos != DSTR_VECTOR_END)
        return 0;
#endif

    if (pos == DSTR_VECTOR_END){
        dstr_decref(vec->arr[sz]);
        vec->arr[sz] = 0;
        vec->sz--;
        return 1;
    } else {
        dstr_decref(vec->arr[pos]);
        for (x = pos; x != sz; x++){
            vec->arr[x] = vec->arr[x + 1];
        }
        vec->sz--;
        return 1;
    }
    return 0;
}
