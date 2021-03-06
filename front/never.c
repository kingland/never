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

#include "never.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

never * never_new(enumtype_list * enums, record_list * records, func_list * funcs)
{
    never * n = (never *)malloc(sizeof(never));

    n->stab = NULL;
    n->enums = enums;
    n->records = records;
    n->funcs = funcs;

    return n;
}

void never_delete(never * nev)
{
    if (nev->enums)
    {
        enumtype_list_delete(nev->enums);
    }
    if (nev->records)
    {
        record_list_delete(nev->records);
    }
    if (nev->funcs)
    {
        func_list_delete(nev->funcs);
    }
    if (nev->stab)
    {
        symtab_delete(nev->stab);
    }
    free(nev);
}
