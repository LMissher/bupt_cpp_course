#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifndef min
#define min(a,b) (((a)<(b))?(a):(b))
#endif
#define LEN_OF_STR 919943483
#define NUM_OF_PATTERNS 2256690
#define KIND 256

int head, tail, n_nodes;

typedef struct Word {
    char *ptr;
    int len;
} Word;
typedef struct GlobalStats {
    long long mem;
    long long cmpnum;
} GlobalStats;
extern GlobalStats global_stats;

void *bupt_malloc(int size);

int byte_cmp(char a, char b);

int str_cmp(char *a, int la, char *b, int lb);

typedef struct Result
{
    Word w;
    int tm;
} Result;

typedef struct AC_Node
{
    struct AC_Node* next[KIND];
    struct AC_Node* fail;
    Result* r;
}AC_Node;

AC_Node* createACNode();
void insertACNode(AC_Node* root, Result* r);
void buildFailPointer(AC_Node* root);
void ACAutomation(AC_Node* root, Word s);

AC_Node* createACNode()
{
    AC_Node* node = (AC_Node*)bupt_malloc(sizeof(AC_Node));
    for (int j = 0; j < KIND; j++)
        node->next[j] = 0;
    node->fail = 0;
    node->r = NULL;
    n_nodes++;
    return node;
}

void insertACNode(AC_Node* root, Result* r)
{
    Word w = r->w;
    AC_Node* p = root;
    for (int i = 0; i < w.len; i++)
    {
        unsigned char x = w.ptr[i];
        if (p->next[x] == NULL)
        {
            p->next[x] = createACNode();
        }
        p = p->next[x];
    }
    p->r = r;
}

// need to travel from shallow level to deep level. So use DFS with queue.
void buildFailPointer(AC_Node* root)
{
    head = 0;
    tail = 1;
    AC_Node** q = (AC_Node**)bupt_malloc(n_nodes * sizeof(AC_Node*));
    q[head] = root;
    AC_Node* p;
    AC_Node* temp;
    while (head < tail)
    {
        temp = q[head++];
        for (int i = 0; i < KIND; i++)
        {
            if (temp->next[i])
            {
                if (temp == root)
                {
                    temp->next[i]->fail = root;
                }
                else
                {
                    p = temp->fail;
                    while (p)
                    {
                        if (p->next[i])
                        {
                            temp->next[i]->fail = p->next[i];
                            break;
                        }
                        p = p->fail;
                    }
                    if (p == NULL) temp->next[i]->fail = root;
                }
                q[tail++] = temp->next[i];
            }
        }
    }
}


void ACAutomation(AC_Node* root, Word s)
{
    AC_Node* p = root;
    for (int i = 0; i < s.len; i++)
    {
        unsigned char x = s.ptr[i];
        while (!p->next[x] && p != root) p = p->fail;
        p = p->next[x];
        if (!p) p = root;
        AC_Node* temp = p;
        while (temp != root)
        {
            if(temp->r != NULL)
                temp->r->tm += 1;
            temp = temp->fail;
        }
    }
}

GlobalStats global_stats = { 0, 0 };
void* bupt_malloc(int size)
{
    if (size <= 0)
    {
        return NULL;
    }
    global_stats.mem += size;
    return malloc(size);
}
int byte_cmp(char a, char b)
{
    global_stats.cmpnum++;
    return a - b;
}
int str_cmp(char* a, int la, char* b, int lb)
{
    int l = min(la, lb);
    for (int i = 0; i < l; i++)
    {
        int t = byte_cmp(a[i], b[i]);
        if (t != 0)
        {
            return t;
        }
    }
    if (la != lb)
    {
        return la - lb;
    }
    return 0;
}

void get_next(int* next, char* a, int la)
{
    next[0] = -1;
    int i = -1;
    int j = 0;
    while (j < la - 1)
    {
        if (i == -1 || byte_cmp(a[j], a[i]) == 0)
        {
            ++j;
            ++i;
            if (a[j] != a[i])
                next[j] = i;
            else if (next[i] != -1)
                next[j] = next[i];
            else
                next[j] = 0;
        }
        else
        {
            i = next[i];
        }
    }
}

int compare_results(const void *a, const void *b) {
    Result arg1 = *(const Result *) a;
    Result arg2 = *(const Result *) b;

    if (arg1.tm < arg2.tm) return 1;
    if (arg1.tm > arg2.tm) return -1;
    return 0;
}

int main() {
    //FILE* fpPattern = fopen("./pattern_bf_kmp.txt", "rb");
    FILE *fpPattern = fopen("./pattern.txt", "r");
    //FILE* fpString = fopen("./string.txt", "rb");
    FILE *fpString = fopen("./string.txt", "r");
    FILE *fpResults = fopen("./ac_auto/result.txt", "a");

    if (fpPattern == NULL || fpString == NULL || fpResults == NULL) {
        printf("File not found.");
        return 0;
    }

    char *str = (char *) bupt_malloc(LEN_OF_STR);
    int n = 0, r = 0;
    while ((r = fgetc(fpString)) != EOF) {
        str[n++] = r;
    }
    Word str_w;
    str_w.ptr = str;
    str_w.len = n;

    // build ac_auto automan
    AC_Node *root = createACNode();

    int m = 0, l = 0;
    char buffer[128];
//    Result results[NUM_OF_PATTERNS];
    Result* results = bupt_malloc(sizeof(Result) * NUM_OF_PATTERNS);
    while (1) {
        char c = fgetc(fpPattern);
        if (c != '\n' && c != EOF) {
            buffer[m] = c;
            m++;
        } else {
            buffer[m] = 0;
            results[l].w.ptr = (char *) bupt_malloc(m + 1);
            memcpy(results[l].w.ptr, buffer, m + 1);
            results[l].w.len = m;
            results[l].tm = 0;
            insertACNode(root, &(results[l]));
            m = 0;
            l++;
        }
        if (c == EOF)
            break;
    }

    buildFailPointer(root);
    ACAutomation(root, str_w);

    qsort(results, NUM_OF_PATTERNS, sizeof(Result), compare_results);
    for (int i = 0; i < NUM_OF_PATTERNS; i++) {
        fprintf(fpResults, "%s %d\n", results[i].w.ptr, results[i].tm);
        free(results[i].w.ptr);
    }

    fprintf(fpResults, "%lld %lld", global_stats.cmpnum / 1000, global_stats.mem / 1024);
    fclose(fpPattern);
    fclose(fpString);
    fclose(fpResults);
    return 0;
}