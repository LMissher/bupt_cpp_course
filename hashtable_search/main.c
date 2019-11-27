//
// Created by morningstarwang on 2019/9/26.
//

#define _XOPEN_SOURCE 500 /* Enable certain library functions (strdup) on linux.  See feature_test_macros(7) */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "hashtable.h"

typedef struct Result Result;

struct Result
{
    int cmpNum;
    int memOccupy;
    int correctWordNum;
    int totalWordNum;
};

Result result = {0, 0, 0, 0};

int buptStrcmp(const char *s1, const char *s2)
{
    while ((result.cmpNum++) && *s1 && (*s1 == *s2))
    {
        s1++;
        s2++;
    }
    return *(const unsigned char *)s1 - *(const unsigned char *)s2;
}

void clearSuffix(char *line)
{
    char *find = strchr(line, '\n'); //查找换行符
    if (find)                        //如果find不为空指针
        *find = '\0';
}

void *buptMalloc(size_t size)
{
    if (size <= 0)
    {
        return NULL;
    }
    result.memOccupy += size;
    return malloc(size);
}

struct elt
{
    struct elt *next;
    char *key;
};

struct dict
{
    int size; /* size of the pointer table */
    int n;    /* number of elements stored */
    struct elt **table;
};

#define INITIAL_SIZE (1024)
#define GROWTH_FACTOR (2)
#define MAX_LOAD_FACTOR (1)

/* dictionary initialization code used in both DictCreate and grow */
Dict internalDictCreate(int size)
{
    Dict d;
    int i;

    d = buptMalloc(sizeof(*d));

    assert(d != 0);

    d->size = size;
    d->n = 0;
    d->table = buptMalloc(sizeof(struct elt *) * d->size);

    assert(d->table != 0);

    for (i = 0; i < d->size; i++)
        d->table[i] = 0;

    return d;
}

Dict DictCreate(void)
{
    return internalDictCreate(INITIAL_SIZE);
}

#define MULTIPLIER (97)

static unsigned long
hash_function(const char *s)
{
    unsigned const char *us;
    unsigned long h;

    h = 0;

    for (us = (unsigned const char *)s; *us; us++)
    {
        h = h * MULTIPLIER + *us;
    }

    return h;
}

void DictDestroy(Dict d)
{
    int i;
    struct elt *e;
    struct elt *next;

    for (i = 0; i < d->size; i++)
    {
        for (e = d->table[i]; e != 0; e = next)
        {
            next = e->next;

            free(e->key);
            free(e);
        }
    }

    free(d->table);
    free(d);
}

static void grow(Dict d)
{
    Dict d2;          /* new dictionary we'll create */
    struct dict swap; /* temporary structure for brain transplant */
    int i;
    struct elt *e;

    d2 = internalDictCreate(d->size * GROWTH_FACTOR);

    for (i = 0; i < d->size; i++)
    {
        for (e = d->table[i]; e != 0; e = e->next)
        {
            /* note: this recopies everything */
            /* a more efficient implementation would
             * patch out the strdups inside DictInsert
             * to avoid this problem */
            DictInsert(d2, e->key);
        }
    }

    /* the hideous part */
    /* We'll swap the guts of d and d2 */
    /* then call DictDestroy on d2 */
    swap = *d;
    *d = *d2;
    *d2 = swap;

    DictDestroy(d2);
}

/* insert a new key-value pair into an existing dictionary */
void DictInsert(Dict d, const char *key)
{
    struct elt *e;
    unsigned long h;

    assert(key);

    e = buptMalloc(sizeof(*e));

    assert(e);

    e->key = strdup(key);

    h = hash_function(key) % d->size;

    e->next = d->table[h];
    d->table[h] = e;

    d->n++;

    /* grow table if there is not enough room */
    if (d->n >= d->size * MAX_LOAD_FACTOR)
    {
        grow(d);
    }
}

/* return the most recently inserted value associated with a key */
/* or 0 if no matching key is present */
int DictSearch(Dict d, const char *key)
{
    struct elt *e;

    for (e = d->table[hash_function(key) % d->size]; e != 0; e = e->next)
    {
        if (!buptStrcmp(e->key, key))
        {
            /* got it */
            return 1;
        }
    }

    return 0;
}

int main()
{
    Dict d;
    d = DictCreate();
    FILE *fpPattern = fopen("patterns-127w.txt", "r");
    FILE *fpWords = fopen("words-98w.txt", "r");
    FILE *fpResults = fopen("./hashtable_search/result.txt", "a");
    if (fpPattern == NULL || fpWords == NULL || fpResults == NULL)
    {
        printf("File not found.");
        return 0;
    }
    char patternLine[128];
    char wordLine[128];
    while (!feof(fpPattern))
    {
        fgets(patternLine, 128, fpPattern);
        clearSuffix(patternLine);
        DictInsert(d, patternLine);
    }
    int j = 986004;
    for (int m = 0; m < j; m++) // each word
    {
        fgets(wordLine, 128, fpWords);
        clearSuffix(wordLine);
        if (DictSearch(d, wordLine) == 1)
        {
            fprintf(fpResults, "%s", wordLine);
            fprintf(fpResults, " ");
            fprintf(fpResults, "yes\n");
            result.correctWordNum++;
        }
        else
        {
            fprintf(fpResults, "%s", wordLine);
            fprintf(fpResults, " ");
            fprintf(fpResults, "no\n");
        }
    }
    result.totalWordNum = j;
    fprintf(fpResults, "%d %d %d %d", result.memOccupy / 1024, result.cmpNum, result.totalWordNum, result.correctWordNum);
    fclose(fpPattern);
    fclose(fpWords);
    fclose(fpResults);
    return 0;
}