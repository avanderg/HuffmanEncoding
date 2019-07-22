#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <stdint.h>

#define SIZE 4096 
#define BYTE_SIZE 8
#define FREQ_SIZE 4
#define CHAR_SIZE 1
#define CHARMAX 256

typedef struct node node;
struct node {
	uint8_t ch;
	uint32_t occur;
	node *rt;
	node *lft;
	node *nxt;
};
node *addNode(node *n, node *head);
int compare(node *n, node *m); 
node *build_list(uint32_t *freq, uint32_t *len); 
node *create_huff_tree(node *list, int len); 
void traverse_tree(node *tree, char *code, char **chars);
void free_tree(node *list); 
void free_chars (char **chars); 
void *safe_malloc(size_t size); 
/*void traverseTree(node *tree, char *code, char **chars, char *codel, 
		char* coder); */ 

int safe_open(char *fname, int flags, mode_t mode); 
