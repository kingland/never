/**
 * Copyright 2018 Slawomir Maludzinski
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
#include "emit.h"
#include "freevar.h"
#include "symtab.h"
#include "utils.h"
#include <assert.h>
#include <stdio.h>

/* GP old, FP old, IP old, line_no, PP old */
#define NUM_FRAME_PTRS 5

/**
 * emit code
 */
int expr_int_emit(expr * value, int stack_level, module * module_value,
                  int * result)
{
    bytecode bc = { 0 };

    bc.type = BYTECODE_INT;
    bc.integer.value = value->int_value;

    bytecode_add(module_value->code, &bc);

    return 0;
}

int expr_float_emit(expr * value, int stack_level, module * module_value,
                    int * result)
{
    bytecode bc = { 0 };

    bc.type = BYTECODE_FLOAT;
    bc.real.value = value->float_value;

    bytecode_add(module_value->code, &bc);

    return 0;
}

int expr_nil_emit(expr * value, int stack_level, module * module_value,
                  int * result)
{
    bytecode bc = { 0 };
    
    bc.type = BYTECODE_NIL_RECORD_REF;
    
    bytecode_add(module_value->code, &bc);
    
    return 0;
}                  

int expr_string_emit(expr * value, int stack_level, module * module_value,
                     int * result)
{
    bytecode bc = { 0 };
    
    bc.type = BYTECODE_STRING;
    bc.string.index = strtab_add_string(module_value->strtab_value, value->string_value);

    bytecode_add(module_value->code, &bc);

    return 0;
}

int func_freevar_id_local_emit(freevar * value, int stack_level,
                               module * module_value, int * result)
{
    bytecode bc = { 0 };

    if (value->src.param_value->type == PARAM_DIM)
    {
        bc.type = BYTECODE_ID_DIM_LOCAL;
        bc.id_dim_local.stack_level = stack_level;
        bc.id_dim_local.index = value->src.param_value->array->index;
        bc.id_dim_local.dim_index = value->src.param_value->index;
    }
    else
    {
        bc.type = BYTECODE_ID_LOCAL;
        bc.id_local.stack_level = stack_level;
        bc.id_local.index = value->src.param_value->index;
    }

    bytecode_add(module_value->code, &bc);

    return 0;
}

int func_freevar_id_bind_emit(freevar * value, int stack_level,
                              module * module_value, int * result)
{
    bytecode bc = { 0 };

    bc.type = BYTECODE_ID_LOCAL;
    bc.id_local.stack_level = stack_level;
    bc.id_local.index = value->src.bind_value->index;

    bytecode_add(module_value->code, &bc);

    return 0;
}

int func_freevar_id_qualifier_emit(freevar * value, int stack_level,
                                   module * module_value, int * result)
{
    bytecode bc = { 0 };

    bc.type = BYTECODE_ID_LOCAL;
    bc.id_local.stack_level = stack_level - value->src.qualifier_value->stack_level - 3;
    bc.id_local.index = 0;
    
    bytecode_add(module_value->code, &bc);
    
    return 0;
}

int func_freevar_emit(freevar * value, int stack_level, module * module_value,
                      int * result)
{
    bytecode bc = { 0 };

    switch (value->src.type)
    {
    case FREEVAR_UNKNOWN:
        print_error_msg(0, "unknown freevar %s during emit\n", value->id);
        assert(0);
        break;
    case FREEVAR_PARAM:
        func_freevar_id_local_emit(value, stack_level, module_value, result);
        break;
    case FREEVAR_QUALIFIER:
        func_freevar_id_qualifier_emit(value, stack_level, module_value, result);
        break;
    case FREEVAR_BIND:
        func_freevar_id_bind_emit(value, stack_level, module_value, result);
        break;
    case FREEVAR_FREEVAR:
        bc.type = BYTECODE_ID_GLOBAL;
        bc.id_global.index = value->src.freevar_value->index;

        bytecode_add(module_value->code, &bc);
        break;
    case FREEVAR_FUNC:
        bc.type = BYTECODE_ID_LOCAL;
        bc.id_local.stack_level = stack_level;
        bc.id_local.index = value->src.func_value->index;

        bytecode_add(module_value->code, &bc);
        break;
    }

    return 0;
}

int func_freevar_list_emit(freevar_list * freevars, int stack_level,
                           module * module_value, int * result)
{
    int e = 0;
    freevar_list_node * node = freevars->tail;
    while (node != NULL)
    {
        freevar * value = node->value;
        if (value != NULL)
        {
            func_freevar_emit(value, stack_level + e++, module_value, result);
        }
        node = node->next;
    }

    return 0;
}

int expr_id_local_emit(expr * value, int stack_level, module * module_value,
                       int * result)
{
    bytecode bc = { 0 };

    if (value->id.id_param_value->type == PARAM_DIM)
    {
        bc.type = BYTECODE_ID_DIM_LOCAL;
        bc.id_dim_local.stack_level = stack_level;
        bc.id_dim_local.index = value->id.id_param_value->array->index;
        bc.id_dim_local.dim_index = value->id.id_param_value->index;
    }
    else
    {
        bc.type = BYTECODE_ID_LOCAL;
        bc.id_local.stack_level = stack_level;
        bc.id_local.index = value->id.id_param_value->index;
    }

    bytecode_add(module_value->code, &bc);

    return 0;
}

int expr_id_bind_emit(expr * value, int stack_level, module * module_value,
                       int * result)
{
    bytecode bc = { 0 };

    bc.type = BYTECODE_ID_LOCAL;
    bc.id_local.stack_level = stack_level;
    bc.id_local.index = value->id.id_bind_value->index;

    bytecode_add(module_value->code, &bc);

    return 0;
}

int expr_id_global_emit(expr * value, int stack_value, module * module_value,
                        int * result)
{
    bytecode bc = { 0 };

    bc.type = BYTECODE_ID_GLOBAL;
    bc.id_global.index = value->id.id_freevar_value->index;

    bytecode_add(module_value->code, &bc);

    return 0;
}

int expr_id_func_top_emit(func * func_value, int stack_level,
                          module * module_value, int * result)
{
    bytecode bc = { 0 };

    bc.type = BYTECODE_GLOBAL_VEC;
    bc.global_vec.count = 0;
    bytecode_add(module_value->code, &bc);

    bc.type = BYTECODE_ID_FUNC_FUNC;
    bc.id_func.func_value = func_value;
    bytecode_add(module_value->code, &bc);

    return 0;
}

int expr_id_func_emit(func * func_value, int stack_level, module * module_value,
                      int * result)
{
    bytecode bc = { 0 };

    bc.type = BYTECODE_ID_LOCAL;
    bc.id_local.stack_level = stack_level;
    bc.id_local.index = func_value->index;

    bytecode_add(module_value->code, &bc);

    return 0;
}

int expr_id_func_nest_emit(func * func_value, int stack_level,
                           module * module_value, int * result)
{
    bytecode bc = { 0 };

    bc.type = BYTECODE_COPYGLOB;
    bytecode_add(module_value->code, &bc);

    bc.type = BYTECODE_ID_FUNC_FUNC;
    bc.id_func.func_value = func_value;
    bytecode_add(module_value->code, &bc);

    return 0;
}

int expr_id_emit(expr * value, int stack_level, module * module_value,
                 int * result)
{
    switch (value->id.id_type_value)
    {
    case ID_TYPE_UNKNOWN:
        print_error_msg(value->line_no,
                        "not recognized id, at this stage it is very bad\n");
        assert(0);
        break;
    case ID_TYPE_LOCAL:
        expr_id_local_emit(value, stack_level, module_value, result);
        break;
    case ID_TYPE_GLOBAL:
        expr_id_global_emit(value, stack_level, module_value, result);
        break;
    case ID_TYPE_BIND:
        expr_id_bind_emit(value, stack_level, module_value, result);
        break;
    case ID_TYPE_QUALIFIER:
        expr_id_qualifier_emit(value, stack_level, module_value, result);
        break;        
    case ID_TYPE_FUNC_TOP:
        if (value->id.id_func_value != NULL)
        {
            expr_id_func_top_emit(value->id.id_func_value, stack_level, module_value,
                                  result);
        }
        break;
    case ID_TYPE_FUNC:
        if (value->id.id_func_value != NULL)
        {
            expr_id_func_emit(value->id.id_func_value, stack_level, module_value,
                              result);
        }
        break;
    case ID_TYPE_FUNC_NEST:
        if (value->id.id_func_value != NULL)
        {
            expr_id_func_nest_emit(value->id.id_func_value, stack_level, module_value,
                                   result);
        }
        break;
    case ID_TYPE_RECORD:
        expr_nil_emit(value, stack_level, module_value, result);
        break;
    case ID_TYPE_ENUMTYPE:
        assert(0);
        break;
    }
    return 0;
}

