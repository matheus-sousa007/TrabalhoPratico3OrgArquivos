#include "../include/btree.h"
#include <stdio.h>

struct BTreeNode
{
    char folha;
    int nroChavesIndexadas;
    int RRNdoNo;
    int keys[4];
    ll dataRegReferences[4];
    int pointers[5];
};

enum RETURN_VALUES{
    PROMOTION=0,
    NO_PROMOTION,
    ERROR
};

enum RETURN_VALUES_SEARCH{
    NOT_FOUND=-1,
};


void BTreeWriteHeader(BTreeHeader_t *header, s_file_t *indexFile)
{
    fseek(indexFile->fp, 0, SEEK_SET);
    fwrite(&header->status, sizeof(char), 1, indexFile->fp);
    fwrite(&header->noRaiz, sizeof(int), 1, indexFile->fp);
    fwrite(&header->RRNProxNo, sizeof(int), 1, indexFile->fp);
    fwrite(header->lixo, sizeof(char), 68, indexFile->fp);
}

BTreeHeader_t *BTreeReadHeader(s_file_t *indexFile)
{
    BTreeHeader_t *header = (BTreeHeader_t *)calloc(1, sizeof(BTreeHeader_t));
    fseek(indexFile->fp, 0, SEEK_SET);
    fread(&header->status, sizeof(char), 1, indexFile->fp);
    fread(&header->noRaiz, sizeof(int), 1, indexFile->fp);
    fread(&header->RRNProxNo, sizeof(int), 1, indexFile->fp);
    fread(header->lixo, sizeof(char), 68, indexFile->fp);
    return header;
}

void BTreeWriteNode(BTreeNode_t *WriteNode, s_file_t *indexFile)
{
    fseek(indexFile->fp, 77 * (WriteNode->RRNdoNo + 1), SEEK_SET);
    fwrite(&WriteNode->folha, sizeof(char), 1, indexFile->fp);
    fwrite(&WriteNode->nroChavesIndexadas, sizeof(int), 1, indexFile->fp);
    fwrite(&WriteNode->RRNdoNo, sizeof(int), 1, indexFile->fp);
    for (int i = 0; i < MAX_CHAVES_INDEXADAS; i++)
    {
        fwrite(&WriteNode->pointers[i], sizeof(int), 1, indexFile->fp);
        fwrite(&WriteNode->keys[i], sizeof(int), 1, indexFile->fp);
        fwrite(&WriteNode->dataRegReferences[i], sizeof(ll), 1, indexFile->fp);
    }
    fwrite(&WriteNode->pointers[MAX_CHAVES_INDEXADAS], sizeof(int), 1, indexFile->fp);
}

BTreeNode_t *BTreeReadNode(s_file_t *indexFile, int RRNnode)
{
    BTreeNode_t *ReadNode = (BTreeNode_t *)calloc(1, sizeof(BTreeNode_t));
    fseek(indexFile->fp, 77 * (RRNnode + 1), SEEK_SET);
    fread(&ReadNode->folha, sizeof(char), 1, indexFile->fp);
    fread(&ReadNode->nroChavesIndexadas, sizeof(int), 1, indexFile->fp);
    fread(&ReadNode->RRNdoNo, sizeof(int), 1, indexFile->fp);
    for (int i = 0; i < MAX_CHAVES_INDEXADAS; i++)
    {
        fread(&ReadNode->pointers[i], sizeof(int), 1, indexFile->fp);
        fread(&ReadNode->keys[i], sizeof(int), 1, indexFile->fp);
        fread(&ReadNode->dataRegReferences[i], sizeof(ll), 1, indexFile->fp);
    }
    fread(&ReadNode->pointers[MAX_CHAVES_INDEXADAS], sizeof(int), 1, indexFile->fp);
    return ReadNode;
}

// Creating B-Tree Header
void BTreeHeaderCreate(s_file_t *indexFile)
{
    BTreeHeader_t *header = (BTreeHeader_t *)calloc(1, sizeof(BTreeHeader_t));
    header->RRNProxNo = 0;
    header->noRaiz = -1;
    header->status = '0';
    strcpy(header->lixo, "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@");
    BTreeWriteHeader(header, indexFile);
}

