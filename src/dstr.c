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
#include <malloc.h>

#include "dstr.h"

/****************************** DYNAMIC STRING ********************************/

static int alloc_more(dstr* str, int sz)
{
    int more_mem;

    more_mem = (int)(str->allocd_mem + (sz * sizeof(char))) * str->growth_rate;
    str->allocd_mem = more_mem;
    str->buf = realloc(str->buf, more_mem);
    if (!str->buf)
        return 0;
    return 1;
}

static int can_hold(const dstr *str, int chars)
{
    if (!str->buf)
        return 0;
    else
        return (chars < str->allocd_mem);
}

void dstr_decref(dstr *str)
{
    str->ref--;
    if (!str->ref){
        free(str->buf);
        free(str);
    }
}

void dstr_incref(dstr *str)
{
    str->ref++;
}

const char *dstr_to_cstr_const(const dstr* str)
{
    return str->buf;
}

dstr *dstr_new()
{
    dstr *str = malloc(sizeof(dstr));

    if (!str)
        return 0;
    str->end = 0;
    str->buf = 0;
    str->growth_rate = 2;
    str->allocd_mem = 0;
    str->ref = 1;
    return str;
}

dstr *dstr_with_initial(const char *initial)
{
    dstr *str = malloc(sizeof(dstr));

    if (!str)
        return 0;
    str->end = strlen(initial);
    str->growth_rate = 2;
    str->allocd_mem = str->end + 1;
    str->buf = strdup(initial);
    if (!str->buf)
        return 0;
    str->ref = 1;
    return str;
}

dstr *dstr_with_prealloc(unsigned int sz)
{
    dstr *str = malloc(sizeof(dstr));
    int pre_alloc_mem = sz + 1;

    if (!str)
        return 0;
    str->end = 0;
    str->buf = 0;
    str->allocd_mem = 0;
    str->growth_rate = 2;
    if (!alloc_more(str, pre_alloc_mem))
        return 0;
    str->buf[0] = '\0';
    str->ref = 1;
    return str;
}

int dstr_compact(dstr *str)
{
    int alloc;
    if (str->allocd_mem > str->end){
        alloc = str->end + 1;
        str->buf = realloc(str->buf, str->end + 1);
        str->allocd_mem = alloc;
        if (str->buf)
            return 1;
    }
    return 0;
}

int dstr_append(dstr* dest, const dstr* src)
{
    int total = src->end + dest->end;
    if (!can_hold(dest, total + 1)){
        if (!alloc_more(dest, total))
            return 0;
    }
    strcpy(dest->buf+dest->end, src->buf);
    dest->end = total;
    return 1;
}

int dstr_append_cstr(dstr* dest, const char *src)
{
    int total = strlen(src) + dest->end;
    if (!can_hold(dest, total + 1)){
        if (!alloc_more(dest, total))
            return 0;
    }
    strcpy(dest->buf+dest->end, src);
    dest->end = total;
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
    dstr* str = dstr_with_prealloc(copy->end + 1);
    if (!str)
        return 0;
    rc = dstr_append(str, copy);
    if (!rc)
        return 0;
    return str;
}

void dstr_clear(dstr *str)
{
    int i = str->allocd_mem;
    for (; i >= 0; i--){
        str->buf[i] = 0;
    }
    str->end = 0;
}

int dstr_print(const dstr *src)
{
    return printf("%s", src->buf);
}

void dstr_growth_rate(dstr *dest, int rate)
{
    dest->growth_rate = rate;
}

/**************************** DYNAMIC STRING LIST  ****************************/

dstr_list *dstr_list_new()
{
    dstr_list *list = malloc(sizeof(dstr_list));
    if (!list)
        return 0;
    list->first = 0;
    list->last = 0;
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

    if (list->last) {
        list->last->next = link;
        link->prev = list->last;
        list->last = link;
    } else {
        list->first = link;
        list->last = link;
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
            list->last = prev;
        }
    } else {
        if (next){
            next->prev = 0;
            list->first = next;
        } else {
            list->first = 0;
            list->last = 0;
        }
    }

    dstr_decref(link->str);
    free(link);
}

void dstr_list_traverse(dstr_list * list, void (*callback)(dstr *))
{
    dstr_link_t *link;

    for (link = list->first; link; link = link->next){
        callback((void *) link->str);
    }
}

void dstr_list_traverse_reverse (dstr_list *list, void (*callback)(dstr *))
{
    dstr_link_t * link;

    for (link = list->last; link; link = link->prev) {
        callback ((void *) link->str);
    }
}

void dstr_list_traverse_delete (dstr_list * list, int (*callback)(dstr *))
{
    dstr_link_t *link;

    for (link = list->first; link; link = link->next) {
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
        for (link = list->first; link; link = next){
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
        for (link = list->first; link; link = link->next){
            dstr_append(str, link->str);
            if (link->next)
                dstr_append_cstr(str, sep);
        }
    } else {
        for (link = list->first; link; link = link->next){
            dstr_append(str, link->str);
        }
    }

    return str;
}
