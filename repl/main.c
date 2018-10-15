
#include "repl.h"

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>

struct repl_t repl;

static void sigint(int signo)
{
    repl_cleanup(&repl);
    exit(0);
}

int main(int argc, char ** argv)
{
    int err = 0;

    if (signal(SIGINT, sigint) == SIG_ERR)
    {
        fprintf(stderr, "An error occurred while setting a signal handler.\n");
    }

    repl_setup(&repl);
    do
    {
        err = repl_loop(&repl);
    } while (!err);

    repl_cleanup(&repl);
}