int expr_neg_emit(expr * value, int stack_level, module * module_value,
                  func_list_weak * list_weak, int * result)
{
    bytecode bc = { 0 };
    
    expr_emit(value->left, stack_level, module_value, list_weak, result);

    if (value->comb.comb == COMB_TYPE_INT)
    {
        bc.type = BYTECODE_OP_NEG_INT;
        bytecode_add(module_value->code, &bc);
    }
    else if (value->comb.comb == COMB_TYPE_FLOAT)
    {
        bc.type = BYTECODE_OP_NEG_FLOAT;
        bytecode_add(module_value->code, &bc);
    }
    else if (value->comb.comb == COMB_TYPE_ARRAY &&
             value->comb.comb_ret->type == PARAM_INT)
    {
        bc.type = BYTECODE_OP_NEG_ARR_INT;
        bytecode_add(module_value->code, &bc);
    }
    else if (value->comb.comb == COMB_TYPE_ARRAY &&
             value->comb.comb_ret->type == PARAM_FLOAT)
    {
        bc.type = BYTECODE_OP_NEG_ARR_FLOAT;
        bytecode_add(module_value->code, &bc);
    }
    else
    {
        *result = EMIT_FAIL;
        print_error_msg(value->line_no, "cannot neg type %s\n",
                        comb_type_str(value->comb.comb));
        assert(0);
    }

    return 0;
}

int expr_add_emit(expr * value, int stack_level, module * module_value,
                  func_list_weak * list_weak, int * result)
{
    bytecode bc = { 0 };
    
    expr_emit(value->left, stack_level, module_value, list_weak, result);
    expr_emit(value->right, stack_level + 1, module_value, list_weak, result);

    if (value->comb.comb == COMB_TYPE_INT)
    {
        bc.type = BYTECODE_OP_ADD_INT;
        bytecode_add(module_value->code, &bc);
    }
    else if (value->comb.comb == COMB_TYPE_FLOAT)
    {
        bc.type = BYTECODE_OP_ADD_FLOAT;
        bytecode_add(module_value->code, &bc);
    }
    else if (value->left->comb.comb == COMB_TYPE_STRING &&
             value->right->comb.comb == COMB_TYPE_STRING)
    {
        bc.type = BYTECODE_OP_ADD_STRING;
        bytecode_add(module_value->code, &bc);
    }
    else if (value->left->comb.comb == COMB_TYPE_INT &&
             value->right->comb.comb == COMB_TYPE_STRING)
    {
        bc.type = BYTECODE_OP_ADD_INT_STRING;
        bytecode_add(module_value->code, &bc);
    }
    else if (value->left->comb.comb == COMB_TYPE_STRING &&
             value->right->comb.comb == COMB_TYPE_INT)
    {
        bc.type = BYTECODE_OP_ADD_STRING_INT;
        bytecode_add(module_value->code, &bc);
    }
    else if (value->left->comb.comb == COMB_TYPE_FLOAT &&
             value->right->comb.comb == COMB_TYPE_STRING)
    {
        bc.type = BYTECODE_OP_ADD_FLOAT_STRING;
        bytecode_add(module_value->code, &bc);
    }
    else if (value->left->comb.comb == COMB_TYPE_STRING &&
             value->right->comb.comb == COMB_TYPE_FLOAT)
    {
        bc.type = BYTECODE_OP_ADD_STRING_FLOAT;
        bytecode_add(module_value->code, &bc);
    }
    else if (value->comb.comb == COMB_TYPE_ARRAY &&
             value->comb.comb_ret->type == PARAM_INT)
    {
        bc.type = BYTECODE_OP_ADD_ARR_INT;
        bytecode_add(module_value->code, &bc);
    }
    else if (value->comb.comb == COMB_TYPE_ARRAY &&
             value->comb.comb_ret->type == PARAM_FLOAT)
    {
        bc.type = BYTECODE_OP_ADD_ARR_FLOAT;
        bytecode_add(module_value->code, &bc);
    }
    else
    {
        *result = EMIT_FAIL;
        print_error_msg(value->line_no, "cannot add type %s\n",
                        comb_type_str(value->comb.comb));
        assert(0);
    }

    return 0;
}

int expr_sub_emit(expr * value, int stack_level, module * module_value,
                  func_list_weak * list_weak, int * result)
{
    bytecode bc = { 0 };

    expr_emit(value->left, stack_level, module_value, list_weak, result);
    expr_emit(value->right, stack_level + 1, module_value, list_weak, result);

    if (value->comb.comb == COMB_TYPE_INT)
    {
        bc.type = BYTECODE_OP_SUB_INT;
        bytecode_add(module_value->code, &bc);
    }
    else if (value->comb.comb == COMB_TYPE_FLOAT)
    {
        bc.type = BYTECODE_OP_SUB_FLOAT;
        bytecode_add(module_value->code, &bc);
    }
    else if (value->comb.comb == COMB_TYPE_ARRAY &&
             value->comb.comb_ret->type == PARAM_INT)
    {
        bc.type = BYTECODE_OP_SUB_ARR_INT;
        bytecode_add(module_value->code, &bc);
    }
    else if (value->comb.comb == COMB_TYPE_ARRAY &&
             value->comb.comb_ret->type == PARAM_FLOAT)
    {
        bc.type = BYTECODE_OP_SUB_ARR_FLOAT;
        bytecode_add(module_value->code, &bc);
    }
    else
    {
        *result = EMIT_FAIL;
        print_error_msg(value->line_no, "cannot sub type %s\n",
                        comb_type_str(value->comb.comb));
        assert(0);
    }

    return 0;
}

int expr_mul_emit(expr * value, int stack_level, module * module_value,
                  func_list_weak * list_weak, int * result)
{
    bytecode bc = { 0 };

    expr_emit(value->left, stack_level, module_value, list_weak, result);
    expr_emit(value->right, stack_level + 1, module_value, list_weak, result);

    if (value->comb.comb == COMB_TYPE_INT)
    {
        bc.type = BYTECODE_OP_MUL_INT;
        bytecode_add(module_value->code, &bc);
    }
    else if (value->comb.comb == COMB_TYPE_FLOAT)
    {
        bc.type = BYTECODE_OP_MUL_FLOAT;
        bytecode_add(module_value->code, &bc);
    }
    else if (value->comb.comb == COMB_TYPE_ARRAY &&
             value->left->comb.comb == COMB_TYPE_INT)
    {
        bc.type = BYTECODE_OP_MUL_ARR_INT;
        bytecode_add(module_value->code, &bc);
    }
    else if (value->comb.comb == COMB_TYPE_ARRAY &&
             value->left->comb.comb == COMB_TYPE_FLOAT)
    {
        bc.type = BYTECODE_OP_MUL_ARR_FLOAT;
        bytecode_add(module_value->code, &bc);
    }
    else if (value->comb.comb == COMB_TYPE_ARRAY &&
             value->comb.comb_ret->type == PARAM_INT &&
             value->left->comb.comb == COMB_TYPE_ARRAY &&
             value->right->comb.comb == COMB_TYPE_ARRAY)
    {
        bc.type = BYTECODE_OP_MUL_ARR_ARR_INT;
        bytecode_add(module_value->code, &bc);
    }
    else if (value->comb.comb == COMB_TYPE_ARRAY &&
             value->comb.comb_ret->type == PARAM_FLOAT &&
             value->left->comb.comb == COMB_TYPE_ARRAY &&
             value->right->comb.comb == COMB_TYPE_ARRAY)
    {
        bc.type = BYTECODE_OP_MUL_ARR_ARR_FLOAT;
        bytecode_add(module_value->code, &bc);
    }
    else
    {
        *result = EMIT_FAIL;
        print_error_msg(value->line_no, "cannot mul type %s\n",
                        comb_type_str(value->comb.comb));
        assert(0);
    }

    return 0;
}

int expr_and_emit(expr * value, int stack_level, module * module_value,
                  func_list_weak * list_weak, int * result)
{
    bytecode bc = { 0 };
    bytecode *condzA, *condzB, *jumpE;
    bytecode *labelEF, *labelE;

    expr_emit(value->left, stack_level, module_value, list_weak, result);

    bc.type = BYTECODE_JUMPZ;
    condzA = bytecode_add(module_value->code, &bc);

    expr_emit(value->right, stack_level, module_value, list_weak, result);

    bc.type = BYTECODE_JUMPZ;
    condzB = bytecode_add(module_value->code, &bc);

    bc.type = BYTECODE_INT;
    bc.integer.value = 1;
    bytecode_add(module_value->code, &bc);

    bc.type = BYTECODE_JUMP;
    jumpE = bytecode_add(module_value->code, &bc);

    bc.type = BYTECODE_LABEL;
    labelEF = bytecode_add(module_value->code, &bc);
    condzA->jump.offset = labelEF->addr - condzA->addr;
    condzB->jump.offset = labelEF->addr - condzB->addr;

    bc.type = BYTECODE_INT;
    bc.integer.value = 0;
    bytecode_add(module_value->code, &bc);

    bc.type = BYTECODE_LABEL;
    labelE = bytecode_add(module_value->code, &bc);
    jumpE->jump.offset = labelE->addr - jumpE->addr;

    return 0;
}

