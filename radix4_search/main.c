#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>


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


typedef struct radix4node {
    uint8_t value;
    struct radix4node *child1;
    struct radix4node *child2;
    struct radix4node *child3;
    struct radix4node *child4;
} radix4_node;

typedef struct radix4nodepack {
    char *word;
    int bit_flag;
    struct radix4nodepack *child1;
    struct radix4nodepack *child2;
    struct radix4nodepack *child3;
    struct radix4nodepack *child4;
} radix4_node_pack;

void my_error_handeler(int error_code) {
    switch (error_code) {
        case 0:
            printf("error0:bit get error,bit>3!\n");
            break;
        case 1:
            printf("error1:bit get error,bit>3!\n");
            break;
        case 2:
            printf("error2:bit get error,bit>3!\n");
            break;
        case 3:
            printf("error3:bit get error,bit>3!\n");
            break;
        default:
            printf("error:default error!");
    }
    exit(0);
}


void *create_node() {
    radix4_node *new_node = (radix4_node *) bupt_malloc(sizeof(radix4_node));
    new_node->value = 0;
    new_node->child1 = NULL;
    new_node->child2 = NULL;
    new_node->child3 = NULL;
    new_node->child4 = NULL;
    global_stats.treenodenum++;
    return new_node;
}

void insert_node_pack(radix4_node_pack *tree, const char *str, int len) {
    int count_23 = 0, flag_get_bit_pos = 8, flag_get_which_word = -1;
    uint8_t flag = 0x03;

    radix4_node_pack *new_node = NULL;
    radix4_node_pack *node = tree;
    while (count_23 != (len - 1) * 4) {
        if (count_23 % 4 == 0) {
            flag_get_bit_pos = 8;
            flag_get_bit_pos -= 2;
            flag_get_which_word++;
        }
        switch (((str[flag_get_which_word] & 0Xff) & (flag << flag_get_bit_pos)) >> flag_get_bit_pos) {
            case 0:
                if (node->child1 == NULL) {
                    new_node = create_node();
                    node->child1 = new_node;
                    new_node->word = str;
                    new_node->bit_flag = count_23 * 2 + 1;
                } else {
                    if (strlen(node->child1->word) >= len) {

                    } else if (strlen(node->child1->word) <= len) {

                    } else {
                        if (cmp(node->child1->word, str, strlen(node->child1->word), len))break;
                        else {

                        }
                    }
                }
                node = node->child1;
                break;
            case 1:
                if (node->child2 == NULL) {
                    new_node = create_node();
                    node->child2 = new_node;
                    new_node->word = str;
                    new_node->bit_flag = count_23 * 2 + 1;
                }
                node = node->child2;
                break;
            case 2:
                if (node->child3 == NULL) {
                    new_node = create_node();
                    node->child3 = new_node;
                    new_node->word = str;
                    new_node->bit_flag = count_23 * 2 + 1;
                }
                node = node->child3;
                break;
            case 3:
                if (node->child4 == NULL) {
                    new_node = create_node();
                    node->child4 = new_node;
                    new_node->word = str;
                    new_node->bit_flag = count_23 * 2 + 1;
                }
                node = node->child4;
                break;
            default:
                my_error_handeler(0);
        }
        count_23++;
        flag_get_bit_pos -= 2;
    }
}

void insert_node(radix4_node *tree, const char *str, int len) {               //len contains the '\0'
    int count_23 = 0, flag_get_bit_pos = 8, flag_get_which_word = -1;
    uint8_t flag = 0x03;

    radix4_node *new_node = NULL;
    radix4_node *node = tree;
    while (count_23 != (len - 1) * 4) {
        if (count_23 % 4 == 0) {
            flag_get_bit_pos = 8;
            flag_get_bit_pos -= 2;
            flag_get_which_word++;
        }

        switch (((str[flag_get_which_word] & 0Xff) & (flag << flag_get_bit_pos)) >> flag_get_bit_pos) {
            case 0:
                if (node->child1 == NULL) {
                    new_node = create_node();
                    node->child1 = new_node;
                }

                node = node->child1;
                break;
            case 1:
                if (node->child2 == NULL) {
                    new_node = create_node();
                    node->child2 = new_node;
                }

                node = node->child2;
                break;
            case 2:
                if (node->child3 == NULL) {
                    new_node = create_node();
                    node->child3 = new_node;
                }
                node = node->child3;
                break;
            case 3:
                if (node->child4 == NULL) {
                    new_node = create_node();
                    node->child4 = new_node;
                }

                node = node->child4;
                break;
            default:
                my_error_handeler(0);
        }
        count_23++;
        flag_get_bit_pos -= 2;
    }
    if (node->value == 0) {
        node->value = 1;
        global_stats.memsize += sizeof(char);
    }
}


void delet_tree(radix4_node *tree) {
    if (tree->child1 != NULL)delet_tree(tree->child1);
    if (tree->child2 != NULL)delet_tree(tree->child2);
    if (tree->child3 != NULL)delet_tree(tree->child3);
    if (tree->child4 != NULL)delet_tree(tree->child4);
    free(tree);
}

FILE *fp_words, *fp_patterns, *fp_result;

int main() {
    char buf_words[MAX_LINE], buf_patterns[MAX_LINE];
    int len_words, len_patterns;
    int judge = 0;
    int i = 0, n = 0;

    if ((fp_words = fopen("./words-98w.txt", "r")) == NULL ||
        (fp_patterns = fopen("./patterns-127w.txt", "r")) == NULL ||
        (fp_result = fopen("./radix4_search/result.txt", "a")) == NULL) {
        perror("fail to read");
        exit(1);
    }

    radix4_node *root = create_node();

    while (fgets(buf_patterns, MAX_LINE, fp_patterns) != NULL) {
        len_patterns = strlen(buf_patterns);
        buf_patterns[len_patterns - 1] = '\0';
        insert_node(root, buf_patterns, len_patterns);
    }
    printf("insert done\n");

    while (fgets(buf_words, MAX_LINE, fp_words) != NULL) {
        global_stats.wordsnum++;
        len_words = strlen(buf_words);
        buf_words[len_words - 1] = '\0';
        fprintf(fp_result, "%s", buf_words);
        if (find_word(root, buf_words, len_words)) {
            global_stats.wordsfoundnum++;
            fputs(" yes\n", fp_result);
        } else {
            fputs(" no\n", fp_result);
        }
        if ((n % 10000) == 0) {
            printf("%d\n", n);
            printf("%d %lf %d %d %d\n", global_stats.memsize, global_stats.cmpnum / 1000, global_stats.wordsnum,
                   global_stats.wordsfoundnum, global_stats.treenodenum);
            fclose(fp_result);
            (fp_result = fopen("./radix4_search/result.txt", "a"));
        }
        n++;
        i = 0;
    }
    fprintf(fp_result, "%d %lf %d %d %d\n", global_stats.treenodenum, global_stats.memsize / 1024, global_stats.cmpnum / 1000,
            global_stats.wordsnum, global_stats.wordsfoundnum);
    printf("find done\n");
    printf("%d %lf %d %d %d\n", global_stats.memsize, global_stats.cmpnum / 1000, global_stats.wordsnum,
           global_stats.wordsfoundnum, global_stats.treenodenum);
    fclose(fp_result);
    (fp_result = fopen("./radix4_search/result.txt", "a"));
    fputs("\n", fp_result);

    delet_tree(root);
    return 0;
}
