#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#ifndef NULL
#define NULL 0
#endif
#ifndef min
#define max(a,b) (((a)>(b))?(a):(b))
#define min(a,b) (((a)<(b))?(a):(b))
#endif
#define LCHILD(x) 2 * x + 1
#define RCHILD(x) 2 * x + 2
#define PARENT(x) (x - 1) / 2
int LEN_OF_STR = 919943483;
int NUM_OF_PATTERNS = 1500;
typedef struct Word
{
    char* ptr;
    int len;
} Word;
typedef struct GlobalStats
{
    long long mem;
    long long cmpnum;
} GlobalStats;
extern GlobalStats global_stats;
void *bupt_malloc(int size);
int byte_cmp(char a, char b);
int str_cmp(char* a, int la, char* b, int lb);
int word_cmp(Word a, Word b);
void get_next(int *next, char *a, int la);
void get_word_next(int *next, Word w);
int str_kmp(int *next, char *S, int lS, char *P, int lP);
int word_kmp(int *next, Word a, Word b);

typedef struct Node
{
    int data;
    Word word;
} Node;

typedef struct MaxHeap
{
    int size;
    Node* elem;
} MaxHeap;

MaxHeap initMaxHeap(int size);
void insertNode(MaxHeap* hp, Node nd);
void deleteNode(MaxHeap* hp);
void deleteMaxHeap(MaxHeap* hp);

/*
	Function to initialize the max heap with size = 0
*/
MaxHeap initMaxHeap(int size)
{
    MaxHeap hp;
    hp.size = 0;
    hp.elem = malloc(size * sizeof(Node));
    return hp;
}


/*
	Function to swap data within two nodes of the max heap using pointers
*/
void swap(Node* n1, Node* n2)
{
    Node temp = *n1;
    *n1 = *n2;
    *n2 = temp;
}


/*
	Heapify function is used to make sure that the heap property is never violated
	In case of deletion of a node, or creating a max heap from an array, heap property
	may be violated. In such cases, heapify function can be called to make sure that
	heap property is never violated
*/
void heapify(MaxHeap* hp, int i)
{
    int largest = (LCHILD(i) < hp->size && hp->elem[LCHILD(i)].data > hp->elem[i].data) ? LCHILD(i) : i;
    if (RCHILD(i) < hp->size && hp->elem[RCHILD(i)].data > hp->elem[largest].data)
    {
        largest = RCHILD(i);
    }
    if (largest != i)
    {
        swap(&(hp->elem[i]), &(hp->elem[largest]));
        heapify(hp, largest);
    }
}


/*
	Build a Max Heap given an array of numbers
	Instead of using insertNode() function n times for total complexity of O(nlogn),
	we can use the buildMaxHeap() function to build the heap in O(n) time
*/
void buildMaxHeap(MaxHeap* hp, int* arr, int size)
{
    int i;

    // Insertion into the heap without violating the shape property
    for (i = 0; i < size; i++)
    {
        if (hp->size)
        {
            hp->elem = realloc(hp->elem, (hp->size + 1) * sizeof(Node));
        }
        else
        {
            hp->elem = malloc(sizeof(Node));
        }
        Node nd;
        nd.data = arr[i];
        hp->elem[(hp->size)++] = nd;
    }

    // Making sure that heap property is also satisfied
    for (i = (hp->size - 1) / 2; i >= 0; i--)
    {
        heapify(hp, i);
    }
}


/*
	Function to insert a node into the max heap, by allocating space for that node in the
	heap and also making sure that the heap property and shape propety are never violated.
*/
void insertNode(MaxHeap* hp, Node nd)
{
    int i = (hp->size)++;
    while (i && nd.data > hp->elem[PARENT(i)].data)
    {
        hp->elem[i] = hp->elem[PARENT(i)];
        i = PARENT(i);
    }
    hp->elem[i] = nd;
}


/*
	Function to delete a node from the max heap
	It shall remove the root node, and place the last node in its place
	and then call heapify function to make sure that the heap property
	is never violated
*/
void deleteNode(MaxHeap* hp)
{
    if (hp->size)
    {
        free(hp->elem[0].word.ptr);
        hp->elem[0] = hp->elem[--(hp->size)];
        hp->elem = realloc(hp->elem, hp->size * sizeof(Node));
        heapify(hp, 0);
    }
    else
    {
        free(hp->elem);
    }
}


