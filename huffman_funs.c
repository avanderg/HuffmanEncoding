/* Name: Aaron VanderGraaff 
   CPE357 Lab03
   1/29/19
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include "huffman_funs.h"

node *add_node(node *new, node *head) {
	/* Add a node into the appropriate location in the list. List is 
	   created in ascending order. Used method discussed in class.
	*/

	node *res = NULL;
	if(!head || compare(new, head)) {
		new->nxt = head;
		res = new;
	}
	else {
		res = head;
		while (head->nxt && compare(head->nxt, new)) {
			head = head->nxt;
		}
		new->nxt = head->nxt;
		head->nxt = new;
	}
	return res;
}
				
				
int compare(node *n, node *m) {
	/* Compares 2 nodes, this acts as "<" operator */

	if(n->occur == m->occur && n->ch == m->ch) {
		return 0;
	}

	else if (n->occur == m->occur) {
		if (m->ch == -1) {
			return 0;
		}
		else {
			return n->ch < m->ch;
		}
	}
	else {
		return n->occur < m->occur;
	}
}


node *build_list(uint32_t *freq, uint32_t *len) {
	/* Takes frequency list with its length and creates the linked list.
	   Builds linked list of nodes in ascending order by occurences then
	   by alphabet.
	*/

	int i;
	node *list = NULL;
	for (i=0; i<CHARMAX; i++) {
		if (freq[i] != 0) {
			node *new_node = NULL;
			if (!(new_node = malloc(sizeof(node)))) {
				perror("malloc");
				exit(EXIT_FAILURE);
			}
			new_node->ch =  (uint8_t) i;
			new_node->occur = (uint32_t) freq[i];
			new_node->lft = NULL;
			new_node->rt = NULL;
			new_node->nxt = NULL;
			(*len)++;
			list = add_node(new_node, list);
		}
	}
	return list;
}


node *create_huff_tree(node *list, int len) {
	/* Creates huffman tree from linked list of frequencies. */ 
	
	while (len > 1) {
		node *new_node = NULL;
		if (!(new_node = malloc(sizeof(node)))) {
			perror("malloc");
			exit(EXIT_FAILURE);
		}
		new_node->occur = list->occur + list->nxt->occur;
		new_node->ch = -1;
		new_node->nxt = NULL;

		new_node->rt = list->nxt;
		new_node->lft = list;
		/* Remove these two nodes */
		list = list->nxt->nxt;

		list = add_node(new_node, list);
		len--;
	}
	return list;

}

void traverse_tree(node *tree, char *code, char **chars)  {

	/* Traverses huffman tree and stores the huffman codes into an array
	   of strings (char **chars)
	*/

	int i;
	node *temp=tree;
	if (temp != NULL) {
		if (temp->rt == NULL && temp->lft == NULL) {
			i = temp->ch;
			char *finalcode = NULL;
			if (!(finalcode = malloc(strlen(code) + 1))) {
				perror("malloc");
				exit(EXIT_FAILURE);
			}

			/*strncpy(finalcode, code, strlen(code)+1);*/
			strcpy(finalcode, code);
			chars[i] = finalcode;
		} 


		char codel[strlen(code) + 2];
		/*strncpy(codel, code, strlen(code)+1);*/
		strcpy(codel, code);
	
		if (codel[0] == '\0') {
			codel[0] = '0';
			codel[1] = '\0';
		}
		else {
			codel[strlen(code)] = '0';
			codel[strlen(code) + 1] = '\0';
		}
		
		char coder[strlen(code) + 2];
		/*strncpy(coder, code, strlen(code)+1);*/
		strcpy(coder, code);
		if (coder[0] == '\0') {
			coder[0] = '1';
			coder[1] = '\0';
		}
		else{
			coder[strlen(code)] = '1';
			coder[strlen(code) + 1] = '\0';
		}

		
		traverse_tree(temp->lft, codel, chars);
		traverse_tree(temp->rt, coder, chars);
	}
	
}	

void free_tree(node *list) {
	
	if (list == NULL) {
		return;
	}
	free_tree(list->lft);
	free_tree(list->rt);
	free(list);
}

void free_chars (char **chars) {

	int i;
	for(i=0; i<CHARMAX; i++) {
		if (chars[i] != NULL) {
			free(chars[i]);

		}
	}
}

void *safe_malloc(size_t size) {
	void *ptr = malloc(size);
	if (!ptr) {
		perror("safe_malloc");
		exit(EXIT_FAILURE);
	}
	return ptr;
}

int safe_open(char *fname, int flags, mode_t mode) {
	int fd;
	if (mode == 0) {
		fd = open(fname, flags);
		if (fd == -1) {
			perror("safe_open");
			exit(EXIT_FAILURE);
		}
	}
	else {
		fd = open(fname, flags, mode);
	}
	return fd;
}