int expr_or_emit(expr * value, int stack_level, module * module_value,
                 func_list_weak * list_weak, int * result)
{
    bytecode bc = { 0 };
    bytecode *condzA, *condzB, *jumpET, *jumpE;
    bytecode *labelB, *labelET, *labelEF, *labelE;

    expr_emit(value->left, stack_level, module_value, list_weak, result);

    bc.type = BYTECODE_JUMPZ;
    condzA = bytecode_add(module_value->code, &bc);

    bc.type = BYTECODE_JUMP;
    jumpET = bytecode_add(module_value->code, &bc);

    bc.type = BYTECODE_LABEL;
    labelB = bytecode_add(module_value->code, &bc);
    condzA->jump.offset = labelB->addr - condzA->addr;

    expr_emit(value->right, stack_level, module_value, list_weak, result);

    bc.type = BYTECODE_JUMPZ;
    condzB = bytecode_add(module_value->code, &bc);

    bc.type = BYTECODE_LABEL;
    labelET = bytecode_add(module_value->code, &bc);
    jumpET->jump.offset = labelET->addr - jumpET->addr;

    bc.type = BYTECODE_INT;
    bc.integer.value = 1;
    bytecode_add(module_value->code, &bc);

    bc.type = BYTECODE_JUMP;
    jumpE = bytecode_add(module_value->code, &bc);

    bc.type = BYTECODE_LABEL;
    labelEF = bytecode_add(module_value->code, &bc);
    condzB->jump.offset = labelEF->addr - condzB->addr;

    bc.type = BYTECODE_INT;
    bc.integer.value = 0;
    bytecode_add(module_value->code, &bc);

    bc.type = BYTECODE_LABEL;
    labelE = bytecode_add(module_value->code, &bc);
    jumpE->jump.offset = labelE->addr - jumpE->addr;

    return 0;
}

int expr_not_emit(expr * value, int stack_level, module * module_value,
                  func_list_weak * list_weak, int * result)
{
    bytecode bc = { 0 };
    expr_emit(value->left, stack_level, module_value, list_weak, result);

    if (value->comb.comb == COMB_TYPE_INT)
    {
        bc.type = BYTECODE_OP_NOT_INT;
        bytecode_add(module_value->code, &bc);
    }
    else
    {
        *result = EMIT_FAIL;
        print_error_msg(value->line_no, "cannot not type %s\n",
                        comb_type_str(value->comb.comb));
        assert(0);
    }

    return 0;
}

int expr_ass_emit(expr * value, int stack_level, module * module_value,
                  func_list_weak * list_weak, int * result)
{
    bytecode bc = { 0 };
    
    expr_emit(value->left, stack_level, module_value, list_weak, result);
    expr_emit(value->right, stack_level + 1, module_value, list_weak, result);

    if ((value->left->comb.comb == COMB_TYPE_RECORD ||
         value->left->comb.comb == COMB_TYPE_RECORD_ID) &&
         value->right->comb.comb == COMB_TYPE_NIL)
    {
        bc.type = BYTECODE_OP_ASS_RECORD_NIL;
    }
    else if (value->comb.comb == COMB_TYPE_INT)
    {
        bc.type = BYTECODE_OP_ASS_INT;
    }
    else if (value->comb.comb == COMB_TYPE_FLOAT)
    {
        bc.type = BYTECODE_OP_ASS_FLOAT;
    }
    else if (value->comb.comb == COMB_TYPE_ENUMTYPE)
    {
        bc.type = BYTECODE_OP_ASS_INT;
    }
    else if (value->comb.comb == COMB_TYPE_STRING)
    {
        bc.type = BYTECODE_OP_ASS_STRING;
    }
    else if (value->comb.comb == COMB_TYPE_ARRAY)
    {
        bc.type = BYTECODE_OP_ASS_ARRAY;
    }
    else if (value->comb.comb == COMB_TYPE_FUNC)
    {
        bc.type = BYTECODE_OP_ASS_FUNC;
    }
    else if (value->comb.comb == COMB_TYPE_RECORD)
    {
        bc.type = BYTECODE_OP_ASS_RECORD;
    }
    else
    {
        *result = EMIT_FAIL;
        print_error_msg(value->line_no, "cannot assign type %s\n",
                        comb_type_str(value->comb.comb));
        assert(0);
    }

    bytecode_add(module_value->code, &bc);

    return 0;
}

int expr_cond_emit(expr * value, int stack_level, module * module_value,
                   func_list_weak * list_weak, int * result)
{
    bytecode bc = { 0 };
    bytecode *cond, *condz;
    bytecode *labelA, *labelB;

    expr_emit(value->left, stack_level, module_value, list_weak, result);

    bc.type = BYTECODE_JUMPZ;
    condz = bytecode_add(module_value->code, &bc);

    expr_emit(value->middle, stack_level, module_value, list_weak, result);

    bc.type = BYTECODE_JUMP;
    cond = bytecode_add(module_value->code, &bc);

    bc.type = BYTECODE_LABEL;
    labelA = bytecode_add(module_value->code, &bc);
    condz->jump.offset = labelA->addr - condz->addr;

    expr_emit(value->right, stack_level, module_value, list_weak, result);

    bc.type = BYTECODE_LABEL;
    labelB = bytecode_add(module_value->code, &bc);
    cond->jump.offset = labelB->addr - cond->addr;

    return 0;
}

int expr_while_emit(expr * value, int stack_level, module * module_value, 
                    func_list_weak * list_weak, int * result)
{
    bytecode bc = { 0 };
    bytecode *cond, *condz;
    bytecode *labelA, *labelB;
    
    bc.type = BYTECODE_LABEL;
    labelA = bytecode_add(module_value->code, &bc);
    
    expr_emit(value->whileloop.cond, stack_level, module_value, list_weak, result);
    
    bc.type = BYTECODE_JUMPZ;
    condz = bytecode_add(module_value->code, &bc);
    
    expr_emit(value->whileloop.do_value, stack_level, module_value, list_weak, result);
    
    /* pop previous value of stack */
    bc.type = BYTECODE_SLIDE;
    bc.slide.m = 0;
    bc.slide.q = 1;            
    bytecode_add(module_value->code, &bc);
    
    bc.type = BYTECODE_JUMP;
    cond = bytecode_add(module_value->code, &bc);
    cond->jump.offset = labelA->addr - cond->addr;
    
    bc.type = BYTECODE_LABEL;
    labelB = bytecode_add(module_value->code, &bc);
    condz->jump.offset = labelB->addr - condz->addr;

    /* while loop returns int 0 */
    bc.type = BYTECODE_INT;
    bc.integer.value = 0;
    bytecode_add(module_value->code, &bc);

    return 0;
}

int expr_do_while_emit(expr * value, int stack_level, module * module_value, 
                       func_list_weak * list_weak, int * result)
{
    bytecode bc = { 0 };
    bytecode *cond, *condz;
    bytecode *labelA, *labelB;
    
    bc.type = BYTECODE_LABEL;
    labelA = bytecode_add(module_value->code, &bc);

    expr_emit(value->whileloop.do_value, stack_level, module_value, list_weak, result);
    
    /* pop previous value of stack */
    bc.type = BYTECODE_SLIDE;
    bc.slide.m = 0;
    bc.slide.q = 1;            
    bytecode_add(module_value->code, &bc);

    expr_emit(value->whileloop.cond, stack_level, module_value, list_weak, result);
    
    bc.type = BYTECODE_JUMPZ;
    condz = bytecode_add(module_value->code, &bc);

    bc.type = BYTECODE_JUMP;
    cond = bytecode_add(module_value->code, &bc);
    cond->jump.offset = labelA->addr - cond->addr;

    bc.type = BYTECODE_LABEL;
    labelB = bytecode_add(module_value->code, &bc);
    condz->jump.offset = labelB->addr - condz->addr;

    /* do while loop returns int 0 */
    bc.type = BYTECODE_INT;
    bc.integer.value = 0;
    bytecode_add(module_value->code, &bc);

    return 0;
}

int expr_for_emit(expr * value, int stack_level, module * module_value, 
                  func_list_weak * list_weak, int * result)
{
    bytecode bc = { 0 };
    bytecode *cond, *condz;
    bytecode *labelA, *labelB;

    expr_emit(value->forloop.init, stack_level, module_value, list_weak, result);
    
    /* pop previous value of stack */
    bc.type = BYTECODE_SLIDE;
    bc.slide.m = 0;
    bc.slide.q = 1;            
    bytecode_add(module_value->code, &bc);

    bc.type = BYTECODE_LABEL;
    labelA = bytecode_add(module_value->code, &bc);

    expr_emit(value->forloop.cond, stack_level, module_value, list_weak, result);
    
    bc.type = BYTECODE_JUMPZ;
    condz = bytecode_add(module_value->code, &bc);
        
    expr_emit(value->forloop.do_value, stack_level, module_value, list_weak, result);

    /* pop previous value of stack */
    bc.type = BYTECODE_SLIDE;
    bc.slide.m = 0;
    bc.slide.q = 1;            
    bytecode_add(module_value->code, &bc);

    expr_emit(value->forloop.incr, stack_level, module_value, list_weak, result);    

    /* pop previous value of stack */
    bc.type = BYTECODE_SLIDE;
    bc.slide.m = 0;
    bc.slide.q = 1;            
    bytecode_add(module_value->code, &bc);

    bc.type = BYTECODE_JUMP;
    cond = bytecode_add(module_value->code, &bc);
    cond->jump.offset = labelA->addr - cond->addr;

    bc.type = BYTECODE_LABEL;
    labelB = bytecode_add(module_value->code, &bc);
    condz->jump.offset = labelB->addr - condz->addr;

    /* for loop returns int 0 */
    bc.type = BYTECODE_INT;
    bc.integer.value = 0;
    bytecode_add(module_value->code, &bc);

    return 0;
}

