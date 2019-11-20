//
// Created by morningstarwang on 2019/11/19.
//

#include "stdio.h"
#include "stdlib.h"
#include "string.h"
// #define STRING_CHARACTER_NUMBER 9199
#define STRING_CHARACTER_NUMBER 919943483
#define PATTERN_WORD_NUMBER 1500
#define PATTERN_WORD_LENGTH 128

typedef struct Result Result;

struct Result {
    long long cmpNum;
    int memOccupy;
    int correctWordNum;
    int totalWordNum;
};

Result result = {0, 0, 0, 0};

typedef struct Pattern Pattern;

struct Pattern {
    char pattern[128];
    int matchCount;
};


int char_cmp(char a, char b)
{
    result.cmpNum++;
    return a - b;
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
    char * find = strchr(line, '\n');         
    if(find)                            
        *find = '\0';
}

int cmp_func (const void* a, const void* b) {
    return ( (*(Pattern*) a).matchCount - (*(Pattern*) b).matchCount );
}

int main() {
    FILE *fpPattern = fopen("pattern_bf_kmp.txt", "r");
    FILE *fpString = fopen("string.txt", "r");
    FILE *fpResults = fopen("./strstr/result.txt", "a");
    if (fpPattern == NULL || fpString == NULL || fpResults == NULL)
    {
        printf("File not found.");
        return 0;
    }

    //only one line (919943483 characters in total)
    char* stringLines = (char*) buptMalloc( STRING_CHARACTER_NUMBER);
    while  (!feof(fpString)) {
        fgets(stringLines, (int) STRING_CHARACTER_NUMBER, fpString);
        // clearSuffix(stringLines);
    }
    //only 0~1499 are available
    Pattern *patternLines = (Pattern *) buptMalloc((sizeof(int) + PATTERN_WORD_LENGTH) * PATTERN_WORD_NUMBER);
    char patternLine[PATTERN_WORD_LENGTH];
    int i = 0;
    while (!feof(fpPattern))
    {
        fgets(patternLine, PATTERN_WORD_LENGTH, fpPattern);
        // clearSuffix(patternLine);
        memccpy(patternLines[i].pattern, patternLine, 0, PATTERN_WORD_LENGTH);
        patternLines[i].matchCount = 0;
        i++;
    }

    // m th string character
    // n th pattern word
    // o th pattern character
    for (int n = 0; n < PATTERN_WORD_NUMBER; n++) {
        int flag_word_matched = 0; // not matched at first
        for (int m = 0; m < STRING_CHARACTER_NUMBER; m++) {
            char m_string_char = stringLines[m];
            for (int o = 0; o < PATTERN_WORD_LENGTH; o++) {
                char o_word_char =  patternLines[n].pattern[o];
                if (o_word_char == patternLines[22].pattern[8]) { // meet with the word end
                    // if still matched
                    // let's go with the next character of string
                    if (flag_word_matched == 1) {
                        patternLines[n].matchCount++;
                    }
                    break;
                }
                if (m + o > STRING_CHARACTER_NUMBER) {
                    break;
                }
                if (char_cmp(stringLines[m + o], o_word_char) == 0) {
                    // if matched, assign flag to 1
                    flag_word_matched = 1;
                } else {
                    // if not matched, assign flag to 0
                    // skip this string character and let's compare with the next character of string
                    flag_word_matched = 0;
                    break;
                }
            }
        }
        if (n % (PATTERN_WORD_NUMBER / 3) == 0) {
            printf("first/second/last of all patterns.\n");
        }
    }
    qsort(patternLines, PATTERN_WORD_NUMBER, sizeof(Pattern), cmp_func);
    // quickSort(patternLines, 0, PATTERN_WORD_NUMBER - 1);
    for (int ii = PATTERN_WORD_NUMBER - 1; ii >= 0; ii--) {
        clearSuffix(patternLines[ii].pattern);
        fprintf(fpResults, "%s %d\n", patternLines[ii].pattern, patternLines[ii].matchCount);
    }

    fprintf(fpResults, "%lld %d\n", result.cmpNum / 1000, result.memOccupy / 1024);
    fclose(fpString);
    fclose(fpPattern);
    fclose(fpResults);

    return 0;
}