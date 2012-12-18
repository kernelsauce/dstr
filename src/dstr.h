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

#include <stdlib.h>

#ifndef _DSTR_H
#define _DSTR_H 1

typedef struct dstr{
    char* data; ///< Internal pointer.
    size_t sz; ///< Current size of string.
    size_t mem; ///< Current memory allocated.
    unsigned int ref; ///< Reference count.
    unsigned int grow_r; ///< Memory growth rate.
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
 * Create a new dynamic string object filled with initial C string up until n
 * characters.
 * @param initial Initial string to copy into dynamic string.
 * @return a new dynamic string with copied initial string.
 */
dstr *dstr_with_initialn(const char *initial, size_t n);

/**
 * Create a new dynamic string object with pre allocated space.
 * @param sz Amount of charaters to pre-allocate for.
 * @return a new dynamic string with pre-allocated space.
 */
dstr *dstr_with_prealloc(size_t sz);

/**
 * Returns pointer to C string from given dynamic string.
 * @note When the dynamic string is changed the data of the pointer is
 *  also changed, and vice versa.
 * @param str Source dynamic string to return C string from.
 * @return Zero terminated C string from dynamic string.
 */
const char *dstr_to_cstr_const(const dstr* str);

/**
 * Decreases reference to the dynamic string.
 * If no more references is found, the string is free'd.
 * @param str The dynamic string to decrease reference for.
 */
void dstr_decref(dstr *str);

/**
 * Increases reference to the dynamic string.
 * @param str The dynamic string to increase reference for.
 */
void dstr_incref(dstr *str);

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
 * Appends a C string up until n characters to a dynamic string.
 * @param dest Destination string to append to.
 * @param src Source C string to append.
 * @return 0 on failure, 1 on success.
 */
int dstr_append_cstrn(dstr* dest, const char *src, size_t n);

/**
 * Creates a copy of a dynamic string object.
 * @param copy The dynamic string to copy.
 * @return A independent copy of the source dynamic string, or 0 on allocation failure.
 */
dstr *dstr_copy(const dstr *copy);

/**
 * Clears contents of a dynamic string.
 * Fills the entire memory area occupised with zero bytes.
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
 * Search for needle in a haystack (string).
 * @param haystack The string to search in.
 * @param needle The C string to search for.
 * @return 0 if not contains, 1 if contains.
 */
int dstr_contains(const dstr *haystack, const char *needle);

/**
 * Search for needle in a haystack (string).
 * @param haystack The dynamic string to search in.
 * @param needle The dynamic string to search for.
 * @return 0 if not contains, 1 if contains.
 */
int dstr_contains_dstr(const dstr *haystack, const dstr *needle);

/**
 * Check if string starts with.
 * @param str Dynamic string to match start of.
 * @param starts_with C string to match with.
 * @return 0 if not starts with, 1 if starts with.
 */
int dstr_starts_with(const dstr *str, const char *starts_with);

/**
 * Check if string starts with.
 * @param str Dynamic string to match start of.
 * @param starts_with Dynamic string to match with.
 * @return 0 if not starts with, 1 if starts with.
 */
int dstr_starts_with_dstr(const dstr *str, const dstr *starts_with);

/**
 * Split a dynamic string to a vector.
 * @param str String to split.
 * @param sep Character to split on.
 * @return Vector with string fragments. If none were found a zero sized vector
 *  is returned.
 */
dstr_vector *dstr_split_to_vector(const dstr *str, const char *sep);

/**
 * Modify the growth rate of memory when doing new allocations.
 * Higher rate means more memory potentially lost, but string concatination speeds up.
 * Lower rate means the opposite.
 * @param dest The dynamic string to modify.
 * @param rate The rate that required memory is multiplied by when allocating (to avoid thrasing).
 */
void dstr_growth_rate(dstr *dest, int rate);

/**
 * Print the string to stdout.
 * @param src The string to print.
 * @return Characters printed.
 */
int dstr_print(const dstr *src);

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
 * Get the amount of elements in the list.
 * @param list The list to count.
 * @return Amount of elements.
 */
size_t dstr_list_size(const dstr_list *list);

/**
 * Traverse a list with a callback.
 * @param list The list to traverse
 */
void dstr_list_traverse(dstr_list * list,
                        void (*callback)(dstr *, void *),
                        void *user_data);

/**
 * Traverse a list in reverse with a callback.
 * @param list The list to traverse
 */
void dstr_list_traverse_reverse (dstr_list *list,
                                 void (*callback)(dstr *, void *),
                                 void *userdata);

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
 * Add reference to string list.
 * @param list The list to modify.
 */
void dstr_list_incref (dstr_list *list);

/**
 * Concat a string list a dynamic string.
 * @param sep Seperator to seperate each list element. Use 0 if not wanted.
 * @param list The list to concat.
 * @return A dynamic string.
 */
dstr *dstr_list_to_dstr(const char *sep, dstr_list *list);

/********************* DYNAMIC VECTOR LIST PUBLIC API *************************/

#define DSTR_VECTOR_END 0xffffff ///< End of vector position.
#define DSTR_VECTOR_BEGIN  0x0 ///< Start of vector position.

/**
 * Create a new vector with no initial size.
 * @note To avoid thrashing of reallocations it is advised to prealloc large
 *  vectors with dstr_vector_prealloc();
 * @see dstr_vector_prealloc
 * @return New dstr_vector.
 */
dstr_vector *dstr_vector_new();

/**
 * Creates a new vector with a initial size.
 * @param elements The amount of strings to pre allocate for.
 * @return New dstr_vector.
 */
dstr_vector *dstr_vector_prealloc(unsigned int elements);

/**
 * Insert a string into position in vector.
 * @note It is slow to insert elements into the middle or front of vectors.
 * @note There is no safety that prevents out of boundary positions to be used!
 * @param vec Vector to insert into.
 * @param pos Position to insert in.
 * @param str The string to insert.
 * @return 0 on failure, 1 on success.
 */
int dstr_vector_insert(dstr_vector *vec, int pos, dstr *str);

/**
 * Insert a string into position in vector and steal its reference.
 * @note It is slow to insert elements into the middle of vectors.
 * @note There is no safety that prevents out of boundary positions to be used!
 * @param vec Vector to insert into.
 * @param pos Position to insert in.
 * @param str The string to insert.
 * @return 0 on failure, 1 on success.
 * @see dstr_vector_insert
 */
int dstr_vector_insert_decref(dstr_vector *vec, int pos, dstr *str);

/**
 * Push a string to front of vector.
 * @note Insertions into front of vectors are slow.
 * @param vec Vector to insert into.
 * @param str The string to push.
 * @return 0 on failure, 1 on success.
 */
int dstr_vector_push_front(dstr_vector *vec, dstr *str);

/**
 * Push a string to front of vector and steal its reference.
 * @note Insertions into front of vectors are slow.
 * @param vec Vector to insert into.
 * @param str The string to push.
 * @return 0 on failure, 1 on success.
 */
int dstr_vector_push_front_decref(dstr_vector *vec, dstr *str);

/**
 * Push a string to back of vector.
 * @param vec Vector to insert into.
 * @param str The string to push.
 * @return 0 on failure, 1 on success.
 */
int dstr_vector_push_back(dstr_vector *vec, dstr *str);

/**
 * Push a string to back of vector and steal its reference.
 * @param vec Vector to insert into.
 * @param str The string to push.
 * @return 0 on failure, 1 on success.
 */
int dstr_vector_push_back_decref(dstr_vector *vec, dstr *str);

/**
 * Pop item from back of vector.
 * @param vec Vector to pop from.
 */
void dstr_vector_pop_back(dstr_vector *vec);

/**
 * Pop item from front of vector.
 * @param vec Vector to pop from.
 */
void dstr_vector_pop_front(dstr_vector *vec);

/**
 * Return item from back of vector.
 * @param vec Vector to return from.
 */
dstr *dstr_vector_back(dstr_vector *vec);

/**
 * Return item from front of vector.
 * @param vec Vector to return from.
 */
dstr *dstr_vector_front(dstr_vector *vec);

/**
 * Get string from position.
 * @param vec The vector to get from.
 * @param pos The position to return.
 * @return dstr at the position.
 */
dstr *dstr_vector_at(dstr_vector *vec, int pos);

/**
 * Check if a vector is empty or not.
 * @param vec The vector to check.
 * @return 1 if empty, 0 if not empty.
 */
int dstr_vector_is_empty(const dstr_vector *vec);

/**
 * Get the size of vector.
 * @param vec Vector to count.
 * @return The current amount of strings in the vector.
 */
size_t dstr_vector_size(const dstr_vector *vec);

/**
 * Remove a string at given position from a vector.
 * The positions after the removed position are moved to close gap.
 * @note Removing items from the middle or front of a vector is slow.
 * @param vec The vector to remove from.
 * @param pos The position to remove.
 * @return
 */
int dstr_vector_remove(dstr_vector *vec, size_t pos);

/**
 * Decrement reference count by one. When no more references exists the
 * vector is emptied (and strings decrefed) and free'd.
 * @param vec The vector to decref.
 * @see dstr_vector_incref
 */
void dstr_vector_decref(dstr_vector *vec);

/**
 * Increment referece count by one.
 * @see dstr_vector_decref
 * @param vec The vector to decref.
 */
void dstr_vector_incref(dstr_vector *vec);

#endif /* dstr.h */