void cleanNode(BTreeNode_t *node)
{
    for (int i = 0; i < MAX_CHAVES_INDEXADAS; i++)
    {
        node->keys[i] = node->dataRegReferences[i] = node->pointers[i] = NIL;
    }
    node->pointers[MAX_CHAVES_INDEXADAS] = NIL;
    node->nroChavesIndexadas = 0;
}

BTreeNode_t *createNode(int RRNdoNo, char folha)
{
    BTreeNode_t *newNode = (BTreeNode_t *)calloc(1, sizeof(BTreeNode_t));
    newNode->RRNdoNo = RRNdoNo;
    newNode->folha = folha;
    newNode->nroChavesIndexadas = 0;
    cleanNode(newNode);

    return newNode;
}

int SearchInBTree(int RRN, int codLinha, s_file_t *indexFile){
    if(RRN == NIL) return NOT_FOUND;
    else{
        BTreeNode_t *curNode = BTreeReadNode(indexFile, RRN);
        int pos = 0;
        while(codLinha > curNode->keys[pos] && pos < curNode->nroChavesIndexadas) pos++;

        if(codLinha == curNode->keys[pos]) return curNode->dataRegReferences[pos];

        return SearchInBTree(curNode->pointers[pos], codLinha, indexFile);
    }
}

void Split(int *newCodLinha, ll *newDataRegReference, int *promo_L_child, int *newRRN, BTreeNode_t *curNode, int RRNnewNode, int pos, s_file_t *indexFile){
    int keys[MAX_CHAVES_INDEXADAS + 1];
    ll dataRegReferences[MAX_CHAVES_INDEXADAS + 1];
    int pointers[MAX_CHAVES_INDEXADAS + 2];

    memcpy(keys, curNode->keys, sizeof(int) * MAX_CHAVES_INDEXADAS);
    memcpy(dataRegReferences, curNode->dataRegReferences, sizeof(ll) * MAX_CHAVES_INDEXADAS);
    memcpy(pointers, curNode->pointers, sizeof(int) * (MAX_CHAVES_INDEXADAS + 1));

    for(int i = MAX_CHAVES_INDEXADAS - 1; i >= pos; i--){
        keys[i+1] = keys[i];
        dataRegReferences[i+1] = dataRegReferences[i];
        pointers[i+2] = pointers[i+1];
    }

    if(keys[pos] == (* newCodLinha)) printf("Chave duplicada");
    keys[pos] = (* newCodLinha);
    dataRegReferences[pos] = (* newDataRegReference);
    pointers[pos+1] = (* newRRN);
    
    BTreeNode_t *newNode = createNode(RRNnewNode, curNode->folha); 
    (* newCodLinha) = keys[(MAX_CHAVES_INDEXADAS + 1) / 2];
    (* newDataRegReference) = dataRegReferences[(MAX_CHAVES_INDEXADAS + 1) / 2];
    (* newRRN) = newNode->RRNdoNo;
    (* promo_L_child) = curNode->RRNdoNo;
    cleanNode(curNode);
    for(int i = 0; i < 2; i++){
        curNode->keys[i] = keys[i];
        curNode->dataRegReferences[i] = dataRegReferences[i];
        curNode->pointers[i] = pointers[i];

        newNode->keys[i] = keys[i+3];
        newNode->dataRegReferences[i] = dataRegReferences[i+3];
        newNode->pointers[i] = pointers[i+3];
    }

    curNode->pointers[2] = pointers[2];
    newNode->pointers[2] = pointers[5];
    newNode->nroChavesIndexadas = curNode->nroChavesIndexadas = 2;
    BTreeWriteNode(curNode, indexFile);
    BTreeWriteNode(newNode, indexFile);
    free(newNode);
}

void InsertKeyInNode(BTreeNode_t *node, int codLinha, ll dataRegReference,int promo_L_child, int promo_R_child, int position){
    for(int i = node->nroChavesIndexadas - 1; i >= position; i--){
        node->keys[i+1] = node->keys[i];
        node->dataRegReferences[i+1] = node->dataRegReferences[i];
        node->pointers[i+2] = node->pointers[i+1];
    }
    node->keys[position] = codLinha;
    node->dataRegReferences[position] = dataRegReference;
    node->pointers[position+1] = promo_R_child;
    if (node->pointers[position] == NIL)
        node->pointers[position] = promo_L_child;
    node->nroChavesIndexadas++;
}


