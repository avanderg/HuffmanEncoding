/* Name: Aaron VanderGraaff 
   CPE357 Assgn3  
   2/4/19
*/

/* This program huffman decodes a given huffman encoded file (encoded by the 
   hencode program). Usage is: hdecode [(infile)|-] [outfile]. If infile is not
   given, the - signals to use stdin and if outfile is not given, the output is
   stdout. See hencode for more info on supported files.
*/

#include "hdecode.h"

#ifndef O_LARGEFILE
	#define O_LARGEFILE 0
#endif

int main(int argc, char *argv[]) {
	int infd, outfd;
	int num;
	uint8_t buff[SIZE] = {0};
	uint32_t freq[CHARMAX] = {0};
	uint32_t num_chars;
	uint32_t len;
	int i;
	uint8_t last_written = 0;
	infd = 0;
	outfd = 0;
	/* total keeps a tally of the total characters in the encoded file, since
	   we could be handling some files larger than 500 mB (~2^32 chars), use
	   long long to increase our possible file size to something stupid big.
	*/
	unsigned long long total = 0; 
	node *list = NULL;


	/* Input handling */
	if (argc > 3 || argc == 0) {
		printf("Usage: hdecode [(infile)|-) [outfile]]\n"); 
		exit(EXIT_FAILURE);
	}
	else if (argc == 3) {
		if (!strcmp(argv[1], "-")) {
			infd = STDIN_FILENO;
		}
		else {
			infd = safe_open(argv[1], O_RDONLY|O_LARGEFILE, 0);
		}

		outfd = safe_open(argv[2], O_WRONLY|O_CREAT|O_TRUNC|O_LARGEFILE,
				S_IRUSR|S_IWUSR);
	}
	else if (argc == 2) {
		infd = safe_open(argv[1], O_RDONLY|O_LARGEFILE, 0);
		outfd = STDOUT_FILENO;
	}
	else if (argc == 1) {
		infd = STDIN_FILENO;
		outfd = STDOUT_FILENO;
	}
	/* Read in the total number of characters from the header. */
	num = read(infd, &num_chars, FREQ_SIZE);
	if (num == -1) {
		perror("read");
		exit(EXIT_FAILURE);
	}

	/* for each character, there is a 5 byte block. 1 for the character
	   and 4 for the frequency.
	*/
	
	/* buff is size 4096 which is greater than the maximium size of the 
	   header (256*5 = 1280) so only need one read
	*/
	num = read(infd, buff, num_chars*BLK_SIZE);
	if (num == -1) {
		perror("read");
		exit(EXIT_FAILURE);
	}
	if (num != num_chars*BLK_SIZE) {
		fprintf(stderr, "wrong number of bytes read");
		exit(EXIT_FAILURE);
	}
	/* loop through each 5 byte block. The first byte is the character
	   which is used as the index into the frequency array, and the other
	   4 bytes are the frequency count, which is the data at the index
	*/

	total = build_freq_table(freq, num, buff, &last_written);

	if (num_chars == 1) {
		for (i=0; i<freq[last_written]; i++) {
			buff[i] = last_written;
		}
		num = write(outfd, buff, freq[last_written]);
		if (num == -1) {
			perror("write");
			exit(EXIT_FAILURE);
		}
	}
	else {		
		/* Now build huffman tree as in encode */
		len = 0;
		list = build_list(freq, &len);
		list = create_huff_tree(list, len);

		/* Now traverse tree as given by the contents of the file */
		decode_traversal(list, infd, outfd, buff, total);

		free_tree(list);
	}
	if (close(infd) == -1) {
		perror("close");
		exit(EXIT_FAILURE);
	}
	if (close(outfd) == -1) {
		perror("close");
		exit(EXIT_FAILURE);
	}

}

void decode_traversal(node *list, int infd, int outfd, uint8_t *buff, 
		unsigned long long total) {
	/* This function reads through the encoded file and writes the decoded
	   original to outfd
	*/

	int num;
	int i;
	int j;
	int num2;
	j = 0;
	unsigned long long counter;
	uint8_t temp;
	uint8_t buff2[SIZE];
	node *current = list;
	int k;
	uint8_t val;
	int write_flag;
	uint32_t test;
	test = 0;
	counter = 0;
	write_flag = 0;
	while ((num = read(infd, buff, SIZE)) > 0) {
		for (i=0; i<num; i++) {
			val = buff[i];
			for (k=0; k<BYTE_SIZE; k++) {
				/* use this flag to check if a node is a leaf
				   node
				*/ 
				write_flag = 0;
				if (counter >= total) {
					break;
				}
				if (current->rt == NULL && 
						current->lft == NULL) {
					/* if the children are null, add to 
					   buffer
					*/
					buff2[j++] = current->ch;
					counter++;
					if (j == SIZE) {
						num2 = write(outfd, buff2, 
								SIZE);
						test += SIZE;
						if (num2 == -1) {
							perror("write");
							exit(EXIT_FAILURE);
						}
						j = 0;
						
					}
					current = list;
					
				}
				temp = val & (1<<(BYTE_SIZE-1-k));
				temp >>= (BYTE_SIZE-1-k);
				/* Set the flag to check for leaf node */
				if (temp == 0) {
					current = current->lft;
					write_flag = 1;
				}
				else if (temp == 1) {
					current = current->rt;
					write_flag = 1;
				}
				
			}

		}
	}
	if (num == -1) {
		perror("write");
		exit(EXIT_FAILURE);
	}


	if (write_flag && current && 
			current->rt == NULL && current->lft == NULL) { 
			buff2[j++] = current->ch;
			counter++;
		
	}
	num2 = write(outfd, buff2, j);
	if (num2 == -1) {
		perror("write");
		exit(EXIT_FAILURE);
	}
}

unsigned long long build_freq_table(uint32_t *freq, int num, uint8_t *buff, 
		uint8_t *last_written) {

	int i;
	unsigned long long total = 0;
	uint8_t loc;
	uint32_t temp;
	loc = 0;
	temp = 0;

	for(i=0; i<=num; i++) {
		if (i%5 == 0) {
			/* if it's not the first iteration, this is a char.
			   Put the value of the next 4 bytes into the array
			*/
			freq[loc] = temp;
			*last_written = loc;
			/* Need total number of chars for reading file
			   so grow this total as the list is made 
			*/
			total += temp;
			
			loc = buff[i];
			temp = 0;
		}
		else {
			/* otherwise, add the bytes into a variable called temp
			   each byte is shifted so we keep the correct value
			*/
			temp += buff[i] << ((i-1)%BLK_SIZE)*BYTE_SIZE; 
		}
	}
	return total;
}

