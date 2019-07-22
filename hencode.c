/* Name: Aaron VanderGraaff 
   CPE357 Assgn3  
   2/4/19
*/

/* This program huffman encodes a given file. Usage is: hencode infile [outfile].
   If no outfile is given, the output is stdout. Note an infile is needed.
   The hdecode program decodes the encoded file to give the original. This program
   should work on files that are smaller than 2^64 bits (That's about 2 billion
   gB so I hope your file is smaller than that) and each individual character 
   appears less than 2^32 times (about 4 billion occurences). This program uses
   O_LARGEFILE so if your system does not support that, remove the GNU_SOURCE flag
   in the Makefile.
*/

#include "hencode.h"

#ifndef O_LARGEFILE
	#define O_LARGEFILE 0
#endif

int main(int argc, char *argv[]) {
	int infd, outfd;
	uint32_t len;
	uint8_t buff[SIZE] = {0};
	int num;
	int i;
	uint32_t freq[CHARMAX] = {0};
	node *list = NULL;
	char code[CHARMAX] = {'\0'};
	char *chars[CHARMAX] = {NULL};
	len = 0;
	infd = 0;
	outfd = 0;


	/* Input handling */
	if (argc > 3) {
		printf("Usage: hencode infile [outfile]\n");
		exit(EXIT_FAILURE);
	}
	else if (argc == 3) {
		infd = safe_open(argv[1], O_RDONLY|O_LARGEFILE, 0);
		outfd = safe_open(argv[2], O_WRONLY|O_CREAT|O_TRUNC|O_LARGEFILE, 
				S_IRUSR|S_IWUSR);
		
	}	
	else if (argc == 2) {
		infd = safe_open(argv[1], O_RDONLY|O_LARGEFILE, 0);
	  	outfd = STDOUT_FILENO;      
	}	
	else if (argc == 1) {
		printf("Usage: hencode infile [outfile]\n");
		exit(EXIT_FAILURE);
	}

	/* Read in occcurences of each character into freq array.
	   freq acts like a hash table; the index is the character value and the
	   data is the occurences
	*/
	while ((num = read(infd, buff, SIZE)) > 0) {
		for (i=0; i < num; i++) {
			/* buff[i] is the character, so we increase the data at
			   that index
			*/
			freq[buff[i]]++;
		}
	}
	
	if (num == -1) {
		perror("read");
		exit(EXIT_FAILURE);
	}

	/* build_list builds a linked list of characters with their frequencies in
	   ascending order. See build_list in huffman_funs
   	*/	   
	list = build_list(freq, &len);
	
	/* create_huff_tree builds a huffman tree with the given linked list */
	list = create_huff_tree(list, len);
	
	/* traverse_tree traverses the huffman tree and generates huffman codes,
	   putting them into the chars array
	*/ 
	traverse_tree(list, code, chars);

	

	/* write_header writes characters and their frequencies by the header
	   requirements. 
	*/
	write_header(outfd, freq, len); 
	
	/* Move the file descriptor for the infile back to the beginning.
	   Now we can read the input file and write it's huffman encoding to the
	   output file
	*/
	if (lseek(infd, 0, SEEK_SET) == -1) {
		perror("lseek");
		exit(EXIT_FAILURE);
	}

	/* Reads infd and writes the corresponding huffman codes to outfd */
	write_encoded(infd, outfd, chars);

	/* Free our huffman tree and chars array */
	free_tree(list);
	free_chars(chars);
	
	if (close(infd) == -1) {
		perror("close");
		exit(EXIT_FAILURE);
	}
	if (close(outfd) == -1) {
		perror("close");
		exit(EXIT_FAILURE);
	}
	
	return 0;
			
}

