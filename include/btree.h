#ifndef BTREE_H_
#define BTREE_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/linha.h"
#include "../include/funcoes-gerais.h"

#define TRUE 1
#define FALSE 0
#define NIL -1

#define MAX_CHAVES_INDEXADAS 4

typedef struct BTreeNode BTreeNode_t;

typedef struct BTreeHeader BTreeHeader_t;

typedef long long int ll;

struct BTreeHeader
{
    char status;
    int noRaiz;
    int RRNProxNo;
    char lixo[69];
};

void BTreeHeaderCreate(s_file_t *indexFile);
BTreeHeader_t *BtreeCreate(s_file_t *indexFile);
void InOrderTraversal(int RRNroot, s_file_t *indexFile);
void BTreeWriteHeader(BTreeHeader_t *header, s_file_t *indexFile);
void InsertInBTreeManager(BTreeHeader_t *header, int codLinha, ll dataRegReference, char removido, s_file_t *indexFile);
BTreeHeader_t *BTreeReadHeader(s_file_t *indexFile);
int SearchInBTree(int RRN, int codLinha, s_file_t *indexFile);

#endif
