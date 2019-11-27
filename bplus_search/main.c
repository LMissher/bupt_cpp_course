#ifndef NULL
#define NULL 0
#endif

#ifndef min
#define max(a,b) (((a)>(b))?(a):(b))
#define min(a,b) (((a)<(b))?(a):(b))
#endif

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

typedef struct
{
    char* ptr;
    int len;
} Words;

typedef struct
{
    int mem;
    int cmpnum;
} GlobalStats;

//extern GlobalStats global_stats;
//void *bupt_malloc(int size);
//int byte_cmp(char a, char b);
//int str_cmp(char* a, int la, char* b, int lb);
//int word_cmp(Words* a, Words* b);
//void get_next(int *next, char *a, int la);
//int str_kmp(int *next, char *S, int lS, char *P, int lP);
GlobalStats global_stats = {0, 0};

void *bupt_malloc(int size)
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

int str_cmp(char *a, int la, char *b, int lb)
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

int word_cmp(Words* a, Words* b)
{
    return str_cmp(a->ptr, a->len, b->ptr, b->len);
}

typedef struct Node
{
    int total;
    int no;
    struct Words **ws;
    struct Node **ns, *f, *next;
} Node;

extern int M;
extern int TOTAL_NODE;
Node* init();
int search(Node* root, Words* w);
Node* insert(Node* root, Words* w);
void free_node(Node* root);

int M = 3;
int TOTAL_NODE = 0;

int words_lower_bound(Words** a, Words* x, int n)
{
    if (word_cmp(x, a[n - 1]) > 0)
    {
        return n;
    }
    int l = 0;
    int h = n;
    while (l < h)
    {
        int mid = (l + h) / 2;
        if (word_cmp(x, a[mid]) <= 0)
        {
            h = mid;
        }
        else
        {
            l = mid + 1;
        }
    }
    return l;
}

Node* init()
{
    Node* r = (Node*)bupt_malloc(sizeof(Node));
    r->ws = (Words**)bupt_malloc(M * sizeof(Words*));
    memset(r->ws, 0, M * sizeof(Words*));
    r->ns = (Node**)bupt_malloc(M * sizeof(Node*));
    memset(r->ns, 0, M * sizeof(Node*));
    r->next = NULL;
    r->f = NULL;
    r->total = 0;
    r->no = -1;
    TOTAL_NODE++;
    return r;
}

int search_node(Node* root, Words* w, Node** n)
{
    if (root->total == 0)
    {
        *n = root;
        return 0;
    }
    int i = words_lower_bound(root->ws, w, root->total);
    if (root->ns[0] == NULL)
    {
        *n = root;
        return i;
    }
    else
    {
        search_node(root->ns[min(i, root->total - 1)], w, n);
    }
}

int search(Node* root, Words* w)
{
    Node* n;
    int i = search_node(root, w, &n);
    if (i < n->total && word_cmp(n->ws[i], w) == 0)
    {
        return 1;
    }
    return 0;
}

