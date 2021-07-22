#include "../include/btree.h"
#include "../include/funcoes-gerais.h"
#include "../include/linha.h"
#include "../include/veiculo.h"
#include "../include/convertePrefixo.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
            TRABALHO DESENVOLVIDO PARA A DISCIPLINA SCC-0215 ORGANIZACAO DE
   ARQUIVOS

    INTEGRANTES:
        ANTONIO RODRIGUES RIGOLINO - 11795791
        MATHEUS VENTURA DE SOUSA - 11345541

*/
int main() {
  int num_func; // indica qual funcionalidade sera pedida do programa
  scanf("%d", &num_func);

  char filenamesrc[50] = "\0";
  char filenamedest[50] = "\0";
  char filenameindice[50] = "\0";
  char tmp[50] = "\0";

  s_file_t *filesrc = NULL, *filedest = NULL, *fileindice = NULL;
  db_t *db = NULL, *searchResult = NULL;
  header_t *header = NULL;
  dataReg_t *registro = NULL;
  int offset = 0, find_key_int;
  int nRegistros = 0;
  BTreeHeader_t *Btreeheader = NULL;
  
  ll RegReference = -1; 
  char searchfieldname[30] = "\0";
  char find_key[100] = "\0";

  strcpy(tmp, FILEPATH);

  switch (num_func) { // para melhor eficiencia usa-se um switch
  case 1: {
    char filenamecsv[20];
    char filenamebin[20];
    scanf("%s %s", filenamecsv, filenamebin);
    scanf("%*c");
    // writeBin - parâmetros:
    // - nome de um arquivo CSV com o formato especificado já existente, ou
    // seja, modo "r"
    // - nome para um arquivo binário que será gerado a partir desse csv, ou
    // seja, modo "wb+", o + é necessário para manipulações no header
    writeBin(filenamecsv, filenamebin);
    break;
  }
  case 2:
    // Manipulação de string do caminho do arquivo origem
    scanf("%s", filenamesrc);

    filesrc = openfile(filenamesrc, "r");
    if (filesrc != NULL) {
      // readDBfromCSV - parâmetros:
      //  - ponteiro da struct que contém o arquivo origem
      //  - número de registros: para indicar o infinito, foi usado o nº:
      //  10000000
      db = readDBfromCSV(filesrc, 10000000);

      closefile(filesrc);

      // Manipulação de string do caminho do arquivo destino
      scanf("%s", filenamedest);

      filedest = openfile(filenamedest, "wb");

      if (filedest != NULL) {
        // Escreve o db no arquivo destino.
        writeDB(filedest, db, 0);

        closefile(filedest);

        binarioNaTela(filenamedest);
      }
    }
    break;
  case 3: {
    char filenamebin[20];
    scanf("%s", filenamebin);
    scanf("%*c");
    // printBin - parâmetros
    // - nome do arquivo binário (JÁ EXISTENTE) que terá seus conteúdos lidos e
    // será impresso, ou seja, modo "rb".
    printBin(filenamebin);
    break;
  }
  case 4:
    // Manipulação de string do caminho do arquivo origem
    scanf("%s", filenamesrc);

    filesrc = openfile(filenamesrc, "rb");

    if (filesrc != NULL) {
      db = readDBfromBIN(filesrc);

      closefile(filesrc);

      // mostra na tela todos os registros encontrados no arquivo destino com
      // exceção dos excluidos
      printSearchResult(db);
    }
    break;
  case 5: {
    char filenamebin[20];
    char campo[50];
    scanf("%s", filenamebin);
    scanf("%*c");
    scanf("%s", campo);
    scanf("%*c");
    // searchPrint - parâmetros:
    // - nome de um arquivo binário no qual já estão contidos os registros, ou
    // seja, modo "rb"
    // - string que define o campo procurado. O valor do campo será coletado
    // dentro da função
    searchPrint(filenamebin, campo);
    break;
  }
  case 6:
    // Manipulação de string do caminho do arquivo origem
    scanf("%s", filenamesrc);

    filesrc = openfile(filenamesrc, "rb");

    if (filesrc != NULL) {
      db = readDBfromBIN(filesrc);

      closefile(filesrc);

      fgetc(stdin);
      scanf("%s ", searchfieldname);
      scan_quote_string(find_key);

      // procura o campo desejado
      searchResult = searchAttrib(db, searchfieldname, find_key);
      if (searchResult != NULL)
        searchResult->header = db->header;

      // Mostra na tela todos os resultados encontrados, com exceção dos
      // removidos
      printSearchResult(searchResult);
    }
    break;
  case 7: {
    char filenamebin[20];
    int num_inserts;
    scanf("%s %d", filenamebin, &num_inserts);
    scanf("%*c");
    // insert_num_reg - parâmetros:
    // - número de inserções que serão realizadas
    // - nome do arquivo binário já existente que será o destino das inserções,
    // ou seja, modo "rb+" pois o arquivo já existe e precisamos ler E escrever.
    insert_num_reg(num_inserts, filenamebin);
    break;
  }
  case 8:
    // Manipulação de string do caminho do arquivo destino
    scanf("%s", filenamedest);

    scanf("%d", &nRegistros);
    fgetc(stdin);

    filedest = openfile(filenamedest, "rb+");

    if (filedest != NULL) {
      // posiciona o ponteiro do arquivo para depois do status
      fseek(filedest->fp, 1, SEEK_SET);
      header = readHeaderfromBIN(filedest);

      db = readDBfromStdin(nRegistros, header);

      header = NULL;

      // posiciona o ponteiro no início do arquivo
      fseek(filedest->fp, 0, SEEK_SET);
      writeDB(filedest, db, 1);

      closefile(filedest);

      binarioNaTela(filenamedest);
    }
    break;
  case 9: {
    scanf("%s", filenamesrc);

    filesrc = openfile(filenamesrc, "rb");

    if (filesrc == NULL || filesrc->fp == NULL)
    {
      return 0;
    }

    scanf("%s", filenamedest);

    filedest = openfile(filenamedest, "wb+");
    
    
    Btreeheader = BtreeCreate(filedest);


    
    int cont = 0;
    int regnum = checkRegnum(filesrc->fp);
    fseek(filesrc->fp,175,SEEK_SET);//vamos para o começo dos registros


    while (cont<regnum)
    {
      regv_t *registro = get_reg_bin(filesrc->fp);//coletamos o registro
      if (registro->removido == '1')
      {
        int codPrefixo = convertePrefixo(registro->prefixo);
        int64_t offset = ftell(filesrc->fp)-registro->tamanhoRegistro-5;
        InsertInBTreeManager(Btreeheader, codPrefixo,offset,registro->removido,filedest);
        free(registro);
        cont++;
      }
      
    }
    Btreeheader->status = '1';

    InOrderTraversal(Btreeheader->noRaiz, filedest);

    BTreeWriteHeader(Btreeheader, filedest);

    closefile(filesrc);
    closefile(filedest);
    binarioNaTela(filenamedest);
    break;
    break;
  }
  case 10: {

    scanf("%s", filenamesrc);

    filesrc = openfile(filenamesrc, "rb");
    
    scanf("%s", filenamedest);

    filedest = openfile(filenamedest, "wb+");

    header = readHeaderfromBIN(filesrc);

    Btreeheader = BtreeCreate(filedest);

    
    while ((offset = ftell(filesrc->fp)) && (registro = readRegfromBIN(filesrc)) != NULL) {
      InsertInBTreeManager(Btreeheader, registro->codLinha, offset, registro->removido, filedest);
      free(registro);
      registro = NULL;
    }
    Btreeheader->status = '1';

    InOrderTraversal(Btreeheader->noRaiz, filedest);

    BTreeWriteHeader(Btreeheader, filedest);

    closefile(filesrc);
    closefile(filedest);
    binarioNaTela(filenamedest);
    break;
  }
  case 11: {
    scanf("%s", filenamesrc);

    filesrc = openfile(filenamesrc, "rb");

    scanf("%s", filenamedest);

    filedest = openfile(filenamedest, "rb");

    if (filedest->fp == NULL || filesrc->fp == NULL)
    {
      return 0;
    }
    
    char campo[20];
    scanf("%s",campo);

    char aux[20];
    scan_quote_string(aux);
    int codPrefixo = convertePrefixo(aux);

    Btreeheader = BTreeReadHeader(filedest);

    offset = SearchInBTree(Btreeheader->noRaiz, codPrefixo, filedest);
    if (offset!=-1)
    {
      fseek(filesrc->fp,offset,SEEK_SET);
      regv_t* r = get_reg_bin(filesrc->fp);
      printReg(*r);
    }else
    {
      printf("Registro inexistente.\n");
      return 0;
    }
    
    

    break;
  }
  case 12: {

    scanf("%s", filenamesrc);

    filesrc = openfile(filenamesrc, "rb");

    scanf("%s", filenamedest);

    filedest = openfile(filenamedest, "rb");

    if(filesrc != NULL && filesrc->fp != NULL && filesrc->consistenciaDoArquivo == '1' && filedest != NULL && filedest->fp != NULL && filedest->consistenciaDoArquivo == '1'){
      scanf("%s", tmp);

      scanf("%d", &find_key_int);

      if(filedest->EmptyFile == '0'){
        Btreeheader = BTreeReadHeader(filedest);

        offset = SearchInBTree(Btreeheader->noRaiz, find_key_int, filedest);
      }
      if(offset != NIL && filesrc->EmptyFile == '0' && filedest->EmptyFile == '0'){
        header = readHeaderfromBIN(filesrc);
        fseek(filesrc->fp, offset, SEEK_SET);
        registro = readRegfromBIN(filesrc);
        printRegistro(header, registro);
      }
      else{
        printf("Registro inexistente.");
      }
    }
    else{
      printf("Falha no processamento do arquivo.");
    }
    break;
  }
  case 13: {

    int numInsert = 0;

    scanf("%s", filenamesrc);

    filesrc = openfile(filenamesrc, "rb+");

    scanf("%s", filenamedest);

    filedest = openfile(filenamedest, "rb+");

    scanf("%d",&numInsert);
    scanf("%*c");
    if (filesrc == NULL || filesrc->fp == NULL || filesrc->consistenciaDoArquivo == '0' || filedest == NULL || filedest->fp == NULL || filedest->consistenciaDoArquivo == '0')
    {
      return 0;
    }

    Btreeheader = BTreeReadHeader(filedest);

    fwrite("0", sizeof(char), 1, filedest->fp);
    fwrite("0", sizeof(char), 1, filesrc->fp);

    for (int i = 0; i < numInsert; i++)
    {
      regv_t* registro = (regv_t*)malloc(sizeof(regv_t));
      getReg(registro);
      int64_t offset = checkOffset(filesrc->fp);
      insertBin(*registro,filesrc->fp);
      int codPrefixo = convertePrefixo(registro->prefixo);
      InsertInBTreeManager(Btreeheader,codPrefixo,offset,registro->removido,filedest);
      free(registro);
    }
    
    BTreeWriteHeader(Btreeheader, filedest);
    closefile(filesrc);
    closefile(filedest);
    binarioNaTela(filenamedest);

    break;
  }
  case 14: {

    scanf("%s", filenamesrc);

    filesrc = openfile(filenamesrc, "rb+");

    scanf("%s", filenamedest);

    filedest = openfile(filenamedest, "rb+");

    if (filesrc != NULL && filesrc->fp != NULL && filesrc->consistenciaDoArquivo == '1' && filedest != NULL && filedest->fp != NULL && filedest->consistenciaDoArquivo == '1'){
      scanf("%d", &nRegistros);

      fgetc(stdin);

      fwrite("0", sizeof(char), 1, filedest->fp);
      fwrite("0", sizeof(char), 1, filesrc->fp);

      fseek(filesrc->fp, 0, SEEK_END);

      header = readHeaderfromBIN(filesrc);

      Btreeheader = BTreeReadHeader(filedest);

      for(int i = 0; i < nRegistros; i++){
        offset = ftell(filesrc->fp);
        registro = readRegfromStdin();
        writeReg(filesrc, registro);
        if(registro->removido == '1') header->nroRegistros++;
        else header->nroRegRemovidos++;
        InsertInBTreeManager(Btreeheader, registro->codLinha, offset, registro->removido, filedest);
      }
      header->byteProxReg = ftell(filesrc->fp);
      fseek(filesrc->fp, 0, SEEK_SET);
      writeHeader(header, filesrc);
      BTreeWriteHeader(Btreeheader, filedest);
    
      closefile(filesrc);
      closefile(filedest);
      binarioNaTela(filenamedest);
    }
    else{
      printf("Falha no processamento do arquivo.");
    }
    break;
  }
  case 15:{
    break;
  }
  case 16:{
    scanf("%s", filenamesrc);
    scanf("%s", filenamedest);
    fgets(tmp, 19, stdin);
    scanf("%s", filenameindice);

    filesrc = openfile(filenamesrc, "rb+");
    filedest = openfile(filenamedest, "rb+");
    fileindice = openfile(filenameindice, "rb+");

    if (filesrc == NULL || filesrc->fp == NULL || filesrc->consistenciaDoArquivo == '0' || filedest == NULL || filedest->fp == NULL || filedest->consistenciaDoArquivo == '0' || fileindice == NULL || fileindice->fp == NULL || fileindice->consistenciaDoArquivo == '0')
    {
      return 0;
    }

    header = readHeaderfromBIN(filedest);

    Btreeheader = BtreeCreate(fileindice);

    int cont = 0;
    int regnum = checkRegnum(filesrc->fp);
    fseek(filesrc->fp, 175, SEEK_SET); //vamos para o começo dos registros

    dataReg_t *registroLinha = NULL;

    int nMatchedRegisters = 0;
    while (cont < regnum)
    {
      regv_t *registro = get_reg_bin(filesrc->fp); //coletamos o registro
      if (registro->removido == '1')
      {
        RegReference = SearchInBTree(Btreeheader->noRaiz, registro->codLinha, fileindice);
        if(RegReference != NIL){
          fseek(filedest->fp, RegReference, SEEK_SET);
          registroLinha = readRegfromBIN(filedest);
          printReg(*registro);
          printRegistro(header, registroLinha);
          printf("\n");
          free(registroLinha);
          registroLinha = NULL;
          nMatchedRegisters++;
        }
        free(registro);
        registro = NULL;
        cont++;
      }
    }
    if(nMatchedRegisters == 0) printf("Registro Inexistente.\n");

    break;
  }
  case 17:{
    break;
  }
  case 18:{
    break;
  }
  case 19:{
    break;
  }
  default:
    printf("ERRO CODIGO DA FUNCIONALIDADE INVALIDO!\n"); // codigo da
                                                         // funcionalidade fora
                                                         // do escopo
    break;
  }

  return 0;
}
