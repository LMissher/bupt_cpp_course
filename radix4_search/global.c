#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "global.h"

globalstats global_stats = {0,0,0,0,0};

void* bupt_malloc(size_t size){
    if (size <= 0) {
        return NULL;
    }
    global_stats.memsize += size;
    return malloc(size);
}

void *buptCalloc(size_t bytes, size_t size)
{
    if (size <= 0)
    {
        return NULL;
    }
    global_stats.memsize += bytes * size;
    return calloc(bytes, size);
}

int byte_cmp(char a, char b)
{
    global_stats.cmpnum++;
    return a-b;
}

int cmp(char *ps1, char *ps2, size_t l1, size_t l2)
{
    size_t i=0;
    if (l1 != l2)
        return 0;
    for (; i < l1; i++)
        if (byte_cmp(ps1[i], ps2[i]) != 0)
            return 0;
    if (i == l1)
        return 1;
    else
        return 0;
}
