#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>
#include "huffman_funs.h"
#define BLK_SIZE 5

void decode_traversal(node *list, int infd, int outfd, uint8_t *buff, 
		unsigned long long total); 
unsigned long long build_freq_table(uint32_t *freq, int num, uint8_t *buff, 
		uint8_t *last_written); 
