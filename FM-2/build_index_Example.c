#include "interface.h"
#include "ds_ssort.h" // init_ds_ssort
#include <stdio.h>
#include <string.h>

/* macro to detect and notify errors  */
#define IFERROR(error) {{if (error) { fprintf(stderr, "%s\n", error_index(error)); exit(1); }}}

int read_file(char *filename, uchar **textt, ulong *length);
void print_usage(char *);
double getTime(void);

int main(int argc, char *argv[]) {

	char *infile, *outfile;
    uchar *text;
	ulong text_len;
	void *index;
	int error;

    if (argc < 2) print_usage(argv[0]);

	infile = argv[1];  // input file 
	outfile = argv[2]; // output file

	error = read_file(infile, &text, &text_len);
	IFERROR(error);

	/* Possible options:
	   "-a x": indicates the behaviour of FM-index with the pointer 'text'.
			   x == 0 FM-index uses 'text' directly to build the suffix array.
					  This means that you are responsable to allocate 'length+overshoot' 
					  bytes for the text instead of 'length' bytes. You must include
					  ds_ssort.h. See function read_file();
			   x == 1 FM-index frees the allocated memory for the 'text'. overshoot 
	                  and ds_ssort.h are not necessary.
			   x == 2 FM-index makes its internal copy of 'text'. After the call, 
	                  'text' is available. overshoot and ds_ssort.h are not 
		              necessary.
	    -B Bsize: where Bsize is the size in Kbytes of level 1 buckets.
		-b bsize: where bsize is the size in bytes of level 2 buckets. 
				  bsize must divide Bsize*1024;
	    -f frequency: where frequency is a number from 0 to 1 that indicates the 
					  frequency of the marked characters.
	
	   default "-b 512 -B 16 -f 0.02 -a 1"
	   
	   Example of some call to build_index():
	   - build_index(text, text_len, NULL, &index);
		 uses the default parameters.
	   - build_index(text, text_len, "-a 1 -f 0.1", &index);
	   	 tries to mark 10% of the positions instead of 2% but I cannot reuse 'text' 
         after this call. 	 
	*/
	
	error = build_index(text, text_len, "-a 1", &index);
	IFERROR(error);

	error = save_index(index, outfile);
	IFERROR(error);
	
	ulong index_len;
	index_size(index, &index_len);
	fprintf(stdout,"Input: %lu bytes --> Output %lu bytes.\n", text_len, index_len);
	fprintf(stdout,"Overall compression --> %.2f%% (%.2f bits per char).\n\n",
     			(100.0*index_len)/text_len, (index_len*8.0)/text_len);

	error = free_index(index);
	IFERROR(error);


	exit(0);
}

/* 
  Opens and reads a text file. 
  This procedure allocates the overshoot space necessary to the ds_ssort library.
*/
int read_file(char *filename, uchar **textt, ulong *length) {

  uchar *text;
  ulong t;
 // ulong overshoot = (ulong) init_ds_ssort(500, 2000); 
  FILE *infile;
  
  infile = fopen(filename, "rb"); // b is for binary: required by DOS
  if(infile == NULL) {
	  	fprintf (stderr, "Error: cannot open file %s for reading\n", filename);
	  	exit(1);
  }
  
  /* store input file length */
  if(fseek(infile,0,SEEK_END) !=0 ) {
	  	fprintf (stderr, "Error: cannot read file %s for reading\n", filename);
	  	exit(1);
  }
  *length = ftell(infile);
  
  text = malloc((*length) + overshoot); 
  if(text == NULL) 	{
		fprintf (stderr, "Error: cannot allocate %lu bytes\n", (*length) + overshoot);
	    exit(1);
  }
  
  /* read text in one sweep */
  rewind(infile);
  t = fread(text, sizeof(*text), (size_t) *length, infile);
  if(t!=*length) {
	  	fprintf (stderr, "Error: cannot read file %s for writing\n", filename);
	  	exit(1);
  }
  *textt = text;
  fclose(infile);
  return 0;
}


void print_usage(char * progname) {
	fprintf(stderr, "Usage: %s <source file> <index file>\n", progname);
	exit(1);
}