void write_header(int outfd, uint32_t freq[], uint32_t len) {
	/* Writes the header to the outfile using the frequencies array.
	   Format: 
	   	-First 4 bytes is the number of unique characters
		-Then each character occupies 5 bytes, 1 for the character 
		followed by 4 bytes for the occurences of the character.
	   You might be wondering why there are 4 bytes allocated for only
	   256 possible options (This could be represented with 1 byte). This 
	   was the format given for the assignment (not sure why).
	*/

	uint8_t c;
	int num;
	int i;
	
	/* Write total number of unique chars (up to 256)*/
	num = write(outfd, &len, FREQ_SIZE);
	if (num == -1) {
		perror("write");
		exit(EXIT_FAILURE);
	}

	/* loop through freq array */
	for (i=0; i<CHARMAX; i++) {
		/* If the occurences of a char is > 0, write the char and the 
		   occurences
		*/
		if (freq[i] != 0) {
			c = (uint8_t) i;
			num = write(outfd, &c, CHAR_SIZE); 
			if (num == -1) {
				perror("write");
				exit(EXIT_FAILURE);
			}
			num = write(outfd, &freq[i], FREQ_SIZE);
			if (num == -1) {
				perror("write");
				exit(EXIT_FAILURE);
			}
			
		}
		
	}
}

void write_encoded(int infd, int outfd, char **chars) {
	/* This function writes the body of the huffman encoded file. It takes
	   each character in infd and replaces it with it's corresponding 
	   huffman encoding. This gets tricky because we can only write full
	   bytes and huffman codes are almost definitely smaller than 1 byte
	   (or we aren't compressing very much, are we?). So each code is 
	   built to a byte then written, overflow goes to the next byte
	*/ 

	int num;
	int num2;
	int i;
	int j;
	char s[BYTE_SIZE + 1] = {'\0'};
	uint32_t index;
	uint32_t counter;
	uint8_t c;
	uint8_t buff[SIZE] = {0};
	uint8_t buff2[SIZE] = {0};
	index = 0;
	counter = 0;

	while ((num = read(infd, buff, SIZE)) > 0) {
		/* Loop through all bytes read in (num) */
		for (i=0; i<num; i++) {
			/* loop through all characters of code at index i */
			for (j=0; chars[buff[i]][j] != '\0'; j++) {
				/* s is the array that stores bytes */ 
				/* chars is an array of codes. buff[i] points to
				   the code of the character. Use j to access 
				   one element of the code at a time (to build
				   the byte.
				*/
				s[counter%BYTE_SIZE] = chars[buff[i]][j];
				counter++;
				if (counter != 0 && counter%BYTE_SIZE == 0) {
					/* if the counter is divisible by 8
					   ie the byte is full, add the byte to 
					   write buffer (buff2)
					*/
					/*c = string_to_int(s); */
					c = (uint8_t) strtol(s, NULL, 2); 
					/* put byte in buffer at index */
					buff2[index++] = c;
					if (index == SIZE) {
						/* if the buffer is full, write
						   and reinitialize it
						*/
						num2 = write(outfd, buff2, 
								index);
						if (num2 == -1) {
							perror("write");
							exit(EXIT_FAILURE);
						}
						index = 0;
						/* reset counter as well to
						   prevent overflow for large files
						*/
						counter = 0;
					}
				}
			}
		}
	}
	/* Since our file probablly doesn't have a nice round number of bytes,
	   populate the trailing bits with 0 to make the last byte.
	*/
	
	if (counter %BYTE_SIZE != 0) {
		while(counter%BYTE_SIZE != 0) {
			s[counter%BYTE_SIZE] = '0';
			counter++;
		}
		c = (uint8_t) strtol(s, NULL, 2); 
		 
		/*c = string_to_int(s);*/
		buff2[index++] = c;
	}
	
	
	/* The buffer now contains "index" bytes that need to be written,
	   write those
	*/
	
	num2 = write(outfd, buff2, index);
	if (num2 == -1) {
		perror("write");
		exit(EXIT_FAILURE);
	}
}

/* now use strtol, it's way faster */
uint8_t string_to_int(char *s) {
	/* Convert the huffman code string of 1's and 0's to a uint8_t 
	   If we wrote the strings, each 1 or 0 would take up a full byte
	   (ASCII 1's and 0's), that wouldn't be good for data compression!
	*/ 

	uint8_t res;
	
	int i;
	res = 0;
	for (i=0; s[i] != '\0'; i++) {
		res <<= 1;
		if (s[i] == '1') {
			res += 1;
		}
	}

	/*res = (uint8_t) strtol(s, NULL, 2); */
	return res;
}

