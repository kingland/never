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
#include <stdio.h>

int print_zero()
{
    printf("0");
    
    return 0;
}

int print_one()
{
    printf("1");
    
    return 0;
}

int print_two()
{
    printf("2");
    
    return 0;
}

int print_three()
{
    printf("3");
    
    return 0;
}

int print_four()
{
    printf("4");
    
    return 0;
}

int print_five()
{
    printf("5");
    
    return 0;
}

int print_six()
{
    printf("6");
    
    return 0;
}

int print_seven()
{
    printf("7");
    
    return 0;
}

int print_eight()
{
    printf("8");
    
    return 0;
}

int print_nine()
{
    printf("9");
    
    return 0;
}

int print_newline()
{
    printf("\n");
    
    return 0;
}

int print_char(char c)
{
    switch (c)
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

int print_str(const char * str)
{
    char c = 0;

    while ((c = *str++) != 0)
    {
        print_char(c);
    }
    
    return 0;
}

void test_one()
{
    print_str("1234567890\n");
}

int main(int argc, char * argv[])
{
    test_one();

    return 0;
}