int InsertInBTree(BTreeHeader_t *header, int current_RRN, int *codLinha, ll *dataRegReference, int *promo_L_child, int *promo_R_child, s_file_t *indexFile){
    // Base case recursion: NULL subtree found. Returning the recursion.
    if(current_RRN == NIL){
        (* promo_R_child) = NIL;
        return PROMOTION;
    }
    // If the node is not leaf then run the recursive insertion
    else{
        BTreeNode_t *curNode = BTreeReadNode(indexFile, current_RRN);

        // Finding the position where the key should insert
        int pos = 0;
        while ((* codLinha) > curNode->keys[pos] && pos < curNode->nroChavesIndexadas) pos++;
        
        // If the key exists in this node then ignore inserting
        if((* codLinha) == curNode->keys[pos]) return ERROR;

        // Call function recursively
        int return_value = InsertInBTree(header, curNode->pointers[pos], codLinha, dataRegReference, promo_L_child, promo_R_child, indexFile);

        // In the backtracking recursion, its necessary to insert the element or ignore the insertion if it hasn't a promotion or the insertion has a ERROR as a return value

        // Ignoring insertion
        if(return_value == NO_PROMOTION || return_value == ERROR) return return_value;
        // If is possible to insert the key in the node then insert the key
        else if(curNode->nroChavesIndexadas < MAX_CHAVES_INDEXADAS){
            InsertKeyInNode(curNode, *codLinha, *dataRegReference, *promo_L_child , *promo_R_child, pos);
            BTreeWriteNode(curNode, indexFile);
            return NO_PROMOTION;
        }
        // Else split the node and promove a key to its parent.
        else{
            Split(codLinha, dataRegReference, promo_L_child, promo_R_child, curNode, header->RRNProxNo, pos, indexFile);
            header->RRNProxNo++;
            return PROMOTION;
        }
    }
}

void InsertInBTreeManager(BTreeHeader_t *header, int codLinha, ll dataRegReference, char removido, s_file_t *indexFile){
    if(header == NULL || removido == '0' || indexFile == NULL || indexFile->fp == NULL) return;

    int promo_R_child = NIL, promo_L_child = NIL;
    int returnvalue = InsertInBTree(header, header->noRaiz, &codLinha, &dataRegReference, &promo_L_child, &promo_R_child, indexFile);

    if(returnvalue == PROMOTION){
        BTreeNode_t *newRoot = createNode(header->RRNProxNo, '0');
        header->noRaiz = newRoot->RRNdoNo;
        header->RRNProxNo++;
        InsertKeyInNode(newRoot, codLinha, dataRegReference, promo_L_child, promo_R_child, 0);
        BTreeWriteNode(newRoot, indexFile);
    }

}


BTreeHeader_t *BtreeCreate(s_file_t *indexFile)
{
    BTreeHeaderCreate(indexFile);
    BTreeHeader_t *header = BTreeReadHeader(indexFile);
    BTreeNode_t *newRoot = createNode(header->RRNProxNo, '1');
    header->RRNProxNo++;
    header->noRaiz = newRoot->RRNdoNo;
    fwrite("00000000000000000000000000000000000000000000000000000000000000000000000000000", sizeof(char), 77, indexFile->fp);
    BTreeWriteNode(newRoot, indexFile);
    free(newRoot);
    return header;
}

void InOrderTraversal(int RRNroot, s_file_t *indexFile)
{
    if (RRNroot == -1)
        return;

    BTreeNode_t *root = BTreeReadNode(indexFile, RRNroot);
    printf("node: %d, folha: %c \nkeys:  %d %d %d %d \nchildren: %d  %d  %d  %d  %d\n\n", root->RRNdoNo, root->folha, root->keys[0], root->keys[1], root->keys[2], root->keys[3], root->pointers[0], root->pointers[1], root->pointers[2], root->pointers[3], root->pointers[4]);
    for (int i = 0; i < root->nroChavesIndexadas; i++)
    {
        InOrderTraversal(root->pointers[i], indexFile);
        //printf("(%d, %lld)", root->keys[i], root->dataRegReferences[i]);
    }
    InOrderTraversal(root->pointers[root->nroChavesIndexadas], indexFile);
}