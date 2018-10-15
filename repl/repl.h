#pragma once
#ifndef __REPL_H__
#define __REPL_H__

#include <sys/types.h>

#ifdef __cplusplus
extern "C"
{
#endif

    struct repl_t
    {
        char * src;
        size_t len;
        size_t cap;
        size_t off;
    };

    void repl_setup(struct repl_t * repl);
    void repl_cleanup(struct repl_t * repl);
    int repl_loop(struct repl_t * repl);

#ifdef __cplusplus
}
#endif

#endif /* __REPL_H__ */
