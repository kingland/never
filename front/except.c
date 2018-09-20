#include "except.h"
#include <stdlib.h>

except * except_new_all(expr_list * seq)
{
    except * value = (except *)malloc(sizeof(except));

    value->type = EXCEPT_ALL;
    value->no = EXCEPT_NO_UNKNOWN;
    value->id = NULL;
    value->expr_value = expr_new_seq(seq);
    value->line_no = 0;

    return value;
}

except * except_new_id(char * id, expr_list * seq)
{
    except * value = (except *)malloc(sizeof(except));

    value->type = EXCEPT_ID;
    value->no = EXCEPT_NO_UNKNOWN;
    value->id = id;
    value->expr_value = expr_new_seq(seq);
    value->line_no = 0;

    return value;
}

void except_delete(except * value)
{
    if (value->id)
    {
        free(value->id);
    }
    if (value->expr_value)
    {
        expr_delete(value->expr_value);
    }
    free(value);
}

except_list_node * except_list_node_new(except * value)
{
    except_list_node * node = (except_list_node *)malloc(sizeof(except_list_node));

    node->value = value;
    node->prev = NULL;
    node->next = NULL;

    return node;
}

void except_list_node_delete(except_list_node * node)
{
    if (node->value)
    {
        except_delete(node->value);
    }
    free(node);
}

except_list * except_list_new()
{
    except_list * list = (except_list *)malloc(sizeof(except_list));

    list->count = 0;
    list->head = NULL;
    list->tail = NULL;

    return list;
}

void except_list_delete(except_list * list)
{
    except_list_node * node = list->tail;

    while (node != NULL)
    {
        except_list_node * tmp = node->next;
        except_list_node_delete(node);
        node = tmp;
    }
    free(list);
}

void except_list_add_beg(except_list * list, except * value)
{
    except_list_node * node = except_list_node_new(value);

    list->count++;
    if (list->head == NULL && list->tail == NULL)
    {
        list->head = list->tail = node;
    }
    else
    {
        list->tail->prev = node;
        node->next = list->tail;
        list->tail = node;
    }
}

void except_list_add_end(except_list * list, except * value)
{
    except_list_node * node = except_list_node_new(value);

    list->count++;
    if (list->head == NULL && list->tail == NULL)
    {
        list->head = list->tail = node;
    }
    else
    {
        list->head->next = node;
        node->prev = list->head;
        list->head = node;
    }
}

char * except_to_str(except_no no)
{
    switch (no)
    {
        case EXCEPT_NO_UNKNOWN: return EXCEPT_NO_UNKNOWN_NAME; 
        case EXCEPT_NO_DIVISION: return EXCEPT_NO_DIVISION_NAME; 
        case EXCEPT_NO_ARR_SIZE: return EXCEPT_NO_ARR_SIZE_NAME;
        case EXCEPT_NO_INDEX_OOB: return EXCEPT_NO_INDEX_OOB_NAME;
        case EXCEPT_NO_INVALID: return EXCEPT_NO_INVALID_NAME;
        case EXCEPT_NO_OVERFLOW: return EXCEPT_NO_OVERFLOW_NAME;
        case EXCEPT_NO_UNDERFLOW: return EXCEPT_NO_UNDERFLOW_NAME;
        case EXCEPT_NO_INEXACT: return EXCEPT_NO_INEXACT_NAME;
    }
    
    return EXCEPT_NO_UNKNOWN_NAME;
}



