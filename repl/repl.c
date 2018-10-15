#include "repl.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <nev.h>
#include <program.h>

#define PROMPT "nev:> "
#define TMPLFMT "func main() -> int {\n%s\n0\n}"
#define TMPLLEN 32
#define BUFSIZE 65536

static ssize_t readline(char ** line)
{
    size_t linecap = 0;
    return getline(line, &linecap, stdin);
}

static program * compile(const char * src)
{
    int err;

    program * prog = program_new();
    assert(prog);
    err = nev_compile_str(src, prog);
    if (err != 0)
    {
        program_delete(prog);
        prog = NULL;
    }
    return prog;
}

void repl_setup(struct repl_t * repl)
{
    repl->off = 0;
    repl->len = 0;
    repl->cap = BUFSIZE;
    repl->src = (char *)calloc(repl->cap, sizeof(char));
}

void repl_cleanup(struct repl_t * repl)
{
    if (!repl)
        return;
    if (repl->src)
    {
        free(repl->src);
        repl->src = NULL;
    }

    repl->off = repl->cap = repl->len = 0;
}

int repl_loop(struct repl_t * repl)
{
    int status = 0;
    int empty = 0;
    int err = 0;
    char * line = NULL;
    ssize_t len;
    char * src = NULL;
    object obj;
    program * prog = NULL;

prompt:
    fprintf(stdout, "%s", (repl->off != repl->len) ? "\t" : "");
    fprintf(stdout, PROMPT);
    if ((len = readline(&line)) < 0)
    {
        fprintf(stderr, "An error occured while reading an input.\n");
        return -1;
    }

    switch (len)
    {
    case 0:
        goto prompt;
        break;

    case 1:
        // reset
        if (line[0] == '\n' || line[0] == '\r')
        {
            ++empty;
            if (empty == 2)
            {
                // rollback to the last compiled version
                repl->len = repl->off;
                memset(repl->src + repl->len, 0, repl->cap - repl->len);
                empty = 0;
            }
            free(line);
            line = NULL;
            goto prompt;
        }
        break;

    default:
        if (line[0] == ':')
        {
            // command
            if (!strncmp(line, ":exit", 5))
            {
                status = 1;
                goto cleanup;
            }

            if (!strncmp(line, ":show", 5))
            {
                fprintf(stdout, "\n");
                fprintf(stdout, TMPLFMT, repl->src);
                fprintf(stdout, "\n");
                goto cleanup;
            }

            if (!strncmp(line, ":clear", 6))
            {
                repl_cleanup(repl);
                repl_setup(repl);
                goto cleanup;
            }
        }
        else
        {
            if (repl->cap < repl->len + len)
            {
                repl->cap += len + BUFSIZE;
                repl->src = (char *)realloc(repl->src, repl->cap);
                memset(repl->src + repl->len, 0, repl->cap - repl->len);
            }
            memcpy(repl->src + repl->len, line, len);
            repl->len += len;

            len = repl->len + TMPLLEN + 1;
            src = (char *)calloc(len, sizeof(char));
            snprintf(src, len, TMPLFMT, repl->src);

            prog = compile((const char *)src);
            err = (prog == NULL);
            if (!err)
            {
                repl->off = repl->len;
                err = (nev_execute(prog, &obj, DEFAULT_VM_MEM_SIZE,
                                   DEFAULT_VM_STACK_SIZE) != 0);
                if (err)
                {
                    fprintf(stderr,
                            "An error (%d) occured executing an input.\n", err);
                    // reset
                    repl_cleanup(repl);
                    repl_setup(repl);
                }
            }
        }
    }

cleanup:
    if (prog)
        program_delete(prog);
    if (src)
        free(src);
    if (line)
        free(line);

    return status;
}
