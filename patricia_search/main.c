#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "string.h"

#ifndef _UTIL_RADIX_TREE_H_
#define _UTIL_RADIX_TREE_H_


void *util_radix_tree_create(int preallocate);

void util_radix_tree_destroy(void *vtree);

int util_radix32tree_insert(void *vtree, char* str, uint8_t mask, char* value);

int util_radix32tree_delete(void *vtree, uint32_t key, uint32_t mask);

int util_radix32tree_find(void *vtree, char* ptr);

int util_radix64tree_insert(void *vtree, uint64_t key, uint64_t mask, uintptr_t value);

int util_radix64tree_delete(void *vtree, uint64_t key, uint64_t mask);

uintptr_t util_radix64tree_find(void *vtree, uint64_t key);

int util_radix256tree_insert(void *vtree, uint8_t *key, uint8_t *mask, uintptr_t value);

int util_radix256tree_delete(void *vtree, uint8_t *key, uint8_t *mask);

uintptr_t util_radix256tree_find(void *vtree, uint8_t *key);

#endif // _UTIL_RADIX_TREE_H_


typedef struct util_radix_node_s util_radix_node_t;
uint8_t MASK =0xFF;
int size=0, cmpNum=0,nodeNum=0;
struct util_radix_node_s {
    util_radix_node_t  *right;
    util_radix_node_t  *left;
    util_radix_node_t  *parent;
	int flag;
};
typedef struct {
    util_radix_node_t  *root;
    util_radix_node_t  *free;
} util_radix_tree_t;



static util_radix_node_t *util_radix_alloc(util_radix_tree_t *tree)
{
	nodeNum++;
    util_radix_node_t *p;

    if (tree->free) {
        p = tree->free;
        tree->free = tree->free->right;
        return p;
    }
    p = malloc(sizeof(util_radix_node_t));
	size += sizeof(util_radix_node_t);
    if (p == NULL) {
        return NULL;
    }  

    return p;
}

static void util_radix_free(util_radix_node_t *node)
{
    if (node == NULL) {
        return;
    }
    free(node);
}

void *util_radix_tree_create(int preallocate)
{
    uint8_t            key, mask, inc;
    util_radix_tree_t  *tree;

    tree = malloc(sizeof(util_radix_tree_t));
	size += sizeof(util_radix_tree_t);
    if (tree == NULL) {
        return NULL;
    }

    tree->free = NULL;

    tree->root = util_radix_alloc(tree);
    if (tree->root == NULL) {
        return NULL;
    }

    tree->root->right  = NULL;
    tree->root->left   = NULL;
    tree->root->parent = NULL;
	tree->root->flag = 0;

    if (preallocate <= 0) {
        return tree;
    }
    
    mask = 0;
    inc = 0x80;

    while (preallocate--) {

        key = 0;
        mask >>= 1;
        mask |= 0x80;

        do {
            if (util_radix32tree_insert(tree, key, mask, 0)
                != 0)
            {
                return NULL;
            }

            key += inc;

        } while (key);

        inc >>= 1;
    }

    return tree;
}

static util_radix_node_t *radix_tree_postorder(util_radix_node_t *node)
{
    util_radix_node_t *p;

    if (NULL != node->left) {
        p = radix_tree_postorder(node->left);
        if (NULL != p) {
            return p;
        }
    }
    if (NULL != node->right) {
        p = radix_tree_postorder(node->right);
        if (NULL != p) {
            return p;
        }
    }
    util_radix_free(node);

    return NULL;
}

void util_radix_tree_destroy(void *vtree)
{
    util_radix_tree_t *tree = (util_radix_tree_t*)vtree;
    util_radix_node_t *p;

    radix_tree_postorder(tree->root);
    while (tree->free) {
        p = tree->free;
        tree->free = tree->free->right;
        util_radix_free(p);
    }
}

int util_radix32tree_insert(void* vtree, char* str, uint8_t mask,
	char val[])
{
	util_radix_tree_t* tree = (util_radix_tree_t*)vtree; ;
	util_radix_node_t* node, * next;
	uint8_t key;
	uint8_t bit = 0x80;
	key = str[0];
	node = tree->root;
	next = tree->root;
	for (int i = 0; str[i] != '\0'; i++) {
		key = str[i];
		bit = 0x80;
		while (bit & mask) {


			if (key & bit) {
				next = node->right;
			}
			else {
				next = node->left;
			}

			if (next == NULL) {
				break;
			}

			bit >>= 1;
			node = next;
		}
		if (bit == 0 && str[i + 1] == '\0') {
			if (next) {
				if (node->flag != NULL) {
					return 0;
				}

				return 0;
			}
		}


		while (bit & mask) {

			next = util_radix_alloc(tree);
			if (next == NULL) {
				return -1;
			}

			next->right = NULL;
			next->left = NULL;
			next->parent = node;
			next->flag = 0;

			if (key & bit) {
				node->right = next;

			}
			else {
				node->left = next;
			}

			bit >>= 1;
			node = next;

		}
	}
	node->flag = 1;
    return 0;
}



int util_radix32tree_find(void *vtree, char* ptr)
{
    util_radix_tree_t  *tree = (util_radix_tree_t*)vtree;
    char val[128];
	memset(val, '\0', 128);
    util_radix_node_t  *node;
	uint8_t bit = 0x80;
	uint8_t key = ptr[0];
	int i = 0;
    node  = tree->root;
	
	while (node) {
		cmpNum++;
		if (bit == 0 && key != '\0') {
			bit = 0x80;
			key = ptr[++i];
			if (key == '\0') {
				if (node->flag == 1) return 1;
				break;
			}
		}
			if (key & bit) {
				node = node->right;
			}
			else {
				node = node->left;
			}
			bit >>= 1;
	}
	if (node&&node->flag==1)  return 1;
	return 0;
	
	
}



int count = -3,sum=5;
int main(int argc, char *argv[])
{
	FILE* f3 = fopen("./patricia_search/result.txt", "w+");
    void *tree;
    int   a = 1, b = 2, c = 3, d = 0;
   char ptr[128] ;
    int   ret;
	tree = util_radix_tree_create(0);
	double pp;
	char str[128];
	FILE* f1 = fopen("./patterns-127w.txt", "rt");
	while (!feof(f1)) {
		fscanf(f1, "%s", str);
		ret = util_radix32tree_insert(tree, str, MASK, str);
	}
	fclose(f1);
	FILE* f2 = fopen("./words-98w.txt", "rt");
	//fscanf(f2, "%s", ptr);
	while (!feof(f2)) {
		sum++;
		fscanf(f2, "%s", ptr);
		if (util_radix32tree_find(tree, ptr)) {
			count++;
			fprintf(f3, "%s yes\n", ptr);
		}
		else
			fprintf(f3, "%s no\n", ptr);
	}
	printf("%d %d", count,sum);
	fprintf(f3, "%d %d %d %d %d \n", nodeNum,size/1024, cmpNum/1000, sum,count);
	fclose(f3);
	fclose(f2);
    util_radix_tree_destroy(tree);
    return 0;
}