/*
	Function to get minimum node from a max heap
	The minimum node shall always be one of the leaf nodes. So we shall recursively
	move through both left and right child, until we find their minimum nodes, and
	compare which is smaller. It shall be done recursively until we get the minimum
	node
*/
int getMinNode(MaxHeap* hp, int i)
{
    if (LCHILD(i) >= hp->size)
    {
        return hp->elem[i].data;
    }

    int l = getMinNode(hp, LCHILD(i));
    int r = getMinNode(hp, RCHILD(i));

    if (l <= r)
    {
        return l;
    }
    else
    {
        return r;
    }
}


/*
	Function to clear the memory allocated for the max heap
*/
void deleteMaxHeap(MaxHeap* hp)
{
    free(hp->elem);
}


void inorderTraversal(MaxHeap* hp, int i)
{
    if (LCHILD(i) < hp->size)
    {
        inorderTraversal(hp, LCHILD(i));
    }
    if (RCHILD(i) < hp->size)
    {
        inorderTraversal(hp, RCHILD(i));
    }
}


void preorderTraversal(MaxHeap* hp, int i)
{
    if (LCHILD(i) < hp->size)
    {
        preorderTraversal(hp, LCHILD(i));
    }
    if (RCHILD(i) < hp->size)
    {
        preorderTraversal(hp, RCHILD(i));
    }
}


void postorderTraversal(MaxHeap* hp, int i)
{
    if (LCHILD(i) < hp->size)
    {
        postorderTraversal(hp, LCHILD(i));
    }
    if (RCHILD(i) < hp->size)
    {
        postorderTraversal(hp, RCHILD(i));
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
int word_cmp(Word a, Word b)
{
    return str_cmp(a.ptr, a.len, b.ptr, b.len);
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
void get_word_next(int* next, Word w)
{
    get_next(next, w.ptr, w.len);
}
int str_kmp(int* next, char* S, int lS, char* P, int lP)
{
    int i = 0;
    int j = 0;
    while (i < lS && j < lP)
    {
        if (j == -1 || byte_cmp(S[i], P[j]) == 0)
        {
            i++;
            j++;
        }
        else
        {
            j = next[j];
        }
    }
    if (j == lP)
    {
        return i;
    }
    else
    {
        return -1;
    }
}
int word_kmp(int* next, Word a, Word b)
{
    return str_kmp(next, a.ptr, a.len, b.ptr, b.len);
}

int kmp_match(Word a, Word b, int* match);
int total_match(Word a, Word b);

int next[128];
int total_match(Word a, Word b)
{
    get_word_next(&next, b);
    Word tmp = a;
    int step = 0;
    int match = 0;
    while (1)
    {
        int r = word_kmp(&next, tmp, b);
        if (r == -1)
        {
            break;
        }
        else
        {
            match++;
        }
        step += r;
        tmp.ptr = a.ptr + step;
        tmp.len = a.len - step;
    }
    return match;
}


int main()
{
    FILE* fpPattern = fopen("./pattern_bf_kmp.txt", "rb");
    //FILE* fpPattern = fopen("./dd.txt", "r");
    FILE* fpString = fopen("./string.txt", "rb");
    //FILE* fpString = fopen("./data.txt", "r");
    FILE* fpResults = fopen("./multikmp/result.txt", "a");

    if (fpPattern == NULL || fpString == NULL || fpResults == NULL)
    {
        printf("File not found.");
        return 0;
    }

    char* str = (char*)bupt_malloc(LEN_OF_STR);
    int n = 0, r = 0;
    while ((r = fgetc(fpString)) != EOF)
    {
        str[n++] = r;
    }
    Word str_w;
    str_w.ptr = str;
    str_w.len = n;

    int m = 0;
    char buffer[128];
    MaxHeap heap = initMaxHeap(NUM_OF_PATTERNS);
    while (1)
    {
        char c = fgetc(fpPattern);
        if (c != '\n' && c != EOF)
        {
            buffer[m] = c;
            m++;
        }
        else
        {
            buffer[m] = 0;
            Node n;
            Word word;
            word.ptr = (char*)bupt_malloc(m + 1);
            memcpy(word.ptr, buffer, m + 1);
            word.len = m;
            n.data = total_match(str_w, word);
            n.word = word;
            insertNode(&heap, n);
            printf("total match: %d\n", n.data);
            m = 0;
        }
        if (c == EOF)
            break;
    }

    while (heap.size > 0)
    {
        Node n = heap.elem[0];
        fprintf(fpResults, "%s %d\n", n.word.ptr, n.data);
        deleteNode(&heap);
    }

    fprintf(fpResults, "%lld %lld", global_stats.cmpnum / 1000, global_stats.mem / 1024);
    fclose(fpPattern);
    fclose(fpString);
    fclose(fpResults);
    return 0;
}