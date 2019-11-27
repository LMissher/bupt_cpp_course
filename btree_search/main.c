//
// Created by morningstarwang on 2019/10/28.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct Tree *Tree;
typedef struct BNode *BNode;

Tree  Tree_New        (int (*comp)(void *, void *));
void  Tree_Insert     (Tree t, void *data);
BNode  Tree_SearchNode (Tree t, void *data);


typedef struct Result Result;

struct Result
{
    int cmpNum;
    int memOccupy;
    int correctWordNum;
    int totalWordNum;
    int treeNodeNum;
};

Result result = {0, 0, 0, 0, 0};

int buptStrcmp(const char *s1, const char *s2)
{
    while ((result.cmpNum++) && *s1 && (*s1 == *s2))
    {
        s1++;
        s2++;
    }
    return *(const unsigned char *)s1 - *(const unsigned char *)s2;
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

void clearSuffix(char *line)
{
    char *find = strchr(line, '\n'); //查找换行符
    if (find)                        //如果find不为空指针
        *find = '\0';
}

struct Tree {
    BNode root;
    int (*comp) (void *, void *);
};

struct BNode {
    BNode parent;
    BNode left;
    BNode right;
    void *data;
    int balance;
};

void Tree_InsertBalance (Tree t, BNode node, int balance);
BNode Tree_RotateLeft (Tree t, BNode node);
BNode Tree_RotateRight (Tree t, BNode node);
BNode Tree_RotateLeftRight (Tree t, BNode node);
BNode Tree_RotateRightLeft (Tree t, BNode node);

BNode Node_New (void *data, BNode parent);


Tree Tree_New (int (*comp)(void *, void *)) {
    Tree t;
    t = buptMalloc (sizeof (*t));
    t->root = NULL;
    t->comp = comp;
    return t;
}

void Tree_Insert (Tree t, void *data) {
    if (t->root == NULL) {
        t->root = Node_New (data, NULL);
    } else {
        BNode node = t->root;
        while (node != NULL) {
            if ((t->comp) (data, node->data) < 0) {
                BNode left = node->left;
                if (left == NULL) {
                    node->left = Node_New (data, node);
                    Tree_InsertBalance (t, node, -1);
                    return;
                } else {
                    node = left;
                }
            } else if ((t->comp) (data, node->data) > 0) {
                BNode right = node->right;
                if (right == NULL) {
                    node->right = Node_New (data, node);
                    Tree_InsertBalance (t, node, 1);
                    return;
                } else {
                    node = right;
                }
            } else {
                node->data = data;
                return;
            }
        }
    }
}

BNode Tree_SearchNode (Tree t, void *data) {
    BNode node = t->root;

    while (node != NULL) {
        if ((t->comp) (data, node->data) < 0) {
            node = node->left;
        } else if ((t->comp) (data, node->data) > 0) {
            node = node->right;
        } else {
            return node;
        }
    }

    return NULL;
}

void Tree_InsertBalance (Tree t, BNode node, int balance) {
    while (node != NULL) {
        balance = (node->balance += balance);
        if (balance == 0) {
            return;
        } else if (balance == -2) {
            if (node->left->balance == -1) {
                Tree_RotateRight (t, node);
            } else {
                Tree_RotateLeftRight (t, node);
            }
            return;
        } else if (balance == 2) {
            if (node->right->balance == 1) {
                Tree_RotateLeft (t, node);
            } else {
                Tree_RotateRightLeft (t, node);
            }
            return;
        }
        BNode parent = node->parent;
        if (parent != NULL) {
            balance = (parent->left == node) ? -1 : 1;
        }
        node = parent;
    }
}

BNode Tree_RotateLeft (Tree t, BNode node) {
    BNode right = node->right;
    BNode rightLeft = right->left;
    BNode parent = node->parent;

    right->parent = parent;
    right->left = node;
    node->right = rightLeft;
    node->parent = right;

    if (rightLeft != NULL) {
        rightLeft->parent = node;
    }

    if (node == t->root) {
        t->root = right;
    } else if (parent->right == node) {
        parent->right = right;
    } else {
        parent->left = right;
    }

    right->balance--;
    node->balance = -right->balance;

    return right;
}

BNode Tree_RotateRight (Tree t, BNode node) {
    BNode left = node->left;
    BNode leftRight = left->right;
    BNode parent = node->parent;

    left->parent = parent;
    left->right = node;
    node->left = leftRight;
    node->parent = left;

    if (leftRight != NULL) {
        leftRight->parent = node;
    }

    if (node == t->root) {
        t->root = left;
    } else if (parent->left == node) {
        parent->left = left;
    } else {
        parent->right = left;
    }

    left->balance++;
    node->balance = -left->balance;

    return left;
}

BNode Tree_RotateLeftRight (Tree t, BNode node) {
    BNode left = node->left;
    BNode leftRight = left->right;
    BNode parent = node->parent;
    BNode leftRightRight = leftRight->right;
    BNode leftRightLeft = leftRight->left;

    leftRight->parent = parent;
    node->left = leftRightRight;
    left->right = leftRightLeft;
    leftRight->left = left;
    leftRight->right = node;
    left->parent = leftRight;
    node->parent = leftRight;

    if (leftRightRight != NULL) {
        leftRightRight->parent = node;
    }

    if (leftRightLeft != NULL) {
        leftRightLeft->parent = left;
    }

    if (node == t->root) {
        t->root = leftRight;
    } else if (parent->left == node) {
        parent->left = leftRight;
    } else {
        parent->right = leftRight;
    }

    if (leftRight->balance == 1) {
        node->balance = 0;
        left->balance = -1;
    } else if (leftRight->balance == 0) {
        node->balance = 0;
        left->balance = 0;
    } else {
        node->balance = 1;
        left->balance = 0;
    }

    leftRight->balance = 0;

    return leftRight;
}

BNode Tree_RotateRightLeft (Tree t, BNode node) {
    BNode right = node->right;
    BNode rightLeft = right->left;
    BNode parent = node->parent;
    BNode rightLeftLeft = rightLeft->left;
    BNode rightLeftRight = rightLeft->right;

    rightLeft->parent = parent;
    node->right = rightLeftLeft;
    right->left = rightLeftRight;
    rightLeft->right = right;
    rightLeft->left = node;
    right->parent = rightLeft;
    node->parent = rightLeft;

    if (rightLeftLeft != NULL) {
        rightLeftLeft->parent = node;
    }

    if (rightLeftRight != NULL) {
        rightLeftRight->parent = right;
    }

    if (node == t->root) {
        t->root = rightLeft;
    } else if (parent->right == node) {
        parent->right = rightLeft;
    } else {
        parent->left = rightLeft;
    }

    if (rightLeft->balance == -1) {
        node->balance = 0;
        right->balance = 1;
    } else if (rightLeft->balance == 0) {
        node->balance = 0;
        right->balance = 0;
    } else {
        node->balance = -1;
        right->balance = 0;
    }

    rightLeft->balance = 0;

    return rightLeft;
}

BNode Node_New (void *data, BNode parent) {
    result.treeNodeNum += 1;
    BNode n;

    n = buptMalloc (sizeof (*n));
    n->parent = parent;
    n->left = NULL;
    n->right = NULL;
    n->data = data;
    n->balance = 0;

    return n;
}


int comp (void *a1, void *a2) {

    int cmpResult = buptStrcmp(a1, a2);

    if (cmpResult < 0) {
        return -1;
    } else if (cmpResult > 0) {
        return +1;
    } else {
        return 0;
    }
}

int main (int argc, char **argv) {
    Tree tree;

    tree = Tree_New (comp);

    FILE *fpPattern = fopen("patterns-127w.txt", "r");
    FILE *fpWords = fopen("words-98w.txt", "r");
    FILE *fpResults = fopen("./btree_search/result.txt", "a");
    if (fpPattern == NULL || fpWords == NULL || fpResults == NULL)
    {
        printf("File not found.");
        return 0;
    }


    while (!feof(fpPattern))
    {
        char *patternLine = malloc(128);
        fgets(patternLine, 128, fpPattern);
        clearSuffix(patternLine);
        Tree_Insert(tree, patternLine);
    }
    int j = 986004;
    for (int m = 0; m < j; m++) // each word
    {
        char wordLine[128];
        fgets(wordLine, 128, fpWords);
        clearSuffix(wordLine);
        if (Tree_SearchNode(tree, wordLine) != NULL)
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
    fprintf(fpResults, "%d %d %d %d %d", result.treeNodeNum, result.memOccupy / 1024, result.cmpNum / 1000, result.totalWordNum, result.correctWordNum);
    fclose(fpPattern);
    fclose(fpWords);
    fclose(fpResults);
    return 0;
}
