/*-------------------------------------------------------------------------
 *
 * list.c
 *      implementation for PostgreSQL generic linked list package
 *
 *
 * Portions Copyright (c) 1996-2018, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *
 * IDENTIFICATION
 *      src/backend/nodes/list.c
 *
 *-------------------------------------------------------------------------
 */

#include <assert.h>
#include <errno.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#ifndef bool
typedef unsigned char bool;
#endif

#ifndef true
#define true	((bool) 1)
#endif

#ifndef false
#define false	((bool) 0)
#endif

typedef struct ListCell ListCell;
typedef unsigned int Oid;

typedef enum NodeTag
{
	T_Invalid = 0,
    T_List,
	T_IntList,
	T_OidList

}NodeTag;


typedef struct List
{
    NodeTag        type;            /* T_List, T_IntList, or T_OidList */
    int            length;
    ListCell   *head;
    ListCell   *tail;
} List;

struct ListCell
{
    union
    {
        void       *ptr_value;
        int            int_value;
        Oid            oid_value;
    }            data;
    ListCell   *next;
};

#define NIL                        ((List *) NULL)


#define foreach(cell, l)    \
    for ((cell) = list_head(l); (cell) != NULL; (cell) = lnext(cell))

#define for_each_cell(cell, initcell)    \
    for ((cell) = (initcell); (cell) != NULL; (cell) = lnext(cell))


static inline ListCell *
list_head(const List *l)
{
    return l ? l->head : NULL;
}

static inline ListCell *
list_tail(List *l)
{
    return l ? l->tail : NULL;
}

static inline int
list_length(const List *l)
{
    return l ? l->length : 0;
}



typedef struct Node
{
    NodeTag        type;
} Node;

#define nodeTag(nodeptr)        (((const Node*)(nodeptr))->type)

#define IsA(nodeptr,_type_)        (nodeTag(nodeptr) == T_##_type_)


#define lnext(lc)                ((lc)->next)
#define lfirst(lc)                ((lc)->data.ptr_value)
#define lfirst_int(lc)            ((lc)->data.int_value)
#define lfirst_oid(lc)            ((lc)->data.oid_value)

#define linitial(l)                lfirst(list_head(l))
#define linitial_int(l)            lfirst_int(list_head(l))
#define linitial_oid(l)            lfirst_oid(list_head(l))

#define llast(l)                lfirst(list_tail(l))
#define llast_int(l)            lfirst_int(list_tail(l))
#define llast_oid(l)            lfirst_oid(list_tail(l))

#define IsPointerList(l)        ((l) == NIL || IsA((l), List))
#define IsIntegerList(l)        ((l) == NIL || IsA((l), IntList))
#define IsOidList(l)            ((l) == NIL || IsA((l), OidList))


static void check_list_invariants(const List *list)
{
    if (list == NIL)
        return;

    assert(list->length > 0);
    assert(list->head != NULL);
    assert(list->tail != NULL);

    assert(list->type == T_List ||
           list->type == T_IntList ||
           list->type == T_OidList);

    if (list->length == 1)
        assert(list->head == list->tail);
    if (list->length == 2)
        assert(list->head->next == list->tail);
    assert(list->tail->next == NULL);
}

static List *new_list(NodeTag type)
{
    List       *new_list;
    ListCell   *new_head;

    new_head = (ListCell *) malloc(sizeof(*new_head));
    new_head->next = NULL;
    /* new_head->data is left undefined! */

    new_list = (List *) malloc(sizeof(*new_list));
    new_list->type = type;
    new_list->length = 1;
    new_list->head = new_head;
    new_list->tail = new_head;

    return new_list;
}

static void new_head_cell(List *list)
{
    ListCell   *new_head;

    new_head = (ListCell *) malloc(sizeof(*new_head));
    new_head->next = list->head;

    list->head = new_head;
    list->length++;
}

static void new_tail_cell(List *list)
{
    ListCell   *new_tail;

    new_tail = (ListCell *) malloc(sizeof(*new_tail));
    new_tail->next = NULL;

    list->tail->next = new_tail;
    list->tail = new_tail;
    list->length++;
}

List *lappend(List *list, void *datum)
{
    assert(IsPointerList(list));

    if (list == NIL)
        list = new_list(T_List);
    else
        new_tail_cell(list);

    lfirst(list->tail) = datum;
    check_list_invariants(list);
    return list;
}

List *lappend_int(List *list, int datum)
{
    assert(IsIntegerList(list));

    if (list == NIL)
        list = new_list(T_IntList);
    else
        new_tail_cell(list);

    lfirst_int(list->tail) = datum;
    check_list_invariants(list);
    return list;
}

List *lappend_oid(List *list, Oid datum)
{
    assert(IsOidList(list));

    if (list == NIL)
        list = new_list(T_OidList);
    else
        new_tail_cell(list);

    lfirst_oid(list->tail) = datum;
    check_list_invariants(list);
    return list;
}

static ListCell *add_new_cell(List *list, ListCell *prev_cell)
{
    ListCell   *new_cell;

    new_cell = (ListCell *) malloc(sizeof(*new_cell));
    /* new_cell->data is left undefined! */
    new_cell->next = prev_cell->next;
    prev_cell->next = new_cell;

    if (list->tail == prev_cell)
        list->tail = new_cell;

    list->length++;
    return new_cell;
}


ListCell *lappend_cell(List *list, ListCell *prev, void *datum)
{
    ListCell   *new_cell;

    assert(IsPointerList(list));

    new_cell = add_new_cell(list, prev);
    lfirst(new_cell) = datum;
    check_list_invariants(list);
    return new_cell;
}

ListCell *lappend_cell_int(List *list, ListCell *prev, int datum)
{
    ListCell   *new_cell;

    assert(IsIntegerList(list));

    new_cell = add_new_cell(list, prev);
    lfirst_int(new_cell) = datum;
    check_list_invariants(list);
    return new_cell;
}

ListCell * lappend_cell_oid(List *list, ListCell *prev, Oid datum)
{
    ListCell   *new_cell;

    assert(IsOidList(list));

    new_cell = add_new_cell(list, prev);
    lfirst_oid(new_cell) = datum;
    check_list_invariants(list);
    return new_cell;
}
static void list_free_private(List *list, bool deep)
{
    ListCell   *cell;

    check_list_invariants(list);

    cell = list_head(list);
    while (cell != NULL)
    {
        ListCell   *tmp = cell;

        cell = lnext(cell);
        if (deep)
            free(lfirst(tmp));
        free(tmp);
    }

    if (list)
        free(list);
}

void list_free(List *list)
{
    list_free_private(list, false);
}

void list_free_deep(List *list)
{
    /*
     * A "deep" free operation only makes sense on a list of pointers.
     */
    assert(IsPointerList(list));
    list_free_private(list, true);
}

int main()
{
    List *list = NIL;
    int iVar = 0;
    int i = 0;
    int iMax = 10;
    for(i=0; i<iMax; i++)
    {
        list = lappend_int(list, i);
    }

    ListCell *cell;
    foreach(cell, list)
    {
        iVar = (int)lfirst(cell);
        fprintf(stdout, "%d\n", iVar);
    }

    return 0;
}
