/*
 * pointer_list.h
 * Generic growable array of pointers, with a caller-supplied destructor for
 * freeing each element. Backs OperationList and DataDirectiveList, which
 * share this exact shape (append, iterate, destroy-with-cleanup) and differ
 * only in what each element needs freed.
 */

#ifndef POINTER_LIST_H
#define POINTER_LIST_H

typedef void (*ElementDestructor)(void *element);

typedef struct {
    void **items;
    int count;
    int capacity;
    ElementDestructor destroy_element;
} PointerList;

/**
 * Allocates and initializes an empty pointer list.
 * @param destroy_element called on each element when the list is destroyed
 * @return the new list, or NULL on allocation failure
 */
PointerList* createPointerList(ElementDestructor destroy_element);

/**
 * Appends item to list, growing it if needed. Takes ownership of item.
 * @param list the pointer list
 * @param item the element to append
 * @return 0 on success, -1 on allocation failure
 */
int appendPointer(PointerList *list, void *item);

/**
 * Frees every element (via destroy_element), the backing array, and the
 * list itself.
 * @param list the pointer list to free
 */
void destroyPointerList(PointerList *list);

#endif /* POINTER_LIST_H */
