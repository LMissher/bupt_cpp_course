//
// Created by morningstarwang on 2019/9/20.
//
#include "stdio.h"
#include "stdlib.h"
#include "string.h"

typedef struct Result Result;

struct Result {
    int cmpNum;
    int memOccupy;
    int correctWordNum;
    int totalWordNum;
};


Result result = {0, 0, 0, 0};

typedef struct Lines Lines;

struct Lines
{
    char line[128];
};

int buptStrcmp(const char *s1, const char *s2) {
    if (strlen(s1) != strlen(s2))
    {
        return -1;
    }
    while ((result.cmpNum++) && *s1 && (*s1 == *s2)) {
        s1++;
        s2++;
        result.cmpNum++;
    }
    return *(const unsigned char *) s1 - *(const unsigned char *) s2;
}

void *buptMalloc(size_t size)
{
    if (size <= 0) {
        return NULL;
    }
    result.memOccupy += size;
    return malloc(size);
}

void clearSuffix(char* line)
{
    char * find = strchr(line, '\n');          //查找换行符
    if(find)                            //如果find不为空指针
        *find = '\0';
}

int main() {
    FILE *fpPattern = fopen("patterns-127w.txt", "r");
    FILE *fpWords = fopen("words-98w.txt", "r");
    FILE *fpResults = fopen("./array_search/result.txt", "a");

    if (fpPattern == NULL || fpWords == NULL || fpResults == NULL)
    {
        printf("File not found.");
        return 0;
    }

    Lines *patterns = (Lines*) buptMalloc(128 * 1 * 1270688);
    int i = 0;
    int j = 986004;
    char patternLine[128];
    char wordLine[128];
    while (!feof(fpPattern))
    {
        fgets(patternLine, 128, fpPattern);
        clearSuffix(patternLine);
        memccpy(patterns[i].line, patternLine, 0, 128);
        i++;
    }
    for (int m = 0; m < j; m++) // each word
    {
        fgets(wordLine, 128, fpWords);
        clearSuffix(wordLine);
        int isYes = -1;
        for (int n = 0; n < i; n++) { // each pattern
            if (buptStrcmp(patterns[n].line, wordLine) == 0)
            {
                result.correctWordNum++;
                fprintf(fpResults, "%s", wordLine);
                fprintf(fpResults, " ");
                fprintf(fpResults, "yes\n");
                isYes = 1;
            }
        }
        if (isYes < 0)
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