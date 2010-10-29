/* perm.cpp
 * Copyright (C) 2005, Diego Arroyuelo, all rights reserved.
 *
 * Permutation
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#include "perm.h"
#include <cmath>
#include <cassert>
#include <cstdio>
#include <cstdlib>

namespace lz77index{
namespace basics{

int compare(const void *p1, const void *p2) {
  return  ((auxbwd *)p1)->key - ((auxbwd *)p2)->key;
}


perm createPerm(unsigned int *elems, unsigned int nelems, unsigned int t, static_bitsequence_builder * bmb) {
  perm P;
  unsigned int *b, *baux, nextelem, i, j, bptr,
    aux, antbptr,nbwdptrs, elem,nbits, firstelem, cyclesize;
  auxbwd *auxbwdptr;
  P = new struct sperm;
  P->elems  = elems;
  P->nelems = nelems;
  P->nbits  = bits(nelems-1);
  nbits = bits(nelems-1);
  P->t = t;
  if (t==1) {
    P->bwdptrs = new unsigned int[uint_len(nelems,nbits)];
    for(unsigned int m=0;m<uint_len(nelems,nbits);m++)
        P->bwdptrs[m]=0;
    assert(P->bwdptrs!=NULL);
    P->nbwdptrs = nelems;
    for (i=0; i<nelems; i++) {
      unsigned int bg = get_field(elems, nbits, i);
      assert(bg<nelems);
      set_field(P->bwdptrs, nbits, bg, i);
    }
    P->bmap = NULL;
  }
  else {
    auxbwdptr = new auxbwd[(t+((int)ceil((double)nelems/t)))];
    assert(auxbwdptr!=NULL);
    b = new unsigned int[uint_len(nelems,1)];
    for(i=0;i<uint_len(nelems,1);i++)
      b[i]=0;
    assert(b!=NULL);    
    baux = new unsigned int[uint_len(nelems,1)];
    for(i=0;i<uint_len(nelems,1);i++)
      baux[i] = 0;
    assert(baux!=NULL);
    nbwdptrs = 0;
    for (i = 0; i < nelems; i++) {
      if (bitget(baux,i) == 0) {
        nextelem = j = bptr = antbptr = i;
        aux = 0;
        bitset(baux, j);
        cyclesize = 0;
        firstelem = j;
        while ((elem=get_field(elems, nbits, j)) != nextelem) {//P->elems[j]
          j = elem;
          bitset(baux, j);
          aux++;
          if (aux >= t) {
            auxbwdptr[nbwdptrs].key = j;
            auxbwdptr[nbwdptrs++].pointer = bptr;
            antbptr = bptr;
            bptr    = j;
            aux     = 0;
            bitset(b, j);
          }
          cyclesize++;
        }
        if (cyclesize >= t) {
          auxbwdptr[nbwdptrs].key = nextelem;
          auxbwdptr[nbwdptrs++].pointer = bptr;
          bitset(b, nextelem);
        }
      }
    }
    qsort(auxbwdptr, nbwdptrs, sizeof(auxbwd), &compare);
    aux = uint_len(nbwdptrs,P->nbits);
    P->bwdptrs = new unsigned int[aux];
    assert(P->bwdptrs!=NULL);
    for(i=0;i<aux;i++) P->bwdptrs[i] = 0;
    P->nbwdptrs = nbwdptrs;
    for (i = 0; i < nbwdptrs; i++) {
      set_field(P->bwdptrs, nbits, i, auxbwdptr[i].pointer);
      //if(i<5) 
      //  printf(" %d ",get_field(P->bwdptrs,nbits,i));
    }
    //printf("\n");
    P->bmap = bmb->build(b, nelems);
    //delete [] P->bmap;
    delete [] b;
    delete [] (baux);
    delete [] (auxbwdptr);
  }
  return P;
}


void destroyPerm(perm P) {
  delete [] P->elems;
  if (P->bmap) delete P->bmap;
  delete [] P->bwdptrs;
  delete P;
}


// Computes P-1[i]
unsigned int inversePerm(perm P, unsigned int i) {
  unsigned int j, elem;
  if (P->t==1) {
    j = get_field(P->bwdptrs,P->nbits,i); 
  }
  else {
    j = i;
    while (((elem=get_field(P->elems,P->nbits,j)) != i)&&(!P->bmap->access(j)))
    //while (((elem=P->elems[j]) != i)&&(!P->bmap->access(j)))
      j = elem;

    if (elem != i) {
      // follows the backward pointer
      j = get_field(P->bwdptrs, P->nbits, P->bmap->rank1(j-1));
      while ((elem = get_field(P->elems,P->nbits,j))!= i){
        j = elem;
      }
    }
  }
  return j;
}


// gets the ith element of a perm P

unsigned int getelemPerm(perm P, unsigned int i) {
  return get_field(P->elems, P->nbits, i);
}


unsigned int savePerm(perm P, FILE *f) {
  unsigned int aux;
  unsigned int v;

  if (fwrite(&P->nelems,sizeof(unsigned int),1,f) != 1) {
    fprintf(stderr,"Error: Cannot write Permutation on file\n");
    exit(1);
  }

  aux = uint_len(P->nelems,P->nbits);
  if (fwrite(P->elems,sizeof(unsigned int),aux,f) != aux) {
    fprintf(stderr,"Error: Cannot write Permutation on file\n");
    exit(1);
  }

  aux = ((P->nelems+W-1)/W);

  if (P->bmap) {
    v=1;
    if (fwrite(&v,sizeof(unsigned int),1,f) != 1) {
      fprintf(stderr,"Error: Cannot write Permutation on file\n");
      exit(1);
    }
    P->bmap->save(f);
  }
  else {
    v=0;
    if (fwrite(&v,sizeof(unsigned int),1,f) != 1) {
      fprintf(stderr,"Error: Cannot write Permutation on file\n");
      exit(1);
    }
  }

  if (fwrite(&P->nbwdptrs,sizeof(unsigned int),1,f) != 1) {
    fprintf(stderr,"Error: Cannot write Permutation on file\n");
    exit(1);
  }

  aux = uint_len(P->nbwdptrs,P->nbits);
  if (fwrite(P->bwdptrs,sizeof(unsigned int),aux,f) != aux) {
    fprintf(stderr,"Error: Cannot write Permutation on file\n");
    exit(1);
  }
  if (fwrite(&P->t,sizeof(unsigned int),1,f) != 1) {
    fprintf(stderr,"Error: Cannot write Permutation on file\n");
    exit(1);
  }
  return 0;
}


perm loadPerm(FILE *f) {
  unsigned int aux;
  perm P;
  unsigned int v;

  P = new struct sperm;          //(struct sperm*) malloc(sizeof(struct sperm));

  if (fread(&P->nelems,sizeof(unsigned int),1,f) != 1) {
    fprintf(stderr,"Error: Cannot read Permutation from file\n");
    exit(1);
  }
  P->nbits = bits(P->nelems-1);
  aux = uint_len(P->nelems,P->nbits);
  P->elems = new unsigned int[aux];      //(unsigned int *)malloc(aux*sizeof(unsigned int));

  if (fread(P->elems,sizeof(unsigned int),aux,f) != aux) {
    fprintf(stderr,"Error: Cannot read Permutation from file\n");
    exit(1);
  }

  if (fread(&v,sizeof(unsigned int),1,f) != 1) {
    fprintf(stderr,"Error: Cannot read Permutation from file\n");
    exit(1);
  }

  if (v) {
    P->bmap = static_bitsequence::load(f);
  }
  else P->bmap = NULL;

  if (fread(&P->nbwdptrs,sizeof(unsigned int),1,f) != 1) {
    fprintf(stderr,"Error: Cannot read Permutation from file\n");
    exit(1);
  }

  aux = uint_len(P->nbwdptrs,P->nbits);
  P->bwdptrs = new unsigned int[aux];    //(uint*) malloc(aux*sizeof(unsigned int));

  if (fread(P->bwdptrs,sizeof(unsigned int),aux,f) != aux) {
    fprintf(stderr,"Error: Cannot read Permutation from file\n");
    exit(1);
  }

  if (fread(&P->t,sizeof(unsigned int),1,f) != 1) {
    fprintf(stderr,"Error: Cannot read Permutation from file\n");
    exit(1);
  }

  return P;
}


unsigned int sizeofPerm(perm P) {
  return sizeof(struct sperm) +
    ((uint_len(P->nelems,P->nbits))*sizeof(unsigned int)) +
    ((P->bmap)?(P->bmap->size()):0) +
    ((uint_len(P->nbwdptrs,P->nbits))*sizeof(unsigned int));
}

}
}

