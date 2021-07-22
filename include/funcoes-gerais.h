#ifndef FUNCOESGERAIS_H_
#define FUNCOESGERAIS_H_

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<ctype.h>

#define TRUE 1
#define FALSE 0

#define FILEPATH "./data/"

typedef struct s_file{
    FILE *fp;
    char EmptyFile;
    char consistenciaDoArquivo;
}s_file_t;

void binarioNaTela(char *nomeArquivoBinario);
void scan_quote_string(char *str);
s_file_t *openfile(char *filename, char *openmode);
void closefile(s_file_t *s_file);


#endif