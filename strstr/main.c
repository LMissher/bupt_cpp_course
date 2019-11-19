//
// Created by morningstarwang on 2019/11/19.
//

#include "stdio.h"
#include "stdlib.h"
#include "string.h"
//#define STRING_CHARACTER_NUMBER 9199
#define STRING_CHARACTER_NUMBER 919943483
#define PATTERN_WORD_NUMBER 1500
#define PATTERN_WORD_LENGTH 128

typedef struct Result Result;

struct Result {
    int cmpNum;
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
    char * find = strchr(line, '\n');          //查找换行符
    if(find)                            //如果find不为空指针
        *find = '\0';
}

 int division(Pattern* list, int left, int right) {
    // 以最左边的数(left)为基准
    int base = list[left].matchCount;
    while (left < right) {
        // 从序列右端开始，向左遍历，直到找到小于base的数
        while (left < right && list[right].matchCount >= base)
            right--;
        // 找到了比base小的元素，将这个元素放到最左边的位置
        list[left] = list[right];

        // 从序列左端开始，向右遍历，直到找到大于base的数
        while (left < right && list[left].matchCount <= base)
            left++;
        // 找到了比base大的元素，将这个元素放到最右边的位置
        list[right].matchCount = list[left].matchCount;
    }

    // 最后将base放到left位置。此时，left位置的左侧数值应该都比left小；
    // 而left位置的右侧数值应该都比left大。
    list[left].matchCount = base;
    return left;
}

 void quickSort(Pattern* list, int left, int right){

    // 左下标一定小于右下标，否则就越界了
    if (left < right) {
        // 对数组进行分割，取出下次分割的基准标号
        int base = division(list, left, right);

        // 对“基准标号“左侧的一组数值进行递归的切割，以至于将这些数值完整的排序
        quickSort(list, left, base - 1);

        // 对“基准标号“右侧的一组数值进行递归的切割，以至于将这些数值完整的排序
        quickSort(list, base + 1, right);
    }
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
        clearSuffix(stringLines);
    }
    //only 0~1499 are available
    Pattern *patternLines = (Pattern *) buptMalloc((sizeof(int) + PATTERN_WORD_LENGTH) * PATTERN_WORD_NUMBER);
    char patternLine[PATTERN_WORD_LENGTH];
    int i = 0;
    while (!feof(fpPattern))
    {
        fgets(patternLine, PATTERN_WORD_LENGTH, fpPattern);
        clearSuffix(patternLine);
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
                if (o_word_char == patternLines[n].pattern[127]) { // meet with the word end
                    // if still matched
                    // let's go with the next character of string
                    if (flag_word_matched == 1) {
                        patternLines[n].matchCount++;
                    }
                    break;
                }
                if (char_cmp(m_string_char, o_word_char) == 0) {
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
    }

    quickSort(patternLines, 0, PATTERN_WORD_NUMBER - 1);
    for (int ii = PATTERN_WORD_NUMBER - 1; ii >= 0; ii--) {
        fprintf(fpResults, "%s %d\n", patternLines[ii].pattern, patternLines[ii].matchCount);
    }

    fprintf(fpResults, "%d %d\n", result.cmpNum, result.memOccupy / 1024);
    fclose(fpString);
    fclose(fpPattern);
    fclose(fpResults);

    return 0;
}