int expr_call_emit(expr * value, int stack_level, module * module_value,
                   func_list_weak * list_weak, int * result)
{
    bytecode bc = { 0 };
    bytecode *mark, *label;

    bc.type = BYTECODE_LINE;
    bc.line.no = value->line_no;
    bytecode_add(module_value->code, &bc);

    bc.type = BYTECODE_MARK;
    mark = bytecode_add(module_value->code, &bc);

    int v = NUM_FRAME_PTRS;
    if (value->call.params)
    {
        expr_list_emit(value->call.params, stack_level + v, module_value, list_weak, result);
        v += value->call.params->count;
    }
    expr_emit(value->call.func_expr, stack_level + v, module_value, list_weak, result);

    bc.type = BYTECODE_CALL;
    bytecode_add(module_value->code, &bc);

    bc.type = BYTECODE_LABEL;
    label = bytecode_add(module_value->code, &bc);
    mark->mark.addr = label->addr;

    return 0;
}

int expr_last_call_emit(expr * value, int stack_level, module * module_value,
                        func_list_weak * list_weak, int * result)
{
    int v = 0;
    bytecode bc = { 0 };

    if (value->call.params)
    {
        expr_list_emit(value->call.params, stack_level + v, module_value, list_weak, result);
        v += value->call.params->count;
    }
    expr_emit(value->call.func_expr, stack_level + v, module_value, list_weak, result);

    bc.type = BYTECODE_SLIDE;
    bc.slide.q = stack_level + v;
    bc.slide.m = v + 1;
    bytecode_add(module_value->code, &bc);

    bc.type = BYTECODE_CALL;
    bytecode_add(module_value->code, &bc);

    return 0;
}

int expr_record_emit(expr * value, int stack_level, module * module_value,
                     func_list_weak * list_weak, int * result)
{
    int count = -1;
    bytecode bc = { 0 };
    
    if (value->call.params != NULL)
    {
        count = value->call.params->count;
        expr_list_emit(value->call.params, stack_level, module_value,
                       list_weak, result);
    }

    assert(count >= 0);

    bc.type = BYTECODE_RECORD;
    bc.record.count = count;
    
    bytecode_add(module_value->code, &bc);
    
    return 0;
}

