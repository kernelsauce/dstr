/*  Reference counted dynamic string and string containers.
    Copyright (C) 2012 John Abrahamsen <jhnabrhmsn@gmail.com>

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
    WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.   */

#ifndef _DSTR_H
#define _DSTR_H 1
#include <stdlib.h>

/* Check boundaries for containers access if defined. Slowdown, better to
   code right.  */
#define DSTR_MEM_SECURITY

typedef struct dstr{
    char* data; // Internal pointer.
    size_t sz; // Current size of string.
    size_t mem; // Current memory allocated.
    unsigned int ref; // Reference count.
    unsigned int grow_r; // Memory growth rate.
} dstr;

typedef struct dstr_link_t {
    dstr *str;
    struct dstr_link_t *prev;
    struct dstr_link_t *next;
} dstr_link_t;

typedef struct dstr_list {
    dstr_link_t *head;
    dstr_link_t *tail;
    int ref;
} dstr_list;

typedef struct dstr_vector{
    dstr **arr;
    size_t sz;
    size_t space;
    unsigned int ref;
} dstr_vector;


/*                       DYNAMIC STRING PUBLIC API                         */
/* Note: All functions that returns a integer will return 0 for failure
   and 1 for success. Booleans are not used as to support C89 compilers.
   Functions returning pointers will return 0  on memory allocation
   failures.   */
#define DSTR_MEM_EXPAND_RATE 2 // How much to grow per allocation.
//#define DSTR_MEM_CLEAR  // When free'ing or reallocing clear contents.

/* Create a new dynamic string object.   */
dstr *dstr_new();
/* Create a new dynamic string object filled with initial C string.   */
dstr *dstr_with_initial(const char *initial);
/* Create a new dynamic string object filled with initial C string up until n
   characters.   */
dstr *dstr_with_initialn(const char *initial, size_t n);
/* Create a new dynamic string object with pre allocated space.   */
dstr *dstr_with_prealloc(size_t sz);

/* Returns internal pointer to C string from given dynamic string. When the
   dynamic string is changed the data of the pointer is
   also changed, and vice versa.
   Do not modify returned character array. Doing so will corrupt string
   size and memory sizes.   */
const char *dstr_to_cstr_const(const dstr* str);
/* Copy dynamic string to C string. You must free the returned pointer with free
   when no longer in use.   */
char *dstr_copy_to_cstr(const dstr* str);

/* Decreases reference to the dynamic string.
   If no more references exists, the string is free'd.   */
void dstr_decref(dstr *str);
/* Increases reference to the string by one.   */
void dstr_incref(dstr *str);

/* Appends a dynamic string to a dynamic string. See dstr_append_decref for
   a reference stealing implementation   */
int dstr_append(dstr* dest, const dstr* src);
/* Appends a dynamic string to a dynamic string. Also steals one reference
   attached to the source string.   */
int dstr_append_decref(dstr* dest, dstr* src);
/* Prepends a dynamic string to a dynamic string.   */
int dstr_prepend(dstr* dest, const dstr* src);
/* Prepends a dynamic string to a dynamic string. This function also steals
   one reference attached to the source string.   */
int dstr_prepend_decref(dstr* dest, dstr *src);
/* Prepends a C string to a dynamic string.   */
int dstr_prepend_cstr(dstr* dest, const char *src);
/* Prepends a C string, up until n characters, to a dynamic string.    */
int dstr_prepend_cstrn(dstr* dest, const char *src, size_t n);
/* Appends a C string to a dynamic string.   */
int dstr_append_cstr(dstr* dest, const char *src);
/* Appends a C string up until n characters to a dynamic string.   */
int dstr_append_cstrn(dstr* dest, const char *src, size_t n);

/* Creates a copy of a dynamic string object, with one reference.   */
dstr *dstr_copy(const dstr *copy);

/* Clears contents of a dynamic string. Fills the entire memory area occupied
   with zero bytes. Unused memory is not free'd, use dstr_compact for that
   purpose   */
void dstr_clear(dstr *str);
/* Compacts the memory used by a string. Not needed unless you are going from a
   really big string to a very small one.   */
int dstr_compact(dstr *str);

/* Search for needle in a haystack (C string).   */
int dstr_contains(const dstr *haystack, const char *needle);
/* Search for needle in a haystack (dynamic string).   */
int dstr_contains_dstr(const dstr *haystack, const dstr *needle);

/* Check if string starts with a sub C string.   */
int dstr_starts_with(const dstr *str, const char *starts_with);
/* Check if string starts with a sub dstr.   */
int dstr_starts_with_dstr(const dstr *str, const dstr *starts_with);
/* Check if string ends with a sub C string.   */
int dstr_ends_with(const dstr *str, const char *ends_with);
/* Check if string ends with a sub dstr.   */
int dstr_ends_with_dstr(const dstr *str, dstr *ends_with);

/* Split a dynamic string to a vector. If none occurences of seperator is
   found a zero sized vector is returned.   */
dstr_vector *dstr_split_to_vector(const dstr *str, const char *sep);
/* Split a dynamic string to a list. If none were found a empty list
   is returned.   */
dstr_list *dstr_split_to_list(const dstr *str, const char *sep);

