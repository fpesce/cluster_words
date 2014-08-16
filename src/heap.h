#ifndef HEAP_H
#define HEAP_H

typedef struct heap_t heap_t;
typedef int (heap_cmp_callback_fn_t) (const void *, const void *);
typedef void (heap_display_callback_fn_t) (const void *);
typedef void (heap_del_callback_fn_t) (void *);

/**
 * Make a new heap structure, a heap is a structure that is able to return the
 * smallest (or highest, according to the way you compare data) element of its
 * set, in a complexity of O(lg n) the same as the complexity to insert a
 * random element.
 * @param cmp The function that compare two elements to return the smallest.
 * @param del The function that destroy (de-allocate) an element.
 * @return Return a pointer to a newly allocated heap NULL if an error occured.
 */
heap_t *heap_make(heap_cmp_callback_fn_t *cmp, heap_del_callback_fn_t *del);

/**
 * Re-entrant (Thread safe) version of heap_make.
 * @param cmp The function that compare two elements to return the smallest.
 * @param del The function that destroy (de-allocate) an element.
 * @return Return a pointer to a newly allocated heap NULL if an error occured.
 */
heap_t *heap_make_r(heap_cmp_callback_fn_t *cmp, heap_del_callback_fn_t *del);

/**
 * Deallocate the heap;(NB: if the heap is using apr_pool_t (HAVE_APR macro
 * defined), then it will deallocate elements that have been allocated using
 * the heap allocator, otherwise the function del is used on each element.
 * @param heap The heap you are working with.
 * @return nothing.
 */
void heap_destroy(heap_t *heap);

/**
 * Insert an element in the heap.
 * @param heap The heap you are working with.
 * @param datum The datum you want to insert.
 * @return 0 if no error occured, -1 otherwise.
 */
int heap_insert(heap_t *heap, void *datum);

/**
 * Extract the highest (or the lowest using cmp function) element in the heap,
 * remove it and return it.
 * @param heap The heap you are working with.
 * @return The highest (or lowest according to cmp function) element of the
 * heap, NULL if the heap is empty.
 */
void *heap_extract(heap_t *heap);

/**
 * Get the nth element element in the heap.
 * @param heap The heap you are working with.
 * @param n The index of the tree (take the nth element you encounter in a
 * breadth first traversal of a binary tree)
 * @return A pointer on the nth element of the heap, NULL if there's no
 * such element.
 * @remark if you modify the part of this element that is used in the
 * comparation function, you're doing something really bad!
 */
void *heap_get_nth(const heap_t *heap, unsigned int n);

/**
 * Get the nth element (to a const pointer because you can't extract it until
 * it is the highest or the lowest using cmp function) element in the heap.
 * @param heap The heap you are working with.
 * @param datum The adress of the datum you want to set.
 * @param n The index of the tree (take the nth element you encounter in a
 * breadth first traversal of a binary tree)
 * @return 0 if no error occured, -1 otherwise.
 */
unsigned int heap_size(const heap_t *heap);

/**
 * Re-entrant (Thread safe) version of heap_insert, heap must have been
 * initialized with heap_make_r.
 * @param heap The heap you are working with.
 * @param datum The datum you want to insert.
 * @return 0 if no error occured, -1 otherwise.
 */
int heap_insert_r(heap_t *heap, void *datum);

/**
 * Re-entrant (Thread safe) version of heap_extract, heap must have been
 * initialized with heap_make_r.
 * @param heap The heap you are working with.
 * @return The highest (or lowest according to cmp function) element of the
 * heap, NULL if the heap is empty.
 */
void *heap_extract_r(heap_t *heap);

/** 
 * Attach a callback to the heap in order to display the data stored.
 * @param heap The heap you are working with.
 * @param display A callback used to display content of a datum.
 */
void heap_set_display_cb(heap_t *heap, heap_display_callback_fn_t display);

/** 
 * Display the binary tree using the display callback.
 * @param heap The heap you are working with.
 */
void heap_display(const heap_t *heap);

#endif /* HEAP_H */