int expr_emit(expr * value, int stack_level, module * module_value,
              func_list_weak * list_weak, int * result)
{
    bytecode bc = { 0 };

    switch (value->type)
    {
    case EXPR_INT:
        expr_int_emit(value, stack_level, module_value, result);
        break;
    case EXPR_FLOAT:
        expr_float_emit(value, stack_level, module_value, result);
        break;
    case EXPR_STRING:
        expr_string_emit(value, stack_level, module_value, result);
        break;
    case EXPR_NIL:
        expr_nil_emit(value, stack_level, module_value, result);
        break;
    case EXPR_ID:
        expr_id_emit(value, stack_level, module_value, result);
        break;
    case EXPR_NEG:
        expr_neg_emit(value, stack_level, module_value, list_weak, result);
        break;
    case EXPR_ADD:
        expr_add_emit(value, stack_level, module_value, list_weak, result);
        break;
    case EXPR_SUB:
        expr_sub_emit(value, stack_level, module_value, list_weak, result);
        break;
    case EXPR_MUL:
        expr_mul_emit(value, stack_level, module_value, list_weak, result);    
        break;
    case EXPR_DIV:
        expr_emit(value->left, stack_level, module_value, list_weak, result);
        expr_emit(value->right, stack_level + 1, module_value, list_weak, result);

        bc.type = BYTECODE_LINE;
        bc.line.no = value->line_no;
        bytecode_add(module_value->code, &bc);

        if (value->comb.comb == COMB_TYPE_INT)
        {
            bc.type = BYTECODE_OP_DIV_INT;
            bytecode_add(module_value->code, &bc);
        }
        else if (value->comb.comb == COMB_TYPE_FLOAT)
        {
            bc.type = BYTECODE_OP_DIV_FLOAT;
            bytecode_add(module_value->code, &bc);
        }
        else
        {
            *result = EMIT_FAIL;
            print_error_msg(value->line_no, "cannot div type %s\n",
                            comb_type_str(value->comb.comb));
            assert(0);
        }
        break;
    case EXPR_MOD:
        expr_emit(value->left, stack_level, module_value, list_weak, result);
        expr_emit(value->right, stack_level + 1, module_value, list_weak, result);

        bc.type = BYTECODE_LINE;
        bc.line.no = value->line_no;
        bytecode_add(module_value->code, &bc);

        if (value->left->comb.comb == COMB_TYPE_INT &&
            value->right->comb.comb == COMB_TYPE_INT)
        {
            bc.type = BYTECODE_OP_MOD_INT;
            bytecode_add(module_value->code, &bc);
        }
        else
        {
            *result = EMIT_FAIL;
            print_error_msg(value->line_no, "cannot mod type %s %s\n",
                            comb_type_str(value->left->comb.comb),
                            comb_type_str(value->right->comb.comb));
            assert(0);
        }
        break;
    case EXPR_LT:
        expr_emit(value->left, stack_level, module_value, list_weak, result);
        expr_emit(value->right, stack_level + 1, module_value, list_weak, result);

        if (value->left->comb.comb == COMB_TYPE_INT &&
            value->right->comb.comb == COMB_TYPE_INT)
        {
            bc.type = BYTECODE_OP_LT_INT;
            bytecode_add(module_value->code, &bc);
        }
        else if (value->left->comb.comb == COMB_TYPE_FLOAT &&
                 value->right->comb.comb == COMB_TYPE_FLOAT)
        {
            bc.type = BYTECODE_OP_LT_FLOAT;
            bytecode_add(module_value->code, &bc);
        }
        else
        {
            *result = EMIT_FAIL;
            print_error_msg(value->line_no, "cannot lt different types %s %s\n",
                            comb_type_str(value->left->comb.comb),
                            comb_type_str(value->right->comb.comb));
            assert(0);
        }
        break;
    case EXPR_GT:
        expr_emit(value->left, stack_level, module_value, list_weak, result);
        expr_emit(value->right, stack_level + 1, module_value, list_weak, result);

        if (value->left->comb.comb == COMB_TYPE_INT &&
            value->right->comb.comb == COMB_TYPE_INT)
        {
            bc.type = BYTECODE_OP_GT_INT;
            bytecode_add(module_value->code, &bc);
        }
        else if (value->left->comb.comb == COMB_TYPE_FLOAT &&
                 value->right->comb.comb == COMB_TYPE_FLOAT)
        {
            bc.type = BYTECODE_OP_GT_FLOAT;
            bytecode_add(module_value->code, &bc);
        }
        else
        {
            *result = EMIT_FAIL;
            print_error_msg(value->line_no, "cannot gt different types %s %s\n",
                            comb_type_str(value->left->comb.comb),
                            comb_type_str(value->right->comb.comb));
            assert(0);
        }
        break;
    case EXPR_LTE:
        expr_emit(value->left, stack_level, module_value, list_weak, result);
        expr_emit(value->right, stack_level + 1, module_value, list_weak, result);

        if (value->left->comb.comb == COMB_TYPE_INT &&
            value->right->comb.comb == COMB_TYPE_INT)
        {
            bc.type = BYTECODE_OP_LTE_INT;
            bytecode_add(module_value->code, &bc);
        }
        else if (value->left->comb.comb == COMB_TYPE_FLOAT &&
                 value->right->comb.comb == COMB_TYPE_FLOAT)
        {
            bc.type = BYTECODE_OP_LTE_FLOAT;
            bytecode_add(module_value->code, &bc);
        }
        else
        {
            *result = EMIT_FAIL;
            print_error_msg(value->line_no,
                            "cannot lte different types %s %s\n",
                            comb_type_str(value->left->comb.comb),
                            comb_type_str(value->right->comb.comb));
            assert(0);
        }
        break;
    case EXPR_GTE:
        expr_emit(value->left, stack_level, module_value, list_weak, result);
        expr_emit(value->right, stack_level + 1, module_value, list_weak, result);

        if (value->left->comb.comb == COMB_TYPE_INT &&
            value->right->comb.comb == COMB_TYPE_INT)
        {
            bc.type = BYTECODE_OP_GTE_INT;
            bytecode_add(module_value->code, &bc);
        }
        else if (value->left->comb.comb == COMB_TYPE_FLOAT &&
                 value->right->comb.comb == COMB_TYPE_FLOAT)
        {
            bc.type = BYTECODE_OP_GTE_FLOAT;
            bytecode_add(module_value->code, &bc);
        }
        else
        {
            *result = EMIT_FAIL;
            print_error_msg(value->line_no,
                            "cannot gte different types %s %s\n",
                            comb_type_str(value->left->comb.comb),
                            comb_type_str(value->right->comb.comb));
            assert(0);
        }
        break;
    case EXPR_EQ:
        expr_emit(value->left, stack_level, module_value, list_weak, result);
        expr_emit(value->right, stack_level + 1, module_value, list_weak, result);

        if (value->left->comb.comb == COMB_TYPE_NIL &&
            value->right->comb.comb == COMB_TYPE_NIL)
        {
            bc.type = BYTECODE_OP_EQ_NIL;
        }
        else if (value->left->comb.comb == COMB_TYPE_INT &&
                 value->right->comb.comb == COMB_TYPE_INT)
        {
            bc.type = BYTECODE_OP_EQ_INT;
        }
        else if (value->left->comb.comb == COMB_TYPE_FLOAT &&
                 value->right->comb.comb == COMB_TYPE_FLOAT)
        {
            bc.type = BYTECODE_OP_EQ_FLOAT;
        }
        else if (value->left->comb.comb == COMB_TYPE_ENUMTYPE &&
                 value->right->comb.comb == COMB_TYPE_ENUMTYPE)
        {
            bc.type = BYTECODE_OP_EQ_INT;
        }
        else if (value->left->comb.comb == COMB_TYPE_STRING &&
                 value->right->comb.comb == COMB_TYPE_STRING)
        {
            bc.type = BYTECODE_OP_EQ_STRING;
        }
        else if (value->left->comb.comb == COMB_TYPE_STRING &&
                 value->right->comb.comb == COMB_TYPE_NIL)
        {
            bc.type = BYTECODE_OP_EQ_STRING_NIL;
        }
        else if (value->left->comb.comb == COMB_TYPE_NIL &&
                 value->right->comb.comb == COMB_TYPE_STRING)
        {
            bc.type = BYTECODE_OP_EQ_NIL_STRING;
        }
        else if (value->left->comb.comb == COMB_TYPE_ARRAY &&
                 value->right->comb.comb == COMB_TYPE_NIL)
        {
            bc.type = BYTECODE_OP_EQ_ARRAY_NIL;
        }
        else if (value->left->comb.comb == COMB_TYPE_NIL &&
                 value->right->comb.comb == COMB_TYPE_ARRAY)
        {
            bc.type = BYTECODE_OP_EQ_NIL_ARRAY;
        }
        else if ((value->left->comb.comb == COMB_TYPE_RECORD ||
                  value->left->comb.comb == COMB_TYPE_RECORD_ID) &&
                  value->right->comb.comb == COMB_TYPE_NIL)
        {
            bc.type = BYTECODE_OP_EQ_RECORD_NIL;
        }
        else if (value->left->comb.comb == COMB_TYPE_NIL &&
                (value->right->comb.comb == COMB_TYPE_RECORD ||
                 value->right->comb.comb == COMB_TYPE_RECORD_ID))
        {
            bc.type = BYTECODE_OP_EQ_NIL_RECORD;
        }
        else if (value->left->comb.comb == COMB_TYPE_FUNC &&
                 value->right->comb.comb == COMB_TYPE_NIL)
        {
            bc.type = BYTECODE_OP_EQ_FUNC_NIL;
        }
        else if (value->left->comb.comb == COMB_TYPE_NIL &&
                 value->right->comb.comb == COMB_TYPE_FUNC)
        {
            bc.type = BYTECODE_OP_EQ_NIL_FUNC;
        }
        else
        {
            *result = EMIT_FAIL;
            print_error_msg(value->line_no, "cannot eq different types %s %s\n",
                            comb_type_str(value->left->comb.comb),
                            comb_type_str(value->right->comb.comb));
            assert(0);
        }
        bytecode_add(module_value->code, &bc);
        break;
    case EXPR_NEQ:
        expr_emit(value->left, stack_level, module_value, list_weak, result);
        expr_emit(value->right, stack_level + 1, module_value, list_weak, result);

        if (value->left->comb.comb == COMB_TYPE_NIL &&
            value->right->comb.comb == COMB_TYPE_NIL)
        {
            bc.type = BYTECODE_OP_NEQ_NIL;
        }
        else if (value->left->comb.comb == COMB_TYPE_INT &&
                 value->right->comb.comb == COMB_TYPE_INT)
        {
            bc.type = BYTECODE_OP_NEQ_INT;
        }
        else if (value->left->comb.comb == COMB_TYPE_FLOAT &&
                 value->right->comb.comb == COMB_TYPE_FLOAT)
        {
            bc.type = BYTECODE_OP_NEQ_FLOAT;
        }
        else if (value->left->comb.comb == COMB_TYPE_ENUMTYPE &&
                 value->right->comb.comb == COMB_TYPE_ENUMTYPE)
        {
            bc.type = BYTECODE_OP_NEQ_INT;
        }
        else if (value->left->comb.comb == COMB_TYPE_STRING &&
                 value->right->comb.comb == COMB_TYPE_STRING)
        {
            bc.type = BYTECODE_OP_NEQ_STRING;
        }
        else if (value->left->comb.comb == COMB_TYPE_STRING &&
                 value->right->comb.comb == COMB_TYPE_NIL)
        {
            bc.type = BYTECODE_OP_NEQ_STRING_NIL;
        }
        else if (value->left->comb.comb == COMB_TYPE_NIL &&
                 value->right->comb.comb == COMB_TYPE_STRING)
        {
            bc.type = BYTECODE_OP_NEQ_NIL_STRING;
        }
        else if (value->left->comb.comb == COMB_TYPE_ARRAY &&
                 value->right->comb.comb == COMB_TYPE_NIL)
        {
            bc.type = BYTECODE_OP_NEQ_ARRAY_NIL;
        }
        else if (value->left->comb.comb == COMB_TYPE_NIL &&
                 value->right->comb.comb == COMB_TYPE_ARRAY)
        {
            bc.type = BYTECODE_OP_NEQ_NIL_ARRAY;
        }
        else if ((value->left->comb.comb == COMB_TYPE_RECORD ||
                  value->left->comb.comb == COMB_TYPE_RECORD_ID) &&
                  value->right->comb.comb == COMB_TYPE_NIL)
        {
            bc.type = BYTECODE_OP_NEQ_RECORD_NIL;
        }
        else if (value->left->comb.comb == COMB_TYPE_NIL &&
                (value->right->comb.comb == COMB_TYPE_RECORD ||
                 value->right->comb.comb == COMB_TYPE_RECORD_ID))
        {
            bc.type = BYTECODE_OP_NEQ_NIL_RECORD;
        }
        else if (value->left->comb.comb == COMB_TYPE_FUNC &&
                 value->right->comb.comb == COMB_TYPE_NIL)
        {
            bc.type = BYTECODE_OP_NEQ_FUNC_NIL;
        }
        else if (value->left->comb.comb == COMB_TYPE_NIL &&
                 value->right->comb.comb == COMB_TYPE_FUNC)
        {
            bc.type = BYTECODE_OP_NEQ_NIL_FUNC;
        }
        else
        {
            *result = EMIT_FAIL;
            print_error_msg(value->line_no,
                            "cannot neq different types %s %s\n",
                            comb_type_str(value->left->comb.comb),
                            comb_type_str(value->right->comb.comb));
            assert(0);
        }
        bytecode_add(module_value->code, &bc);
        break;
    case EXPR_AND:
        expr_and_emit(value, stack_level, module_value, list_weak, result);
        break;
    case EXPR_OR:
        expr_or_emit(value, stack_level, module_value, list_weak, result);
        break;
    case EXPR_NOT:
        expr_not_emit(value, stack_level, module_value, list_weak, result);
        break;
    case EXPR_SUP:
        expr_emit(value->left, stack_level, module_value, list_weak, result);
        break;
    case EXPR_COND:
        expr_cond_emit(value, stack_level, module_value, list_weak, result);
        break;
    case EXPR_ARRAY:
        if (value->array.array_value != NULL)
        {
            expr_array_emit(value, stack_level, module_value, list_weak, result);
        }
        break;
    case EXPR_ARRAY_DEREF:
        expr_array_deref_emit(value, stack_level, module_value, list_weak, result);
        break;
    case EXPR_CALL:
        if (value->call.func_expr->comb.comb == COMB_TYPE_RECORD_ID)
        {
            expr_record_emit(value, stack_level, module_value, list_weak, result);
        }
        else
        {
            expr_call_emit(value, stack_level, module_value, list_weak, result);
        }
        break;
    case EXPR_LAST_CALL:
        expr_last_call_emit(value, stack_level, module_value, list_weak, result);
        break;
    case EXPR_FUNC:
        if (value->func_value)
        {
            func_emit(value->func_value, stack_level, module_value, list_weak, result);
        }
        break;
    case EXPR_SEQ:
        if (value->seq.list != NULL)
        {
            expr_seq_emit(value->seq.list, stack_level, module_value, list_weak, result);
        }
        break;
    case EXPR_ASS:
        expr_ass_emit(value, stack_level, module_value, list_weak, result);
        break;
    case EXPR_WHILE:
        expr_while_emit(value, stack_level, module_value, list_weak, result);
        break;
    case EXPR_DO_WHILE:
        expr_do_while_emit(value, stack_level, module_value, list_weak, result);
        break;
    case EXPR_FOR:
        expr_for_emit(value, stack_level, module_value, list_weak, result);
        break;
    case EXPR_BUILD_IN:
        expr_list_emit(value->func_build_in.param, stack_level, module_value,
                       list_weak, result);

        bc.type = BYTECODE_BUILD_IN;
        bc.build_in.id = value->func_build_in.id;
        bytecode_add(module_value->code, &bc);
        break;
    case EXPR_INT_TO_FLOAT:
        expr_emit(value->left, stack_level, module_value, list_weak, result);

        if (value->left->comb.comb == COMB_TYPE_INT)
        {
            bc.type = BYTECODE_INT_TO_FLOAT;
            bytecode_add(module_value->code, &bc);
        }
        else
        {
            *result = EMIT_FAIL;
            print_error_msg(value->line_no, "cannot convert type %s to float\n",
                            comb_type_str(value->left->comb.comb));
            assert(0);
        }

        break;
    case EXPR_FLOAT_TO_INT:
        expr_emit(value->left, stack_level, module_value, list_weak, result);

        if (value->left->comb.comb == COMB_TYPE_FLOAT)
        {
            bc.type = BYTECODE_FLOAT_TO_INT;
            bytecode_add(module_value->code, &bc);
        }
        else
        {
            *result = EMIT_FAIL;
            print_error_msg(value->line_no, "cannot convert type %s to int\n",
                            comb_type_str(value->left->comb.comb));
            assert(0);
        }
        break;
    case EXPR_LISTCOMP:
        if (value->listcomp_value != NULL)
        {
            listcomp_emit(value->listcomp_value, stack_level, module_value,
                          list_weak, result);
        }
        break;
    case EXPR_ATTR:
        if (value->attr.record_value->comb.comb == COMB_TYPE_RECORD ||
            value->attr.record_value->comb.comb == COMB_TYPE_RECORD_ID)
        {
            expr_record_attr_emit(value, stack_level, module_value, list_weak, result);
        }
        else if (value->attr.record_value->comb.comb == COMB_TYPE_ENUMTYPE_ID)
        {
            expr_enumtype_attr_emit(value, stack_level, module_value, list_weak, result);
        }
        else
        {
            assert(0);
        }
        break;
    }
    return 0;
}

