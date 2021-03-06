
// This is the implementation of the LZ-index data structure
// defined in the paper [Arroyuelo, Navarro, and Sadakane - CPM 2006].


#ifndef LZINDEXINCLUDED
#define LZINDEXINCLUDED

#include "lztrie.h"
#include "revtrie.h"
#include "nodemap.h"


// Data structure to report text positions

typedef struct tpos
 {
    uint *SuperBlock;  // array with starting positions of superblocks 
    uint nSuperBlock;  // number of superblocks
    uint nbitsSB;      // number of bits used for starting positions
    uint SBlock_size;  // superblock size
    uint *Offset;      // array of offsets of each phrase within each superblock
    uint nOffset;      // size of the Offset array
    uint nbitsOffs;    // number of bits used per offset
    ulong Tlength;     // text length 
 } *position;



// LZ-index definition

typedef struct 
 {
    lztrie   fwdtrie;  // the LZ78 trie of the text
    revtrie  bwdtrie;  // the trie of the reversed LZ78 phrases
    nodemap  Rev;      // Rev mapping
    nodemap  rmap;     // RNode mapping
    position TPos;     // data structure to get text positions
    ulong    u;        // length of the indexed text
 } lzindex;


// Interface definition
 
// frees lzindex
int free_index(void *index);
// size (in bytes) of LZ-index I
int index_size(void *index, ulong *size);
// length of the indexed text
int get_length(void *index, ulong *text_length);
// creates lzindex over a null-terminated text.
int build_index(byte *text, ulong length, char *build_options, void **index);
// count query
int count(void *index, byte *pattern, ulong length, ulong* numocc);
// text extraction 
int extract(void *index, ulong from, ulong to, byte **snippet, 
            ulong *snippet_length);
// display query    
int display(void *index, byte *pattern, ulong length, ulong numc,
             ulong *numocc, byte **snippet_text, ulong **snippet_lengths);
// locate query
int locate(void *index, byte *pattern, ulong length, 
           ulong **occ, ulong *numocc);
        // writes index to filename fname.*
int save_index(void *index, char *filename);
        //loads index from filename fname.*
int load_index(char *filename, void **index);

char *error_index (int e);

#endif
