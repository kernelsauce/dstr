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
#define DSTR_MAJOR_VERSION 1
#define DSTR_MINOR_VERSION 0
#define DSTR_VERSION "1.0"
#ifndef SIZE_MAX
    #define SIZE_MAX (sizeof(size_t) -1)
#endif

/* Note: All functions that returns a integer will return 0 for failure
   and 1 for success unless otherwise is specified. Booleans are not used as to
   support C89 compilers. Functions returning pointers will return 0 on memory
   allocation failures or out of boundary exceptions (if compiled with boundary
   protection).    */

typedef struct dstr{
    char* data; // Internal pointer.
    size_t sz; // Current size of string.
    size_t mem; // Current memory allocated.
    unsigned int ref; // Reference count.
} dstr;

typedef struct dstr_link{
    dstr *str;
    struct dstr_link *prev;
    struct dstr_link *next;
} dstr_link;

typedef struct dstr_list{
    dstr_link *head;
    dstr_link *tail;
    int ref;
} dstr_list;

typedef struct dstr_vector{
    dstr **arr;
    size_t sz;
    size_t space;
    unsigned int ref;
} dstr_vector;

/* Get library version as string. E.g 1.0, 1.0.1.   */
dstr *dstr_version();

/*                       DYNAMIC STRING PUBLIC API                         */
/* Compile time define options:
   DSTR_MEM_EXPAND_RATE: defines how many times to multiply memory
   allocations to give avoid allocation thrasing. Default is 2.
   DSTR_MEM_CLEAR: zero all memory being released to hold characeter arrays. */
#ifndef DSTR_MEM_EXPAND_RATE
  #define DSTR_MEM_EXPAND_RATE 2 // How much to grow per allocation.
#endif

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
#define dstr_incref(str) \
    (str->ref++)

/* Return current string length (not including sentinel).   */
size_t dstr_len(const dstr* str);

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
/* Append string printf style.   */
int dstr_sprintf(dstr *str, const char *fmt, ...);

/* Make complete string upper-case.   */
void dstr_to_upper(dstr *str);
/* Make complete string lower-case.   */
void dstr_to_lower(dstr *str);
/* Capitalize first letter.   */
void dstr_capitalize(dstr *str);

/* Creates a copy of a dynamic string object, with one reference.   */
dstr *dstr_copy(const dstr *copy);

/* Clears contents of a dynamic string. Fills the entire memory area occupied
   with zero bytes. Unused memory is not free'd, use dstr_compact for that
   purpose   */
void dstr_clear(dstr *str);
/* Compacts the memory used by a string. Not needed unless you are going from a
   really big string to a very small one.   */
int dstr_compact(dstr *str);

/* Search for needle in a haystack (C string). Returns n occurences.  */
int dstr_contains(const dstr *haystack, const char *needle);
/* Search for needle in a haystack (dynamic string). Returns n occurences.  */
int dstr_contains_dstr(const dstr *haystack, const dstr *needle);
/* Check if string starts with a sub C string.   */
int dstr_starts_with(const dstr *str, const char *starts_with);
/* Check if string starts with a sub dstr.   */
int dstr_starts_with_dstr(const dstr *str, const dstr *starts_with);
/* Check if string ends with a sub C string.   */
int dstr_ends_with(const dstr *str, const char *ends_with);
/* Check if string ends with a sub dstr.   */
int dstr_ends_with_dstr(const dstr *str, dstr *ends_with);
/* Check if string is a exact match to sub C string.   */
int dstr_matches(const dstr *haystack, const char *needle);

/* Split a dynamic string to a vector. If none occurences of seperator is
   found a zero sized vector is returned.   */
dstr_vector *dstr_split_to_vector(const dstr *str, const char *sep);
/* Split a dynamic string to a list. If none were found a empty list
   is returned.   */
dstr_list *dstr_split_to_list(const dstr *str, const char *sep);

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
void dstr_list_remove(dstr_list *list, dstr_link *link);

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
/* For each macro for list.    */
#define DSTR_LIST_FOREACH(list, link) \
    for (link = list->head; link; link = link->next)

/* Concat a string list a dynamic string. Seperator to seperate each list
   element is optional, use 0 if not wanted.   */
dstr *dstr_list_to_dstr(const char *sep, dstr_list *list);

/* Returns a new list of strings found in input list that contains
   sub C string.   */
dstr_list *dstr_list_search_contains(dstr_list *search, const char * substr);
/* Returns a new list of strings found in input list that contains
   sub dynamic string.   */
dstr_list *dstr_list_search_contains_dstr(dstr_list *search, const dstr *substr);

/* Decode a bencoded list as to dstr_list.    */
dstr_list *dstr_list_bdecode(const char *str);

/* Bencode a string list.    */
dstr *dstr_list_bencode(const dstr_list *list);

/* Decrement one reference from string list.   */
void dstr_list_decref (dstr_list *list);
/* Add one reference to the string list.   */
#define dstr_list_incref(list) \
    (list->ref++)

/*                    DYNAMIC STRING VECTOR PUBLIC API                      */
/* Note: There is no safety that prevents out of boundary positions to be
   used, unless DSTR_MEM_SECURITY IS DEFINED TO 1! E.g dstr_vector_back on a
   empty vector will give undefined behaviour.

   Compile time define options:
   DSTR_MEM_SECURITY: if defined boundaries for vectors are checked. If a
   invalid position is requested a null pointer will be returned.
   DSTR_VECTOR_MEM_EXPAND_RATE: defines how many times to multiply memory
   allocations to give avoid allocation thrasing.   */
#include <limits.h>
#define DSTR_VECTOR_END SIZE_MAX // End of vector position magix.
#define DSTR_VECTOR_BEGIN  0x0 // Start of vector position magix.
#ifndef DSTR_VECTOR_MEM_EXPAND_RATE
    #define DSTR_VECTOR_MEM_EXPAND_RATE 3 // How much to grow per allocation.
#endif

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
dstr *dstr_vector_at(dstr_vector *vec, size_t pos);

/* Check if a vector is empty or not.   */
int dstr_vector_is_empty(const dstr_vector *vec);
/* Get the size of vector.  */
size_t dstr_vector_size(const dstr_vector *vec);

/* Decrement reference count by one. When no more references exists the
 * vector is emptied (and strings decrefed) and free'd.   */
void dstr_vector_decref(dstr_vector *vec);
/* Increment referece count by one.   */
#define dstr_vector_incref(vec) \
    (vec->ref++)

#endif /* dstr.h */