int expr_list_emit(expr_list * list, int stack_level, module * module_value,
                   func_list_weak * list_weak, int * result)
{
    int e = 0;
    expr_list_node * node = list->head;
    while (node != NULL)
    {
        expr * value = node->value;
        if (value != NULL)
        {
            expr_emit(value, stack_level + e++, module_value, list_weak, result);
        }
        node = node->prev;
    }

    return 0;
}

int expr_seq_emit(expr_list * list, int stack_level, module * module_value,
                 func_list_weak * list_weak,  int * result)
{
    int prev = 0;

    expr_list_node * node = list->tail;
    while (node != NULL)
    {
        if (prev)
        {
            bytecode bc = { 0 };

            /* pop previous value of stack */
            bc.type = BYTECODE_SLIDE;
            bc.slide.m = 0;
            bc.slide.q = 1;            

            bytecode_add(module_value->code, &bc);
        }
        prev = 1;
    
        expr * value = node->value;
        if (value != NULL)
        {
            expr_emit(value, stack_level, module_value, list_weak, result);
        }
        node = node->next;
    }

    return 0;
}

int expr_id_qualifier_emit(expr * value, int stack_level, module * module_value,
                           int * result)
{
    bytecode bc = { 0 };

    bc.type = BYTECODE_ID_LOCAL;
    bc.id_local.stack_level = stack_level - value->id.id_qualifier_value->stack_level - 3;
    bc.id_local.index = 0;

    bytecode_add(module_value->code, &bc);

    return 0;
}

int expr_yeld_emit(listcomp * listcomp_value, int stack_level, module * module_value,
                   func_list_weak * list_weak, int * result)
{
    bytecode bc = { 0 };

    expr_emit(listcomp_value->expr_value, stack_level, module_value, list_weak, result);

    /* append value to the list */
    bc.type = BYTECODE_ARRAY_APPEND;
    bc.id_local.stack_level = stack_level - listcomp_value->stack_level;
    bc.id_local.index = 0;
    bytecode_add(module_value->code, &bc);

    return 0;
}

int generator_emit(listcomp * listcomp_value, qualifier_list_node * node,
                   int stack_level, module * module_value,
                   func_list_weak * list_weak, int * result)
{
    bytecode bc = { 0 };
    bytecode * labelA, * labelE;
    bytecode * cond, * condz;
    qualifier * value = node->value;
    value->stack_level = stack_level;
    
    expr_emit(value->expr_value, stack_level, module_value,
              list_weak, result);

    /* loop counter */
    bc.type = BYTECODE_INT;
    bc.integer.value = 0;
    bytecode_add(module_value->code, &bc);

    bc.type = BYTECODE_LABEL;
    labelA = bytecode_add(module_value->code, &bc);

    /* loop all elements */
    bc.type = BYTECODE_ID_LOCAL;
    bc.id_local.stack_level = 0;
    bc.id_local.index = 0;
    bytecode_add(module_value->code, &bc);    

    bc.type = BYTECODE_ID_DIM_LOCAL;
    bc.id_dim_local.stack_level = 2; 
    bc.id_dim_local.index = 0;
    bc.id_dim_local.dim_index = 0; 
    bytecode_add(module_value->code, &bc);

    bc.type = BYTECODE_OP_LT_INT;
    bytecode_add(module_value->code, &bc);
    
    bc.type = BYTECODE_JUMPZ;
    condz = bytecode_add(module_value->code, &bc);

    /* push value */
    bc.type = BYTECODE_ID_LOCAL;
    bc.id_local.stack_level = 1;
    bc.id_local.index = 0;
    bytecode_add(module_value->code, &bc);
    
    bc.type = BYTECODE_ID_LOCAL;
    bc.id_local.stack_level = 1;
    bc.id_local.index = 0;
    bytecode_add(module_value->code, &bc);

    bc.type = BYTECODE_ARRAY_DEREF;
    bc.array_deref.dims = 1;
    bytecode_add(module_value->code, &bc);

    qualifier_stack_emit(listcomp_value, node->next, stack_level + 3, module_value,
                         list_weak, result);

    /* pop value */
    bc.type = BYTECODE_SLIDE;
    bc.slide.m = 0;
    bc.slide.q = 1;
    bytecode_add(module_value->code, &bc);

    /* inc loop counter */
    bc.type = BYTECODE_OP_INC_INT;
    bc.id_local.stack_level = 0;
    bc.id_local.index = 0;
    bytecode_add(module_value->code, &bc);

    /* jump to beginning */
    bc.type = BYTECODE_JUMP;
    cond = bytecode_add(module_value->code, &bc);
    cond->jump.offset = labelA->addr - cond->addr;

    /* loop end */
    bc.type = BYTECODE_LABEL;
    labelE = bytecode_add(module_value->code, &bc);
    condz->jump.offset = labelE->addr - condz->addr;

    /* pop list and counter */
    bc.type = BYTECODE_SLIDE;
    bc.slide.m = 0;
    bc.slide.q = 2;
    bytecode_add(module_value->code, &bc);

    return 0;
}

int filter_emit(listcomp * listcomp_value, qualifier_list_node * node,
                int stack_level, module * module_value,
                func_list_weak * list_weak, int * result)
{
    bytecode bc = { 0 };
    bytecode * labelE, * condz;
    qualifier * value = node->value;
    value->stack_level = stack_level;

    expr_emit(value->expr_value, stack_level, module_value,
              list_weak, result);

    bc.type = BYTECODE_JUMPZ;
    condz = bytecode_add(module_value->code, &bc);
    
    qualifier_stack_emit(listcomp_value, node->next, stack_level, module_value,
                         list_weak, result);

    bc.type = BYTECODE_LABEL;
    labelE = bytecode_add(module_value->code, &bc);
    condz->jump.offset = labelE->addr - condz->addr;     
    
    return 0;
}

int qualifier_emit(listcomp * listcomp_value, qualifier_list_node * node,
                   int stack_level, module * module_value,
                   func_list_weak * list_weak, int * result)
{
    qualifier * value = node->value;

    switch (value->type)
    {
        case QUALIFIER_GENERATOR:
            generator_emit(listcomp_value, node, stack_level, module_value, list_weak, result);
        break;
        case QUALIFIER_FILTER:
            filter_emit(listcomp_value, node, stack_level, module_value, list_weak, result);
        break;
        case QUALIFIER_UNKNOWN:
            assert(0);
        break;
    }

    return 0;
}

int qualifier_stack_emit(listcomp * listcomp_value, qualifier_list_node * node,
                         int stack_level, module * module_value,
                         func_list_weak * list_weak, int * result)
{
    if (node != NULL)
    {
        qualifier * value = node->value;
        if (value != NULL)
        {
            qualifier_emit(listcomp_value, node, stack_level, module_value,
                           list_weak, result);
        }
    }
    else
    {
        if (listcomp_value->expr_value != NULL)
        {
            expr_yeld_emit(listcomp_value, stack_level, module_value, list_weak, result);
        }
    }

    return 0;
}                         
                 
int listcomp_emit(listcomp * listcomp_value, int stack_level, module * module_value,
                  func_list_weak * list_weak, int * result)
{
    bytecode bc = { 0 };
    listcomp_value->stack_level = stack_level;

    bc.type = BYTECODE_INT;
    bc.integer.value = 0;
    bytecode_add(module_value->code, &bc);    

    bc.type = BYTECODE_MK_INIT_ARRAY;
    bc.mk_array.dims = 1;
    bytecode_add(module_value->code, &bc);

    qualifier_stack_emit(listcomp_value, listcomp_value->list->tail, stack_level + 1,
                         module_value, list_weak, result);
    return 0;
}                  

int array_dims_emit(array * array_value, int stack_level, module * module_value,
                    func_list_weak * list_weak, int * result)
{
    bytecode bc = { 0 };
    
    expr_list_emit(array_value->dims, stack_level, module_value, list_weak, result);

    if (array_value->ret->type == PARAM_INT)
    {
        bc.type = BYTECODE_MK_ARRAY_INT;
    }
    else if (array_value->ret->type == PARAM_FLOAT)
    {
        bc.type = BYTECODE_MK_ARRAY_FLOAT;
    }
    else if (array_value->ret->type == PARAM_STRING)
    {
        bc.type = BYTECODE_MK_ARRAY_STRING;
    }
    else if (array_value->ret->type == PARAM_ENUMTYPE)
    {
        bc.type = BYTECODE_MK_ARRAY_INT;
    }
    else if (array_value->ret->type == PARAM_ARRAY)
    {
        bc.type = BYTECODE_MK_ARRAY_ARRAY;
    }
    else if (array_value->ret->type == PARAM_RECORD)
    {
        bc.type = BYTECODE_MK_ARRAY_RECORD;
    }
    else if (array_value->ret->type == PARAM_FUNC)
    {
        bc.type = BYTECODE_MK_ARRAY_FUNC;
    }
    else
    {
        assert(0);
    }

    bc.mk_array.dims = array_value->dims->count;
    bytecode_add(module_value->code, &bc);

    return 0;
}