/* Modify the growth rate of memory when doing new allocations. Higher rate
   means more memory potentially lost, but string concatination speeds up.
   Lower rate means the opposite. The amount of memory to be allocated is
   multiplied by rate. Default is defined in DSTR_MEM_EXPAND_RATE.   */
void dstr_growth_rate(dstr *dest, int rate);

/* Print the string to stdout.   */
int dstr_print(const dstr *src);


/*                     DYNAMIC STRING LIST PUBLIC API                       */
/* Note: The choice between linked lists and vector depends on your need to
   access random elements in the collection. If you are going to operate on your
   collection randomly you are better off to pick dstr_vector as a container.
   List are generally good for accessing head, tail elements and when there's a
   need to run straight through them.   */

/* Creates a new referenced counted list for dynamic strings.   */
dstr_list *dstr_list_new();

/* Add a dynamic string to a list. One reference is added to the dynamic string.
   Which will be removed when the string is removed from the list or the lists
   has no more references and all elements are decref'ed.   */
int dstr_list_add(dstr_list *list, dstr *str);
/* Add a dynamic string to a list. No reference is added. Nonetheless one
   reference will be removed when the string is removed from the list or the
   lists has no more references.   */
int dstr_list_add_decref(dstr_list *dest, dstr *str);

/* Remove a dynamic string from a list (string is decref'ed).   */
void dstr_list_remove(dstr_list *list, dstr_link_t *link);

/* Get the amount of elements in the list. SLOOW!  */
size_t dstr_list_size(const dstr_list *list);

/* Traverse a list with a callback. Callback should take two arguments.
   First argument is dstr*, second is user data if applicable.   */
void dstr_list_traverse(dstr_list * list,
                        void (*callback)(dstr *, void *),
                        void *user_data);
/* Same as dstr_list_traverse, only we traverse backwards.   */
void dstr_list_traverse_reverse (dstr_list *list,
                                 void (*callback)(dstr *, void *),
                                 void *userdata);
/* Traverse a list with a callback that return 0 or 1 depending on it wants
   to delete the element or not.   */
void dstr_list_traverse_delete (dstr_list * list, int (*callback)(dstr *));

/* Concat a string list a dynamic string. Seperator to seperate each list
   element is optional, use 0 if not wanted.   */
dstr *dstr_list_to_dstr(const char *sep, dstr_list *list);

/* Decrement one reference from string list.   */
void dstr_list_decref (dstr_list *list);
/* Add one reference to the string list.   */
void dstr_list_incref (dstr_list *list);


/*                    DYNAMIC STRING VECTOR PUBLIC API                      */
/* Note: There is no safety that prevents out of boundary positions to be
   used, unless DSTR_MEM_SECURITY IS DEFINED TO 1! E.g dstr_vector_back on a
   empty vector will give undefined behaviour.    */
#define DSTR_VECTOR_END 0xffffff // End of vector position magix.
#define DSTR_VECTOR_BEGIN  0x0 // Start of vector position magix.
#define DSTR_VECTOR_MEM_EXPAND_RATE 3 // How much to grow per allocation.

/* Create a new vector with no initial size. To avoid thrashing of reallocations
   it is advised to prealloc large vectors with dstr_vector_prealloc.   */
dstr_vector *dstr_vector_new();

/* Creates a new vector with a initial size.   */
dstr_vector *dstr_vector_prealloc(size_t elements);

/* Insert a string into position in vector. It is slow to insert elements into
   the middle or front of vectors.  */
int dstr_vector_insert(dstr_vector *vec, size_t pos, dstr *str);
/* Same as dstr_vector_insert only this will decref the string being
   inserted.  */
int dstr_vector_insert_decref(dstr_vector *vec, size_t pos, dstr *str);
/* Push a string to front of vector. Insertions into front of vectors are
   slow.   */
int dstr_vector_push_front(dstr_vector *vec, dstr *str);
/* Push a string to front of vector and steal its reference.   */
int dstr_vector_push_front_decref(dstr_vector *vec, dstr *str);
/* Push a string to back of vector.   */
int dstr_vector_push_back(dstr_vector *vec, dstr *str);
/* Push a string to back of vector and steal its reference.   */
int dstr_vector_push_back_decref(dstr_vector *vec, dstr *str);

/* Pop item from back of vector.  */
void dstr_vector_pop_back(dstr_vector *vec);
/* Pop item from front of vector. Slow.   */
void dstr_vector_pop_front(dstr_vector *vec);
/* Remove a string at given position from a vector. The positions after the
   removed position are moved to close gap. Removing items from the middle or
   front of a vector is slow.   */
int dstr_vector_remove(dstr_vector *vec, size_t pos);

/* Return item from back of vector.   */
dstr *dstr_vector_back(dstr_vector *vec);
/* Return item from front of vector.   */
dstr *dstr_vector_front(dstr_vector *vec);
/* Get string from position.   */
dstr *dstr_vector_at(dstr_vector *vec, int pos);

/* Check if a vector is empty or not.   */
int dstr_vector_is_empty(const dstr_vector *vec);
/* Get the size of vector.  */
size_t dstr_vector_size(const dstr_vector *vec);

/* Decrement reference count by one. When no more references exists the
 * vector is emptied (and strings decrefed) and free'd.   */
void dstr_vector_decref(dstr_vector *vec);
/* Increment referece count by one.   */
void dstr_vector_incref(dstr_vector *vec);

#endif /* dstr.h */