void insert_node(Node* n, Words* w, Node* nn, int i)
{
    if (n->total < M)
    {
        for (int j = M - 1; j > i; j--)
        {
            n->ws[j] = n->ws[j - 1];
            n->ns[j] = n->ns[j - 1];
            if (n->ns[j] != NULL)
            {
                (n->ns[j])->no = j;
            }
        }
        n->ws[i] = w;
        n->ns[i] = nn;
        if (nn != NULL)
        {
            nn->f = n;
            nn->no = i;
        }
        n->total++;
        if (i == M - 1) // modify the max
        {
            Node* temp = n->f;
            while (temp != NULL && word_cmp(temp->ws[temp->total - 1], w) < 0)
            {
                temp->ws[temp->total - 1] = w;
                temp = temp->f;
            }
        }
    }
    else
    {
        Node* nb = init();
        n->total = M / 2 + 1;
        nb->total = (M + 1) / 2;
        if (i <= M / 2)
        {
            for (int j = 0; j < nb->total; j++)
            {
                nb->ws[j] = n->ws[n->total - 1 + j];
                nb->ns[j] = n->ns[n->total - 1 + j];
                n->ws[n->total - 1 + j] = NULL;
                n->ns[n->total - 1 + j] = NULL;
                if (nb->ns[j] != NULL)
                {
                    (nb->ns[j])->no = j;
                    (nb->ns[j])->f = nb;
                }
            }
            for (int j = M / 2; j > i; j--)
            {
                n->ws[j] = n->ws[j - 1];
                n->ns[j] = n->ns[j - 1];
                if (n->ns[j] != NULL)
                {
                    (n->ns[j])->no = j;
                }
            }
            n->ws[i] = w;
            n->ns[i] = nn;
            if (n->ns[i] != NULL)
            {
                (n->ns[i])->f = n;
                (n->ns[i])->no = i;
            }
        }
        else
        {
            for (int j = nb->total - 1; j > i - n->total; j--)
            {
                nb->ws[j] = n->ws[n->total - 1 + j];
                nb->ns[j] = n->ns[n->total - 1 + j];
                n->ws[n->total - 1 + j] = NULL;
                n->ns[n->total - 1 + j] = NULL;
                if (nb->ns[j] != NULL)
                {
                    (nb->ns[j])->no = j;
                    (nb->ns[j])->f = nb;
                }
            }
            nb->ws[i - n->total] = w;
            nb->ns[i - n->total] = nn;
            if (nb->ns[i - n->total] != NULL)
            {
                (nb->ns[i - n->total])->f = nb;
                (nb->ns[i - n->total])->no = i - n->total;
            }
            for (int j = 0; j < i - n->total; j++)
            {
                nb->ws[j] = n->ws[j + n->total];
                nb->ns[j] = n->ns[j + n->total];
                n->ws[j + n->total] = NULL;
                n->ns[j + n->total] = NULL;
                if (nb->ns[j] != NULL)
                {
                    (nb->ns[j])->no = j;
                    (nb->ns[j])->f = nb;
                }
            }
        }
        if (n->f == NULL)
        {
            Node* nr = init();
            nr->ws[0] = n->ws[n->total - 1];
            nr->ns[0] = n;
            n->no = 0;
            n->f = nr;
            nr->ws[1] = nb->ws[nb->total - 1];
            nr->ns[1] = nb;
            nb->no = 1;
            nb->f = nr;
            nr->total = 2;
        }
        else
        {
            (n->f)->ws[n->no] = n->ws[n->total - 1];
            insert_node(n->f, nb->ws[nb->total - 1], nb, n->no + 1);
        }
    }
}

Node* insert(Node* root, Words* w)
{
    Node* n = NULL;
    int i = search_node(root, w, &n);
    insert_node(n, w, NULL, i);
    if (root->f != NULL)
    {
        return root->f;
    }
    return root;
}

void free_node(Node* root)
{
    if (root->ns[0] == NULL)
    {
        for (int i = 0; i < root->total; i++)
        {
            free(root->ws[i]);
        }
    }
    else
    {
        for (int i = 0; i < root->total; i++)
        {
            free_node(root->ns[i]);
        }
    }
    free(root);
}


int PATTERN_WORDS = 1270688;


int main()
{
    FILE* fpPattern = fopen("./patterns-127w.txt", "r");
    FILE* fpWords = fopen("./words-98w.txt", "r");
    FILE* fpResults = fopen("./bplus_search/result.txt", "a");

    if (fpPattern == NULL || fpWords == NULL || fpResults == NULL)
    {
        printf("File not found.");
        return 0;
    }

    char buffer[128];
    int r = 0, n = 0, m = 0;
    Node* root = init();
    while ((r = fgetc(fpPattern)) != EOF)
    {
        char c = r;
        if (c != '\n')
        {
            buffer[m] = c;
            m++;
        }
        else
        {
            buffer[m] = 0;
            char* cptr = (char*)bupt_malloc(m+1);
            memcpy(cptr, buffer, m + 1);
            Words* word = (Words*)bupt_malloc(sizeof(Words));
            word->ptr = cptr;
            word->len = m;
            root = insert(root, word);
            n++;
            m = 0;
        }
    }

    m = 0;
    int next[128];
    int totalWords = 0, totalMatchWords = 0;
    Words* word = (Words*)bupt_malloc(sizeof(Words));
    while ((r = fgetc(fpWords)) != EOF)
    {
        char c = r;
        if (c != '\n')
        {
            buffer[m] = c;
            m++;
        }
        else
        {
            buffer[m] = 0;
            word->ptr = &buffer;
            word->len = m;
            fprintf(fpResults, "%s", buffer);
            fprintf(fpResults, " ");
            if (search(root, word) == 1)
            {
                fprintf(fpResults, "yes\n");
                totalMatchWords++;
            }
            else
            {
                fprintf(fpResults, "no\n");
            }
            totalWords++;
            m = 0;
        }
    }
    free(word);

    free_node(root);
    fprintf(fpResults, "%d %d %d %d %d", TOTAL_NODE, global_stats.mem / 1024, global_stats.cmpnum / 1024, totalWords, totalMatchWords);
    fclose(fpPattern);
    fclose(fpWords);
    fclose(fpResults);
    return 0;
}