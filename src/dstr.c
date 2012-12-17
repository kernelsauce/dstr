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
#include <string.h>
#include <stdlib.h>
#include <malloc.h>

#include "dstr.h"

/****************************** DYNAMIC STRING ********************************/

static int alloc_more(dstr* str, int sz)
{
    int more_mem;

    more_mem = (int)(str->mem + (sz * sizeof(char))) * str->grow_r;
    str->mem = more_mem;
    str->data = realloc(str->data, more_mem);
    if (!str->data)
        return 0;
    return 1;
}

static int can_hold(const dstr *str, int chars)
{
    if (!str->data)
        return 0;
    else
        return (chars < str->mem);
}

void dstr_decref(dstr *str)
{
    str->ref--;
    if (!str->ref){
        free(str->data);
        free(str);
    }
}

void dstr_incref(dstr *str)
{
    str->ref++;
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
    str->grow_r = 2;
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
    str->grow_r = 2;
    str->mem = str->sz + 1;
    str->data = strdup(initial);
    if (!str->data)
        return 0;
    str->ref = 1;
    return str;
}

dstr *dstr_with_prealloc(unsigned int sz)
{
    dstr *str = malloc(sizeof(dstr));
    int pre_alloc_mem = sizeof(char) * sz;

    if (!str)
        return 0;
    str->sz = 0;
    str->grow_r = 2;
    str->mem = pre_alloc_mem;
    str->data = malloc(pre_alloc_mem);
    if (!str->data)
        return 0;
    str->data[0] = '\0';
    str->ref = 1;
    return str;
}

int dstr_compact(dstr *str)
{
    int alloc;
    if (str->mem > str->sz){
        alloc = str->sz + 1;
        str->data = realloc(str->data, str->sz + 1);
        str->mem = alloc;
        if (str->data)
            return 1;
    }
    return 0;
}

int dstr_append(dstr* dest, const dstr* src)
{
    int total = src->sz + dest->sz;
    if (!can_hold(dest, total + 1)){
        if (!alloc_more(dest, total))
            return 0;
    }
    strcpy(dest->data+dest->sz, src->data);
    dest->sz = total;
    return 1;
}

int dstr_append_cstr(dstr* dest, const char *src)
{
    int total = strlen(src) + dest->sz;
    if (!can_hold(dest, total + 1)){
        if (!alloc_more(dest, total))
            return 0;
    }
    strcpy(dest->data+dest->sz, src);
    dest->sz = total;
    return 1;
}

int dstr_append_decref(dstr* dest, dstr* src)
{
    if (dstr_append(dest, src)){
        dstr_decref(src);
        return 1;
    } else {
        return 0;
    }
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
    while (str->sz){
        str->sz--;
        str->data[str->sz] = 0;
    }
}

int dstr_print(const dstr *src)
{
    return printf("%s", src->data);
}

void dstr_growth_rate(dstr *dest, int rate)
{
    dest->grow_r = rate;
}

/**************************** DYNAMIC STRING LIST  ****************************/

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

    link = calloc (1, sizeof (dstr_link_t));
    if (!link)
        return 0;

    link->str = str;
    dstr_incref(str);

    if (list->tail) {
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
        if (next) {
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

int dstr_list_size(const dstr_list *list)
{
    dstr_link_t *link;
    int sz = 0;
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

void dstr_list_incref (dstr_list *list)
{
    list->ref++;
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

/************************* DYNAMIC VECTOR LIST  *******************************/

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

dstr_vector *dstr_vector_prealloc(unsigned int elements)
{
    dstr_vector *vec = malloc(sizeof(dstr_vector));
    if (!vec)
        return 0;
    vec->ref = 1;
    vec->arr = malloc(sizeof(dstr*) * elements);
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

void dstr_vector_incref(dstr_vector *vec)
{
    vec->ref++;
}

static int __dstr_vector_alloc(dstr_vector *vec, unsigned int elements)
{
    int alloc = elements * sizeof(dstr *);
    vec->arr = (dstr **)realloc(vec->arr, alloc);
    if (!vec->arr)
        return 0;
    vec->space = elements;
    return 1;
}

static int __dstr_vector_can_hold(const dstr_vector *vec, unsigned int elements)
{
    if (vec->space >= elements)
        return 1;
    return 0;
}

int dstr_vector_insert(dstr_vector *vec, int pos, dstr *str)
{
    int new_sz, move_n, move_ptr;

    new_sz = vec->sz + 1;
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

int dstr_vector_insert_decref(dstr_vector *vec, int pos, dstr *str)
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
    dstr *str = vec->arr[pos];
    return str;
}

int dstr_vector_is_empty(const dstr_vector *vec)
{
    return !vec->sz;
}

int dstr_vector_size(const dstr_vector *vec)
{
    return vec->sz;
}

int dstr_vector_remove(dstr_vector *vec, int pos)
{
    int x, sz = vec->sz - 1;

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