int array_init_elements_emit(expr_list_weak * depth_list, int * elements_count,
                             int stack_level, module * module_value,
                             func_list_weak * list_weak, int * result)
{
    int elem_dist = -1;
    *elements_count = 0;

    expr_list_weak_node * node = depth_list->tail;
    elem_dist = node->distance;

    while (node != NULL)
    {
        expr * value = node->value;
        if (value != NULL && node->distance == elem_dist)
        {
            expr_emit(value, stack_level + (*elements_count)++, module_value,
                      list_weak, result);
        }
        else
        {
            break;
        }
        node = node->next;
    }

    return 0;
}

int array_init_emit(expr * value, int stack_level, module * module_value,
                    func_list_weak * list_weak, int * result)
{
    bytecode bc = { 0 };
    int elements_count = 0;
    array * array_value = value->array.array_value;

    expr_list_weak * depth_list = expr_list_weak_new();
    array_to_depth_list(value, depth_list);

    array_init_elements_emit(depth_list, &elements_count, stack_level, module_value,
                             list_weak, result);
    expr_list_weak_delete(depth_list);

    expr_list_emit(array_value->dims, stack_level + elements_count, module_value,
                   list_weak, result);

    bc.type = BYTECODE_MK_INIT_ARRAY;
    bc.mk_array.dims = array_value->dims->count;
    bytecode_add(module_value->code, &bc);

    return 0;
}

int expr_array_emit(expr * value, int stack_level, module * module_value,
                    func_list_weak * list_weak, int * result)
{
    array * array_value = value->array.array_value;

    if (array_value->type == ARRAY_INIT)
    {
        array_init_emit(value, stack_level, module_value, list_weak, result);
    }
    else if (array_value->type == ARRAY_SUB)
    {
        assert(0);
    }
    else if (array_value->type == ARRAY_DIMS)
    {
        array_dims_emit(array_value, stack_level, module_value, list_weak, result);
    }

    return 0;
}

int expr_array_deref_emit(expr * value, int stack_level, module * module_value,
                          func_list_weak * list_weak, int * result)
{
    bytecode bc = { 0 };

    expr_emit(value->array_deref.array_expr, stack_level, module_value, list_weak, result);
    expr_list_emit(value->array_deref.ref, stack_level + 1, module_value, list_weak, result);

    bc.type = BYTECODE_ARRAY_DEREF;
    bc.array_deref.dims = value->array_deref.ref->count;

    bytecode_add(module_value->code, &bc);

    return 0;
}

int expr_enumtype_attr_emit(expr * value, int stack_level, module * module_value,
                            func_list_weak * list_weak, int * result)
{
    bytecode bc = { 0 };
    int index = -1;    
    
    if (value->attr.id_enumerator_value != NULL)
    {
        index = value->attr.id_enumerator_value->index;
    }
    assert(index != -1);
    
    bc.type = BYTECODE_INT;
    bc.integer.value = index;

    bytecode_add(module_value->code, &bc);

    return 0;
}

int expr_record_attr_emit(expr * value, int stack_level, module * module_value,
                          func_list_weak * list_weak, int * result)
{
    bytecode bc = { 0 };
    int index = -1;
    
    expr_emit(value->attr.record_value, stack_level, module_value, list_weak, result);
    
    if (value->attr.id_param_value != NULL)
    {
        index = value->attr.id_param_value->index;
    }
    assert(index != -1);
    
    bc.type = BYTECODE_ATTR;
    bc.attr.index = index;
    
    bytecode_add(module_value->code, &bc);

    return 0;
}                   

int bind_emit(bind * bind_value, int stack_level, module * module_value,
              func_list_weak * list_weak, int * result)
{
    switch (bind_value->type)
    {
        case BIND_UNKNOWN:
            fprintf(stderr, "unknown bind type\n");
            assert(0);
        break;
        case BIND_LET:
        case BIND_VAR:
            if (bind_value->expr_value != NULL)
            {
                expr_emit(bind_value->expr_value, stack_level, module_value, list_weak, result);
            }
        break;
    }
    return 0;
}

int bind_list_emit(bind_list * list, int stack_level, module * module_value,
                   func_list_weak * list_weak, int * result)
{
    int b = 0;
    
    bind_list_node * node = list->tail;
    while (node != NULL)
    {
        bind * bind_value = node->value;
        if (bind_value != NULL)
        {
            bind_emit(bind_value, stack_level + b++, module_value, list_weak, result);
        }
        node = node->next;
    }
    return 0;
}

int except_implicit_emit(func * func_value, int stack_level, module * module_value,
                         func_list_weak * list_weak, int * result)
{
    bytecode bc = { 0 };
    
    bc.type = BYTECODE_RETHROW;
    bytecode_add(module_value->code, &bc);

    return 0;
}
    
int except_all_emit(except * value, func * func_value, int stack_level,
                    module * module_value, func_list_weak * list_weak, int * result)
{
    bytecode bc = { 0 };
    bytecode *labelA, *labelE = NULL;
    int param_count = 0;

    if (func_value->decl->params != NULL)
    {
        param_count = func_value->decl->params->count;
    }

    bc.type = BYTECODE_CLEAR_STACK;
    bc.clear.count = param_count;
    labelA = bytecode_add(module_value->code, &bc);

    if (value->expr_value != NULL)
    {
        expr_emit(value->expr_value, stack_level, module_value, list_weak, result);
    }
    
    bc.type = BYTECODE_RET;
    bytecode_add(module_value->code, &bc);

    bc.type = BYTECODE_LABEL;
    labelE = bytecode_add(module_value->code, &bc);

    exception_tab_insert(module_value->exctab_value, labelA->addr, labelE->addr);    
    
    return 0;
}

int except_emit(except * value, func * func_value, int stack_level,
                module * module_value, func_list_weak * list_weak, int * result)
{
    bytecode bc = { 0 };
    bytecode *labelA, *labelE = NULL;
    bytecode *condz = NULL;
    int param_count = 0;

    if (func_value->decl->params != NULL)
    {
        param_count = func_value->decl->params->count;
    }

    bc.type = BYTECODE_CLEAR_STACK;
    bc.clear.count = param_count;
    labelA = bytecode_add(module_value->code, &bc);
    
    bc.type = BYTECODE_INT;
    bc.integer.value = value->no;
    bytecode_add(module_value->code, &bc);

    bc.type = BYTECODE_PUSH_EXCEPT;
    bytecode_add(module_value->code, &bc);
    
    bc.type = BYTECODE_OP_EQ_INT;
    bytecode_add(module_value->code, &bc);
    
    bc.type = BYTECODE_JUMPZ;
    condz = bytecode_add(module_value->code, &bc);

    if (value->expr_value != NULL)
    {
        expr_emit(value->expr_value, stack_level, module_value, list_weak, result);
    }
    
    bc.type = BYTECODE_RET;
    bytecode_add(module_value->code, &bc);
    
    bc.type = BYTECODE_LABEL;
    labelE = bytecode_add(module_value->code, &bc);
    condz->jump.offset = labelE->addr - condz->addr;
    
    exception_tab_insert(module_value->exctab_value, labelA->addr, labelE->addr);    
    
    return 0;
}

int except_list_emit(except_list * list, func * func_value, int stack_level,
                     module * module_value, func_list_weak * list_weak, int * result)
{
    except_list_node * node = list->tail;
    while (node != NULL)
    {
        except * value = node->value;
        if (value != NULL)
        {
            except_emit(value, func_value, stack_level, module_value, list_weak, result);
        }
        node = node->next;
    }

    return 0;
}

int func_except_emit(func_except * value, func * func_value, int stack_level, 
                     module * module_value, func_list_weak * list_weak, int * result)
{
    if (value->list != NULL)
    {
        except_list_emit(value->list, func_value, stack_level, module_value, list_weak,
                         result);
    }
    if (value->all != NULL)
    {
        except_all_emit(value->all, func_value, stack_level, module_value, list_weak, result);
    }

    return 0;
}

int func_body_emit_ffi_param(param * value, module * module_value, int * result)
{
    bytecode bc = { 0 };

    switch (value->type)
    {
        case PARAM_INT:
            bc.type = BYTECODE_FUNC_FFI_INT;
        break;
        case PARAM_FLOAT:
            bc.type = BYTECODE_FUNC_FFI_FLOAT;
        break;
        case PARAM_STRING:
            bc.type = BYTECODE_FUNC_FFI_STRING;
        break;
        case PARAM_DIM:
        case PARAM_ARRAY:
        case PARAM_ENUMTYPE:
        case PARAM_RECORD:
        case PARAM_FUNC:
            *result = EMIT_FAIL;
            print_error_msg(value->line_no, "ffi type not supported\n");
        break;
    }

    bytecode_add(module_value->code, &bc);

    return 0;
}

