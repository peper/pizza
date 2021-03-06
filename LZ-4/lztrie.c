

// Implements the LZtrie data structure

#include "lztrie.h"

	// creates a lztrie structure from a parentheses bitstring,
	// a letter array in preorder, and an id array in preorder,
	// all of which except the latter become owned
        // n is the total number of nodes (n letters/ids, 2n parentheses)

lztrie createLZTrie(uint *string, byte *letters, uint *id, uint n)
 { 
    lztrie T;
    uint i;
    T          = malloc(sizeof(struct slztrie));
    T->data    = string;
    T->pdata   = createParentheses(string,2*n,true,true);
    T->n       = n;
    T->letters = letters;
    // creates the ids permutation
    T->ids     = createPerm(id, n, PARAMETER_T_IDS);
    // boost first access
    T->boost   = malloc(256*sizeof(trieNode));
    for (i=0;i<256;i++) T->boost[i] = NULLT;
    i = 1; // shortcut for first child of root
    while (i != 2*n-1) { // shortcut for its closing parenthesis
       T->boost[T->letters[i-rank(T->pdata->bdata,i)]] = i;
       // shortcut for leftrankLZTrie
       i = findclose(T->pdata,i)+1;
    }
    return T;
 }

	// frees LZTrie structure, including the owned data

void destroyLZTrie(lztrie T)
 { 
    destroyParentheses(T->pdata);
    destroyPerm(T->ids);
    free(T->letters);
    free(T->boost);
    free(T);
 }

	// stores lztrie T on file f

void saveLZTrie(lztrie T, FILE *f)
 { 
    unsigned long long aux;
    // stores n, the number of nodes in LZTrie
    if (fwrite(&T->n,sizeof(uint),1,f) != 1) {
       fprintf(stderr,"Error: Cannot write LZTrie on file\n");
       exit(1);
    }
    // stores the parentheses structure of LZTrie
    saveParentheses(f, T->pdata);
    // stores the letters labeling the edges of the trie, in preorder
    if (fwrite(T->letters,sizeof(byte),T->n,f) != T->n) {
       fprintf(stderr,"Error: Cannot write LZTrie on file\n");
       exit(1);
    }
    // stores the array of elements of the id permutation
    if (savePerm(T->ids, f) == -1) {   
       fprintf(stderr,"Error: Cannot write LZTrie on file\n");
       exit(1);
    }

 }

	// loads lztrie T from file f

lztrie loadLZTrie(FILE *f)
 { 
    lztrie T;
    uint i,*ids;
    unsigned long long aux;
    T = malloc (sizeof(struct slztrie));
    // loads n, the number of nodes in LZTrie
    if (fread(&T->n,sizeof(uint),1,f) != 1) {
       fprintf(stderr,"Error: Cannot read LZTrie from file\n");
       exit(1);
    }
    // loads the parentheses structure of LZTrie
    T->pdata = loadParentheses(f, 2*T->n,true);
    T->data = T->pdata->data;
    // loads the letters labeling the edges of the trie, in preorder
    T->letters = malloc(T->n*sizeof(byte));
    if (fread(T->letters,sizeof(byte),T->n,f) != T->n) {
       fprintf(stderr,"Error: Cannot read LZTrie from file\n");
       exit(1);
    }
    
    T->ids = loadPerm(f);
    
    // boost first access
    T->boost = malloc(256*sizeof(trieNode));
    for (i=0;i<256;i++) T->boost[i] = NULLT;
    i = 1; // shortcut for first child of root
    while (i != 2*T->n-1) { // shortcut for its closing parenthesis
       T->boost[T->letters[i-rank(T->pdata->bdata,i)]] = i;
       // shortcut for leftrankLZTrie
       i = findclose(T->pdata,i)+1;
    }
    return T;
 }


        // letter by which node i descends

byte letterLZTrie(lztrie T, trieNode i)
 { 
    return T->letters[i-rank(T->pdata->bdata,i)]; // shortcut leftrankLZTrie
 }

	// go down by letter c, if possible

#ifdef QUERYREPORT
int JUMPS = 0; // accesses to nodes to run childLZTrie
int CALLS = 0; // calls to childLZTrie
#endif

trieNode childLZTrie(lztrie T, trieNode i, byte c)
 { 
    trieNode j;
    byte tc, *letters;
    bitmap bdata;
    parentheses pdata;
#ifdef QUERYREPORT
    CALLS++;
#endif
    if (i == ROOT) return T->boost[c];
    j = i+1;
    letters = T->letters;
    bdata = T->pdata->bdata;
    pdata = T->pdata;
    while (!bitget1(T->data,j)) { // there is a child here
       tc = letters[j-rank(bdata,j)];
       // shortcut for leftrankLZTrie: is a child by letter c?
       if (tc > c) break;
       if (tc == c) return j;
       j = findclose(pdata,j)+1;
#ifdef QUERYREPORT
       JUMPS++;
#endif
    }
    return NULLT; // no child to go down by c
 }

	// go up, if possible

trieNode parentLZTrie(lztrie T, trieNode i)
 { 
    if (i == ROOT) return NULLT; // parent of root
    if (T->boost[T->letters[i-rank(T->pdata->bdata,i)]] == i) return ROOT;
    return enclose(T->pdata,i);
 }

	// subtree size

uint subtreesizeLZTrie(lztrie T, trieNode i)
 { 
    trieNode j;
    j = findclose(T->pdata,i);
    return (j-i+1)/2;
 }

	// depth

uint depthLZTrie(lztrie T, trieNode i)
 { 
    return excess(T->pdata,i);
 }
   
	// smallest rank in subtree

uint leftrankLZTrie(lztrie T, trieNode i)
 { 
    return i-rank(T->pdata->bdata,i);
 }

	// largest rank in subtree

uint rightrankLZTrie(lztrie T, trieNode i)
 { 
    trieNode j;
    j = findclose(T->pdata,i);
    return j-rank(T->pdata->bdata,j)-1;
 }

	// id of node

uint idLZTrie(lztrie T, trieNode i)
 { 
    uint pos = leftrankLZTrie(T,i);
    return rthLZTrie(T,pos);
 }

	// rth of position

uint rthLZTrie (lztrie T, uint pos)
 {
    return getelemPerm(T->ids,pos);
 }

        // is node i ancestor of node j?

bool ancestorLZTrie(lztrie T, trieNode i, trieNode j)
 { 
    return (i <= j) && (j < findclose(T->pdata,i));
 }

        // next node from i, in preorder, adds/decrements depth accordingly
	// assumes it *can* go on!

trieNode nextLZTrie(lztrie T, trieNode i, uint *depth)
 { 
    uint *data = T->data;
    i++;
    while (bitget1(data,i)) { i++; (*depth)--; }
    (*depth)++;
    return i;
 }


uint sizeofLZTrie(lztrie T)
 {
    return sizeof(struct slztrie) +
           sizeofParentheses(T->pdata) +
           T->n*sizeof(byte) + // letters
           sizeofPerm(T->ids) + // ids
           256*sizeof(trieNode); // boost
 }

uint maxdepthLZTrie(lztrie T)
 {
    trieNode i;
    uint depth, mdepth, j, n;

    i = ROOT;
    depth = mdepth = 0;
    n = T->n;
    for (j=1;j<n;j++) {
       i = nextLZTrie(T,i,&depth);
       if (depth > mdepth) mdepth = depth;
    }
    return mdepth;
 }

