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
#include "nev.h"
#include <stdio.h>
#include <assert.h>

int print_zero()    { printf("zero ");      return 0; }
int print_one()     { printf("I ");         return 0; }
int print_two()     { printf("owt ");       return 0; }
int print_three()   { printf("threee ");    return 0; }
int print_four()    { printf("IV ");        return 0; }
int print_five()    { printf("vife ");      return 0; }
int print_six()     { printf("ssiixx ");    return 0; }
int print_seven()   { printf("zodd ");      return 0; }
int print_eight()   { printf("e-i-g-h-t "); return 0; }
int print_nine()    { printf("9ine ");      return 0; }
int print_newline() { printf("\n");         return 0; }

int print_digit(char d)
{
    switch (d)
    {
        case '0': print_zero(); break;
        case '1': print_one(); break;
        case '2': print_two(); break;
        case '3': print_three(); break;
        case '4': print_four(); break;
        case '5': print_five(); break;
        case '6': print_six(); break;
        case '7': print_seven(); break;
        case '8': print_eight(); break;
        case '9': print_nine(); break;
        case '\n': print_newline(); break;
    }
    
    return 0;
}

int print_digits(const char * digits)
{
    char d = 0;

    while ((d = *digits++) != 0)
    {
        print_digit(d);
    }
    printf("\n");
    
    return 0;
}

void test_one()
{
    const char * prog_str =
        "extern \"main\" func print_digits(digits : string) -> int"
        "                                     "
        "func main() -> int                   "
        "{                                    "
        "    var d = 0;                       "
        "    var dig = \"\";                  "
        "                                     "
        "    for (d = 0; d < 10; d = d + 1)   "
        "        dig = dig + d;               "
        "                                     "
        "    print_digits(dig);               "
        "                                     "
        "    0                                "
        "}                                    "
        "                                     ";
    int ret = 0;
    program * prog = program_new();

    ret = nev_compile_str(prog_str, prog);
    if (ret == 0)
    {
        object result = { 0 };

        ret =
            nev_execute(prog, &result, DEFAULT_VM_MEM_SIZE, DEFAULT_VM_STACK_SIZE);
        if (ret == 0)
        {
            assert(result.type == OBJECT_INT &&
                   result.int_value == 0);
        }
    }

    program_delete(prog);
}

int main(int argc, char * argv[])
{
    test_one();

    return 0;
}