int func_body_emit_ffi_param_list(param_list * params, module * module_value, int * result)
{
    param_list_node * node = params->tail;
    
    while (node != NULL)
    {
        param * value = node->value;
        if (value != NULL)
        {
            func_body_emit_ffi_param(value, module_value, result);
        }
        node = node->next;
    }

    return 0;
}

int func_body_emit_ffi(func * func_value, module * module_value,
                       func_list_weak * list_weak, int * result)
{
    bytecode bc = { 0 };
    unsigned int count = 0;
    bytecode * labelA = NULL;
    bytecode * labelE = NULL;
    
    if (func_value->decl != NULL && func_value->decl->params != NULL)
    {
        count = func_value->decl->params->count;
    }

    bc.type = BYTECODE_FUNC_FFI;
    bc.ffi.count = count;
    bc.ffi.fname_index = strtab_add_string(module_value->strtab_value, func_value->decl->id);
    bc.ffi.libname_index = strtab_add_string(module_value->strtab_value, func_value->libname);

    labelA = bytecode_add(module_value->code, &bc);
    func_value->addr = labelA->addr;
    
    if (func_value->decl != NULL && func_value->decl->params != NULL)
    {
        func_body_emit_ffi_param_list(func_value->decl->params, module_value, result);
    }
    if (func_value->decl != NULL && func_value->decl->ret != NULL)
    {
        func_body_emit_ffi_param(func_value->decl->ret, module_value, result);
    }
    
    bc.type = BYTECODE_RET;
    bytecode_add(module_value->code, &bc);

    bc.type = BYTECODE_LABEL;
    labelE = bytecode_add(module_value->code, &bc);

    exception_tab_insert(module_value->exctab_value, labelA->addr, labelE->addr);

    except_implicit_emit(func_value, 0, module_value, list_weak, result);

    return 0;
}

int func_body_emit_native(func * func_value, module * module_value,
                          func_list_weak * list_weak, int * result)
{
    bytecode bc = { 0 };
    bytecode *labelA, *labelE = NULL;
    int func_count = 0;

    bc.type = BYTECODE_FUNC_DEF;
    labelA = bytecode_add(module_value->code, &bc);
    func_value->addr = labelA->addr;

    if (func_value->body && func_value->body->binds)
    {
        bind_list_emit(func_value->body->binds, 0, module_value, list_weak, result);
        func_count += func_value->body->binds->count;
    }
    if (func_value->body && func_value->body->funcs)
    {
        func_list_emit(func_value->body->funcs, func_count, module_value,
                       list_weak, result);
        func_count += func_value->body->funcs->count;
    }
    if (func_value->body && func_value->body->ret)
    {
        expr_emit(func_value->body->ret, func_count, module_value, list_weak, result);
    }
    if (func_value->body && func_value->body->ret && func_value->body->ret->line_no > 0)
    {
        bc.type = BYTECODE_LINE;
        bc.line.no = func_value->body->ret->line_no;
        bytecode_add(module_value->code, &bc);
    }
    
    bc.type = BYTECODE_RET;
    bytecode_add(module_value->code, &bc);

    bc.type = BYTECODE_LABEL;
    labelE = bytecode_add(module_value->code, &bc);

    exception_tab_insert(module_value->exctab_value, labelA->addr, labelE->addr);

    if (func_value->except != NULL)
    {
        func_except_emit(func_value->except, func_value, 0, module_value, list_weak, result);
    }
    except_implicit_emit(func_value, 0, module_value, list_weak, result);

    return 0;
}

int func_body_emit(func * func_value, module * module_value,
                   func_list_weak * list_weak, int * result)
{
    switch (func_value->type)
    {
        case FUNC_TYPE_UNKNOWN:
            assert(0);
        break;
        case FUNC_TYPE_NATIVE:
            func_body_emit_native(func_value, module_value, list_weak, result);
        break;
        case FUNC_TYPE_FFI:
            func_body_emit_ffi(func_value, module_value, list_weak, result);
        break;
    }
    
    return 0;
}

int func_emit_ffi(func * func_value, int stack_level, module * module_value,
                  func_list_weak * list_weak, int * result)
{
    bytecode bc = { 0 };
    int freevar_count = 0;

    bc.type = BYTECODE_FUNC_OBJ;
    bytecode_add(module_value->code, &bc);

    if (func_value->line_no > 0)
    {
        bc.type = BYTECODE_LINE;
        bc.line.no = func_value->line_no;
        bytecode_add(module_value->code, &bc);
    }

    bc.type = BYTECODE_GLOBAL_VEC;
    bc.global_vec.count = freevar_count;
    bytecode_add(module_value->code, &bc);

    bc.type = BYTECODE_ID_FUNC_FUNC;
    bc.id_func.func_value = func_value;
    bytecode_add(module_value->code, &bc);

    func_list_weak_add(list_weak, func_value);

    return 0;
}

int func_emit_native(func * func_value, int stack_level, module * module_value,
                     func_list_weak * list_weak, int * result)
{
    bytecode bc = { 0 };
    int freevar_count = 0;

    bc.type = BYTECODE_FUNC_OBJ;
    bytecode_add(module_value->code, &bc);

    if (func_value->line_no > 0)
    {
        bc.type = BYTECODE_LINE;
        bc.line.no = func_value->line_no;
        bytecode_add(module_value->code, &bc);
    }

    if (func_value->freevars != NULL)
    {
        func_freevar_list_emit(func_value->freevars, stack_level, module_value, result);
        freevar_count = func_value->freevars->count;
    }

    bc.type = BYTECODE_GLOBAL_VEC;
    bc.global_vec.count = freevar_count;
    bytecode_add(module_value->code, &bc);

    bc.type = BYTECODE_ID_FUNC_FUNC;
    bc.id_func.func_value = func_value;
    bytecode_add(module_value->code, &bc);

    func_list_weak_add(list_weak, func_value);

    return 0;
}

int func_emit(func * func_value, int stack_level, module * module_value,
              func_list_weak * list_weak, int * result)
{
    switch (func_value->type)
    {
        case FUNC_TYPE_UNKNOWN:
            assert(0);
        break;
        case FUNC_TYPE_NATIVE:
            func_emit_native(func_value, stack_level, module_value, list_weak, result);
        break;
        case FUNC_TYPE_FFI:
            func_emit_ffi(func_value, stack_level, module_value, list_weak, result);
        break;
    }

    return 0;
}              

int func_main_emit(
    const char * main_name,
    never * nev,
    int stack_level,
    module * module_value,
    int * result)
{
    symtab_entry * entry = NULL;

    entry = symtab_lookup(nev->stab, main_name, SYMTAB_LOOKUP_LOCAL);
    if (entry != NULL && entry->type == SYMTAB_FUNC)
    {
        bytecode bc = { 0 };
        bytecode *labelA, *labelE, *labelH;

        bc.type = BYTECODE_MARK;
        labelA = bytecode_add(module_value->code, &bc);

        bc.type = BYTECODE_PUSH_PARAM;
        bytecode_add(module_value->code, &bc);

        bc.type = BYTECODE_GLOBAL_VEC;
        bc.global_vec.count = 0;
        bytecode_add(module_value->code, &bc);

        bc.type = BYTECODE_ID_FUNC_FUNC;
        bc.id_func.func_value = entry->func_value;
        bytecode_add(module_value->code, &bc);

        bc.type = BYTECODE_CALL;
        bytecode_add(module_value->code, &bc);

        bc.type = BYTECODE_LABEL;
        labelH = bytecode_add(module_value->code, &bc);
        labelA->mark.addr = labelH->addr;

        bc.type = BYTECODE_HALT;
        bytecode_add(module_value->code, &bc);

        bc.type = BYTECODE_LABEL;
        labelE = bytecode_add(module_value->code, &bc);
        
        bc.type = BYTECODE_UNHANDLED_EXCEPTION;
        bytecode_add(module_value->code, &bc);
        
        exception_tab_insert(module_value->exctab_value, labelA->addr, labelE->addr);            
    }
    else
    {
        *result = EMIT_FAIL;
        print_error_msg(0, "no %s function defined\n", main_name);
    }
    return 0;
}

int func_list_emit(func_list * list, int stack_level, module * module_value,
                   func_list_weak * list_weak, int * result)
{
    bytecode bc = { 0 };
    unsigned int n = list->count;

    bc.type = BYTECODE_ALLOC;
    bc.alloc.n = n;
    bytecode_add(module_value->code, &bc);

    func_list_node * node = list->tail;
    while (node != NULL)
    {
        func * func_value = node->value;
        if (func_value != NULL)
        {
            func_emit(func_value, stack_level + list->count, module_value,
                      list_weak, result);

            bc.type = BYTECODE_REWRITE;
            bc.rewrite.j = n--;
            bytecode_add(module_value->code, &bc);
        }
        node = node->next;
    }
    return 0;
}

int never_emit(const char * main_name, never * nev, module * module_value)
{
    int stack_level = 0;
    int gencode_res = 0;
    func_list_weak * list_weak = func_list_weak_new();

    func_list_emit(nev->funcs, stack_level, module_value, list_weak, &gencode_res);
    func_main_emit(main_name, nev, stack_level, module_value, &gencode_res);
        
    while (list_weak->count > 0)
    {
        func * value = func_list_weak_pop(list_weak);
        if (value != NULL)
        {
            func_body_emit(value, module_value, list_weak, &gencode_res);    
        }
    }
 
    func_list_weak_delete(list_weak);

    return gencode_res;
}
