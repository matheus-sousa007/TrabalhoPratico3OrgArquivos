#ifndef VEICULO_H_
#define VEICULO_H_

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<ctype.h>
#include "../include/funcoes-gerais.h"



typedef struct header_veic headv_t;
typedef struct reg_veic regv_t;

struct reg_veic
{
    char removido;// '0' -> registro logicamente '1' -> registro normal
    int tamanhoRegistro;
    int tamanhoModelo;
    int tamanhoCategoria;
    char prefixo[5];
    char data[10]; // data de entrada na frota. formato: AAAA/MM/DD
    int quantidadeLugares;
    int codLinha;
    char* modelo;
    char* categoria;
};

regv_t* get_reg_bin(FILE* fp);
int checkRegnum(FILE*fp);
void printReg(regv_t registro);
int getReg(regv_t *registro);
int insertBin(regv_t registro,FILE*fp);
int64_t checkOffset(FILE*fp);
int writeBin(char* filenamecsv,char* filenamebin);
int insert_num_reg(int num_ins,char*filenamebin);
void printBin(char*filename);
int searchPrint(char* filename, char* campo);

#endif