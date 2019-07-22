#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>
#include "huffman_funs.h"
void write_header(int outfd, uint32_t freq[], uint32_t len); 
void write_encoded(int infd, int outfd, char **chars); 
uint8_t string_to_int(char *s); 
