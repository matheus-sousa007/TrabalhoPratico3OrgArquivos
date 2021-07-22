#ifndef LINHA_H_
#define LINHA_H_

#include "../include/funcoes-gerais.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct header header_t;
typedef struct dataReg dataReg_t;

#define DB_BUFFER 512
typedef long long int ll;

// header_t size = 1 + 8 + 4 + 4 + 15 + 13 + 13 + 24 = 82
typedef struct header
{
  char status;
  ll byteProxReg;
  int nroRegistros;
  int nroRegRemovidos;
  char descreveCodigo[16]; // Tam = 15 + '\0'
  char descreveCartao[14]; // Tam = 13 + '\0'
  char descreveNome[14];   // Tam = 13 + '\0'
  char descreveLinha[25];  // Tam = 24 + '\0'
} header_t;

typedef struct dataReg {
  char removido;
  int tamRegistro;
  int codLinha;
  char aceitaCartao;
  int tamNome;
  char nomeLinha[50]; // Tamanho Variável
  int tamCor;
  char corLinha[20]; // Tamanho Variável
} dataReg_t;

typedef struct db {
  header_t *header;
  dataReg_t **Regdatabase;
} db_t;

db_t *readDBfromCSV(s_file_t *dataFile, int nRegistros);
void writeDB(s_file_t *binFile, db_t *db, char writePartialHeaderFlag);
dataReg_t *readRegfromBIN(s_file_t *dataFile);
db_t *readDBfromBIN(s_file_t *dataFile);
db_t *readDBfromStdin(int nRegistros, header_t *header);
header_t *readHeaderfromBIN(s_file_t *dataFile);
// header_t *readHeaderfromBIN2(s_file_t *binFile);
db_t *searchAttrib(db_t *database, char *attribute, char *find_key);
void printSearchResult(db_t *searchResult);
void printRegistro(header_t *header, dataReg_t *registro);
dataReg_t *readRegfromStdin();
void writeReg(s_file_t *binFile, dataReg_t *reg);
void writeHeader(header_t *header, s_file_t *binFile);

#endif
