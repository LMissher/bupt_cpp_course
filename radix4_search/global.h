#ifndef CPROJECT_GLOBAL_H
#define CPROJECT_GLOBAL_H

#define MAX_LINE 128
#define WORDS_NUM 986004
#define PATTERNS_NUM 1270688

typedef struct GS{
    int memsize;        //内存使用大小
    double cmpnum;      //比较次数
    int wordsnum;       //带比较字符串个数
    int wordsfoundnum;  //查找成功字符串个数
    int treenodenum;    //树节点个数
}globalstats;

extern globalstats global_stats;
void* bupt_malloc(size_t size);
void *buptCalloc(size_t bytes, size_t size);
int byte_cmp(char a, char b);
int cmp(char *ps1, char *ps2, size_t l1, size_t l2);

#endif //CPROJECT_GLOBAL_H
