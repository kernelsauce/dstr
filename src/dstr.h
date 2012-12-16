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

#ifndef _DSTR_H
#define _DSTR_H 1

typedef struct dstr{
    char* buf;
    unsigned int allocd_mem;
    unsigned int end;
    unsigned int ref;
    unsigned int growth_rate;
} dstr;

typedef struct dstr_link_t {
    dstr *str;
    struct dstr_link_t *prev;
    struct dstr_link_t *next;
} dstr_link_t;

typedef struct dstr_list {
    dstr_link_t *first;
    dstr_link_t *last;
    int ref;
} dstr_list;

/********************* DYNAMIC STRING PUBLIC API ******************************/

/**
 * Create a new dynamic string object.
 * @return a new dynamic string with no buffer allocated.
 */
dstr *dstr_new();

/**
 * Create a new dynamic string object filled with initial C string.
 * @param initial Initial string to copy into dynamic string.
 * @return a new dynamic string with copied initial string.
 */
dstr *dstr_with_initial(const char *initial);

/**
 * Create a new dynamic string object with pre allocated space.
 * @param sz Amount of charaters to pre-allocate for.
 * @return a new dynamic string with pre-allocated space.
 */
dstr *dstr_with_prealloc(unsigned int sz);

/**
 * Returns pointer to C string from given dynamic string.
 * @note When the dynamic string is changed the data of the pointer is also changed.
 * @param str Source dynamic string to return C string from.
 * @return Zero terminated C string from dynamic string.
 */
const char *dstr_to_cstr_const(const dstr* str){ return str->buf; }

/**
 * Decreases reference to the dynamic string.
 * If no more references is found, the string is free'd.
 * @param str The dynamic string to decrease reference for.
 */
void dstr_decref(dstr *str)
{
    str->ref--;
    if (!str->ref){
        free(str->buf);
        free(str);
    }
}

/**
 * Increases reference to the dynamic string.
 * @param str The dynamic string to increase reference for.
 */
void dstr_incref(dstr *str){ str->ref++; }

/**
 * Appends a dynamic string to a dynamic string.
 * @param dest Destination string to append to.
 * @param src Source string to append.
 * @return 0 on failure, 1 on success.
 * @see dstr_append_decref for a reference stealing implementation.
 */
int dstr_append(dstr* dest, const dstr* src);

/**
 * Appends a dynamic string to a dynamic string.
 * This function also steals one reference attached to the source string.
 * @param dest Destination string to append to.
 * @param src Source string to append.
 * @return 0 on failure, 1 on success.
 * @see dstr_append for a non reference stealing implementation.
 */
int dstr_append_decref(dstr* dest, dstr* src);

/**
 * Appends a C string to a dynamic string.
 * @param dest Destination string to append to.
 * @param src Source C string to append.
 * @return 0 on failure, 1 on success.
 */
int dstr_append_cstr(dstr* dest, const char *src);

/**
 * Creates a copy of a dynamic string object.
 * @param copy The dynamic string to copy.
 * @return A independent copy of the source dynamic string, or 0 on allocation failure.
 */
dstr *dstr_copy(const dstr *copy);

/**
 * Clears contents of a dynamic string.
 * @param str  The dynamic string to clear.
 * @note Will not modify reference nor free memory.
 * @see dstr_compact for a deallocation of unused space.
 */
void dstr_clear(dstr *str);

/**
 * Compacts the memory used by a string.
 * @param str The dynamic string object to compact.
 * @return 0 on failure, 1 on success.
 * @note Not needed unless you are going from a really big string to a very small one.
 */
int dstr_compact(dstr *str);

/**
 * Modify the growth rate of memory when doing new allocations.
 * Higher rate means more memory potentially lost, but string concatination speeds up.
 * Lower rate means the opposite.
 * @param dest The dynamic string to modify.
 * @param rate The rate that required memory is multiplied by when allocating (to avoid thrasing).
 */
void dstr_growth_rate(dstr *dest, int rate);

/********************* DYNAMIC STRING LIST PUBLIC API *************************/

/**
 * Creates a new referenced counted list for dynamic strings.
 * @return initialized dynamic string list. If memory could not be allocated, 0 is returned.
 */
dstr_list *dstr_list_new();

/**
 * Add a dynamic string to a list.
 * One reference is added to the dynamic string. Which will be removed when the
 * string is removed from the list or the lists has no more references.
 * @param list The list to add the dynamic string to.
 * @param str The string to add to string list.
 * @return 0 on failure, 1 on success.
 */
int dstr_list_add(dstr_list *list, dstr *str);

/**
 * Add a dynamic string to a list.
 * No reference is added. Nonetheless one reference will be removed when the
 * string is removed from the list or the lists has no more references.
 * @param list The list to add the dynamic string to.
 * @param str The string to add to string list.
 * @return 0 on failure, 1 on success.
 */
int dstr_list_add_decref(dstr_list *dest, dstr *str);

/**
 * Remove a dynamic string from a list.
 * @param list The list to remove from.
 * @param link Link to the dynamic string to remove.
 */
void dstr_list_remove(dstr_list *list, dstr_link_t *link);

/**
 * Traverse a list with a callback.
 * @param list The list to traverse
 */
void dstr_list_traverse(dstr_list * list, void (*callback)(dstr *));

/**
 * Traverse a list in reverse with a callback.
 * @param list The list to traverse
 */
void dstr_list_traverse_reverse (dstr_list *list, void (*callback)(dstr *));

/**
 * Traverse a list with a callback that return 0 or 1 depending on it wants
 * to delete the element or not.
 * @param list The list to traverse.
 */
void dstr_list_traverse_delete (dstr_list * list, int (*callback)(dstr *));

/**
 * Decrement one reference from string list.
 * @param list The list to modify.
 */
void dstr_list_decref (dstr_list *list);

/**
 * Concat a string list a dynamic string.
 * @param sep Seperator to seperate each list element. Use 0 if not wanted.
 * @param list The list to concat.
 * @return A dynamic string.
 */
dstr *dstr_list_to_dstr(const char *sep, dstr_list *list);

#endif /* dstr.h */
