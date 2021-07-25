#include "../include/linha.h"


// Data structure for dataFile

header_t *readHeaderfromCSV(s_file_t *dataFile) {
  // Verifica se há ponteiro para o arquivo
  if (dataFile->fp == NULL) {
    printf("Arquivo inexistente!\n");
    return NULL;
  }
  // Verifica se o arquivo está vazio
  if (dataFile->EmptyFile == '1') {
    printf("Arquivo vazio!\n");
    return NULL;
  }

  header_t *header = (header_t *)calloc(1, sizeof(header_t));
  // Assumindo inconsistência para próxima vez que a informação for escrita
  header->status = '0';
  header->byteProxReg = 82;
  // Lendo o header do arquivo de entrada
  fscanf(dataFile->fp, "%[^,]", header->descreveCodigo);
  fgetc(dataFile->fp);
  fscanf(dataFile->fp, "%[^,]", header->descreveCartao);
  fgetc(dataFile->fp);
  fscanf(dataFile->fp, "%[^,]", header->descreveNome);
  fgetc(dataFile->fp);
  fscanf(dataFile->fp, "%[^\n]", header->descreveLinha);
  fgetc(dataFile->fp);
  return header;
}

void writeHeader(header_t *header, s_file_t *binFile) {
  // Verifica se há ponteiro para o arquivo
  if (binFile->fp == NULL) {
    printf("Arquivo inexistente!\n");
    return;
  }
  if (header == NULL) {
    printf("Cabeçalho inexistente!\n");
    return;
  }

  // escrevendo o header no binário
  fwrite(&header->status, sizeof(char), 1, binFile->fp);
  fwrite(&header->byteProxReg, sizeof(ll), 1, binFile->fp);
  fwrite(&header->nroRegistros, sizeof(int), 1, binFile->fp);
  fwrite(&header->nroRegRemovidos, sizeof(int), 1, binFile->fp);
  fwrite(header->descreveCodigo, sizeof(char), strlen(header->descreveCodigo),
         binFile->fp);
  fwrite(header->descreveCartao, sizeof(char), strlen(header->descreveCartao),
         binFile->fp);
  fwrite(header->descreveNome, sizeof(char), strlen(header->descreveNome),
         binFile->fp);
  fwrite(header->descreveLinha, sizeof(char), strlen(header->descreveLinha),
         binFile->fp);
}

void writePartialHeader(header_t *header, s_file_t *binFile) {
  // Verifica se há ponteiro para o arquivo
  if (binFile->fp == NULL) {
    printf("Arquivo inexistente!\n");
    return;
  }
  // Verifica se o header existe
  if (header == NULL) {
    printf("Cabeçalho inexistente!\n");
    return;
  }

  // Lê parcialmente o header
  fwrite(&header->status, sizeof(char), 1, binFile->fp);
  fseek(binFile->fp, 8, SEEK_CUR);
  fwrite(&header->nroRegistros, sizeof(int), 1, binFile->fp);
  fwrite(&header->nroRegRemovidos, sizeof(int), 1, binFile->fp);
  fseek(binFile->fp, 0, SEEK_END);
}

dataReg_t *readRegfromCSV(s_file_t *dataFile) {
  // Verifica se há ponteiro para o arquivo
  if (dataFile->fp == NULL) {
    printf("Arquivo inexistente!\n");
    return NULL;
  }
  // Verifica se o arquivo está vazio
  if (dataFile->EmptyFile == '1') {
    printf("Arquivo vazio!\n");
    return NULL;
  }

  char tmp[7] = "\0";

  char checkEOF = 0;
  // lendo o registro do arquivo de entrada
  dataReg_t *reg = (dataReg_t *)calloc(1, sizeof(dataReg_t));
  fscanf(dataFile->fp, "%[^,]", tmp);
  checkEOF = fgetc(dataFile->fp);
  // Coloca o registro como removido caso tenha asterisco no primeiro campo
  if (tmp[0] == '*') {
    reg->removido = '0';
    // Caso o registro tenha sido removido, coloca-se como posição inicial do
    // ponteiro o tmp[1] para conversão do codLinha para inteiro (pula o
    // asterisco)
    reg->codLinha = atoi(tmp + 1);
  }
  // Caso não tenha, coloca a flag como não removido.
  else {
    reg->removido = '1';
    reg->codLinha = atoi(tmp);
  }

  // Lendo o registro do CSV
  fscanf(dataFile->fp, "%c", &reg->aceitaCartao);
  checkEOF = fgetc(dataFile->fp);
  fscanf(dataFile->fp, "%[^,]", reg->nomeLinha);
  checkEOF = fgetc(dataFile->fp);
  // caso o campo tenha "NULO" como valor, atribui-se o valor 0 como tamanho
  reg->tamNome =
      (strcmp(reg->nomeLinha, "NULO") == 0) ? 0 : strlen(reg->nomeLinha);
  fscanf(dataFile->fp, "%[^\n]", reg->corLinha);
  checkEOF = fgetc(dataFile->fp);
  reg->tamCor =
      (strcmp(reg->corLinha, "NULO") == 0) ? 0 : strlen(reg->corLinha);
  reg->tamRegistro = sizeof(reg->codLinha) + sizeof(reg->aceitaCartao) +
                     reg->tamNome + reg->tamCor + 2 * sizeof(int);

  // Caso não tenha conseguido ler, retorna NULO
  if (checkEOF == -1) {
    free(reg);
    reg = NULL;
  }
  return reg;
}

void writeReg(s_file_t *binFile, dataReg_t *reg) {
  // Verifica se há ponteiro para o arquivo
  if (binFile->fp == NULL) {
    printf("Arquivo inexistente!\n");
    return;
  }
  if (reg == NULL) {
    printf("Registro inexistente!\n");
    return;
  }

  // Escrevendo o registro no binário

  fwrite(&reg->removido, sizeof(char), 1, binFile->fp);
  fwrite(&reg->tamRegistro, sizeof(int), 1, binFile->fp);
  fwrite(&reg->codLinha, sizeof(int), 1, binFile->fp);
  fwrite(&reg->aceitaCartao, sizeof(char), 1, binFile->fp);
  fwrite(&reg->tamNome, sizeof(int), 1, binFile->fp);
  if (reg->tamNome != 0)
    fwrite(reg->nomeLinha, sizeof(char), strlen(reg->nomeLinha), binFile->fp);
  fwrite(&reg->tamCor, sizeof(int), 1, binFile->fp);
  if (reg->tamCor != 0)
    fwrite(reg->corLinha, sizeof(char), strlen(reg->corLinha), binFile->fp);
}

db_t *readDBfromCSV(s_file_t *dataFile, int nRegistros) {
  // Verifica se há ponteiro para o arquivo
  if (dataFile->fp == NULL) {
    printf("Falha no processamento do arquivo.\n");
    return NULL;
  }
  // Verifica se o arquivo está vazio
  if (dataFile->EmptyFile == '1') {
    // printf("Arquivo vazio!\n");
    return NULL;
  }

  // Lê o banco de dados do CSV

  db_t *db = (db_t *)calloc(1, sizeof(db_t));
  db->Regdatabase = (dataReg_t **)calloc(DB_BUFFER, sizeof(dataReg_t *));

  db->header = readHeaderfromCSV(dataFile);
  dataReg_t *tmp = NULL;

  int pos = 0;
  // Nesse while há condição para limite de número de registros e tambem há a
  // condição de ler até acabarem os registros
  while (pos < nRegistros && (tmp = readRegfromCSV(dataFile)) != NULL) {
    db->Regdatabase[pos] = tmp;
    if (db->Regdatabase[pos]->removido == '0')
      db->header->nroRegRemovidos++;
    else
      db->header->nroRegistros++;
    pos++;
  }
  // printf("%d\n%d\n", db->header->nroRegistros, db->header->nroRegRemovidos);
  return db;
}

dataReg_t *readRegfromStdin() {
  dataReg_t *reg = (dataReg_t *)calloc(1, sizeof(dataReg_t));

  char tmp[7] = "\0";

  scanf("%s", tmp);
  // Caso o primeiro campo seja marcado com asterisco, marca o registro como
  // removido
  if (tmp[0] == '*') {
    reg->removido = '0';
    reg->codLinha = atoi(tmp + 1);
  }
  // Caso contrário, marca-o como não removido
  else {
    reg->removido = '1';
    reg->codLinha = atoi(tmp);
  }

  // Lendo o registro a partir do stdin

  fgetc(stdin);
  scan_quote_string(tmp);
  reg->aceitaCartao = tmp[0];
  fgetc(stdin);
  scan_quote_string(reg->nomeLinha);
  if (strcmp(reg->nomeLinha, "NULO") != 0)
    reg->tamNome = strlen(reg->nomeLinha);
  fgetc(stdin);
  scan_quote_string(reg->corLinha);
  if (strcmp(reg->corLinha, "NULO") != 0)
    reg->tamCor = strlen(reg->corLinha);
  reg->tamRegistro = sizeof(reg->codLinha) + sizeof(reg->aceitaCartao) +
                     reg->tamNome + reg->tamCor + 2 * sizeof(int);
  return reg;
}

db_t *readDBfromStdin(int nRegistros, header_t *header) {
  // Caso não tenha registros a serem lidos, retorna NULO
  if (nRegistros == 0)
    return NULL;

  db_t *db = (db_t *)calloc(1, sizeof(db_t));
  db->Regdatabase = (dataReg_t **)calloc(DB_BUFFER, sizeof(dataReg_t *));

  // mesclando o header já lido com o banco de dados
  db->header = header;
  // Lendo os registros
  for (int pos = 0; pos < nRegistros; pos++) {
    db->Regdatabase[pos] = readRegfromStdin();
    if (db->Regdatabase[pos]->removido == '0')
      db->header->nroRegRemovidos++;
    else
      db->header->nroRegistros++;
  }
  // printf("%d\n%d\n", db->header->nroRegistros, db->header->nroRegRemovidos);
  return db;
}

void writeDB(s_file_t *binFile, db_t *db, char writePartialHeaderFlag) {
  // Verifica se há ponteiro para o arquivo
  if (binFile->fp == NULL) {
    printf("Arquivo inexistente!\n");
    return;
  }
  if (db == NULL) {
    printf("Banco de dados inexistente!\n");
    return;
  }

  // Escreve o header parcialmente caso a flag esteja ativa
  if (writePartialHeaderFlag)
    writePartialHeader(db->header, binFile);
  // Caso contrário, escreve o header completo
  else
    writeHeader(db->header, binFile);

  // Declara o arquivo como não vazio
  binFile->EmptyFile = '0';
  int pos = 0;
  // Escrevendo os registros
  while (db->Regdatabase[pos] != NULL) {
    writeReg(binFile, db->Regdatabase[pos]);
    pos++;
  }
  db->header->byteProxReg = ftell(binFile->fp);
  // Mudando a consistência do arquivo
  fseek(binFile->fp, 0, SEEK_SET);
  fwrite("1", sizeof(char), 1, binFile->fp);
  fwrite(&db->header->byteProxReg, sizeof(ll), 1, binFile->fp);
}

header_t *readHeaderfromBIN(s_file_t *dataFile) {
  // Verifica se há ponteiro para o arquivo
  if (dataFile->fp == NULL) {
    printf("Arquivo inexistente!\n");
    return NULL;
  }
  // Verifica se o arquivo está vazio
  if (dataFile->EmptyFile == '1') {
    printf("Arquivo vazio!\n");
    return NULL;
  }
  if (dataFile->consistenciaDoArquivo == '0')
    return NULL;

  
  char checkNotEOF = 1;
  fread(&checkNotEOF, sizeof(char), 1, dataFile->fp);
  checkNotEOF = 1;
  header_t *header = (header_t *)calloc(1, sizeof(header_t));
  // Lendo o header do binário
  header->status = '0';
  //printf("%ld\n", ftell(dataFile->fp));
  checkNotEOF = fread(&header->byteProxReg, sizeof(ll), 1, dataFile->fp);
  checkNotEOF = fread(&header->nroRegistros, sizeof(int), 1, dataFile->fp);
  checkNotEOF = fread(&header->nroRegRemovidos, sizeof(int), 1, dataFile->fp);
  checkNotEOF = fread(&header->descreveCodigo, sizeof(char), 15, dataFile->fp);
  checkNotEOF = fread(&header->descreveCartao, sizeof(char), 13, dataFile->fp);
  checkNotEOF = fread(&header->descreveNome, sizeof(char), 13, dataFile->fp);
  checkNotEOF = fread(&header->descreveLinha, sizeof(char), 24, dataFile->fp);

  // Caso não tenha conseguido ler, retorna NULO
  if (!checkNotEOF) {
    free(header);
    header = NULL;
  }
  return header;
}

dataReg_t *readRegfromBIN(s_file_t *dataFile) {
  // Verifica se há ponteiro para o arquivo
  if (dataFile->fp == NULL) {
    printf("Arquivo inexistente!\n");
    return NULL;
  }
  // Verifica se o arquivo está vazio
  if (dataFile->EmptyFile == '1') {
    printf("Arquivo vazio!\n");
    return NULL;
  }
  if (dataFile->consistenciaDoArquivo == '0')
    return NULL;

  char checkNotEOF = 1;
  dataReg_t *reg = (dataReg_t *)calloc(1, sizeof(dataReg_t));

  // Lendo o registro do binário.
  checkNotEOF = fread(&reg->removido, sizeof(char), 1, dataFile->fp);
  checkNotEOF = fread(&reg->tamRegistro, sizeof(int), 1, dataFile->fp);
  checkNotEOF = fread(&reg->codLinha, sizeof(int), 1, dataFile->fp);
  checkNotEOF = fread(&reg->aceitaCartao, sizeof(char), 1, dataFile->fp);
  checkNotEOF = fread(&reg->tamNome, sizeof(int), 1, dataFile->fp);
  if (reg->tamNome != 0)
    checkNotEOF =
        fread(&reg->nomeLinha, sizeof(char), reg->tamNome, dataFile->fp);
  checkNotEOF = fread(&reg->tamCor, sizeof(int), 1, dataFile->fp);
  if (reg->tamCor != 0)
    checkNotEOF =
        fread(&reg->corLinha, sizeof(char), reg->tamCor, dataFile->fp);

  // Caso não tenha conseguido ler, retorna NULO
  if (!checkNotEOF) {
    free(reg);
    reg = NULL;
  }
  return reg;
}

db_t *readDBfromBIN(s_file_t *dataFile) {
  // Verifica se há ponteiro para o arquivo
  if (dataFile->fp == NULL) {
    printf("Falha no processamento do arquivo.\n");
    return NULL;
  }
  // Verifica se o arquivo está vazio
  if (dataFile->EmptyFile == '1') {
    printf("Arquivo vazio!\n");
    return NULL;
  }
  // Verifica se o arquivo está consistente
  dataFile->consistenciaDoArquivo = fgetc(dataFile->fp);
  if (dataFile->consistenciaDoArquivo == '0') {
    printf("Falha no processamento do arquivo.\n");
    return NULL;
  }

  fseek(dataFile->fp, 0, SEEK_SET);

  db_t *db = (db_t *)calloc(1, sizeof(db_t));
  // Lendo o header do binário
  db->header = readHeaderfromBIN(dataFile);
  db->header->status = dataFile->consistenciaDoArquivo;
  db->Regdatabase = (dataReg_t **)calloc(DB_BUFFER, sizeof(dataReg_t *));

  dataReg_t *tmp = NULL;

  // Lendo os registros
  int pos = 0;
  while ((tmp = readRegfromBIN(dataFile)) != NULL) {
    if(tmp->removido == '1'){
      db->Regdatabase[pos] = tmp;
      pos++;
    }
  }
  if (pos == 0) {
    free(db->header);
    free(db->Regdatabase);
    free(db);
    db = NULL;
  }
  // printf("%d", pos);
  return db;
}

// A busca pelo código da linha é uma busca sequencial que, assim que é
// encontrada a chave desejada, a busca imediatamente para e retorna o registro
// desejado visto que não ha duplicações para o campo CodLinha
db_t *searchCodLinha(db_t *database, int find_linha) {
  int pos = 0;
  // Flag de campo encontrado
  char resultFound = FALSE;
  db_t *searchResult = NULL;
  // Busca Sequencial pela chave desejada
  while (database->Regdatabase[pos] != NULL && !resultFound) {
    if (database->Regdatabase[pos]->removido == '1' &&
        database->Regdatabase[pos]->codLinha == find_linha) {
      searchResult = (db_t *)calloc(1, sizeof(db_t));
      searchResult->Regdatabase = (dataReg_t **)calloc(1, sizeof(dataReg_t *));
      searchResult->Regdatabase[0] = database->Regdatabase[pos];
      resultFound = TRUE;
    }
    pos++;
  }
  return searchResult;
}

// A busca pelo campo aceita cartão é uma busca sequencial que vai até o fim dos
// registros visto que pode haver duplicidade de dados nesse campo
db_t *searchAceitaCartao(db_t *database, char flagAceitaCartao) {
  int pos = 0;
  db_t *searchResult = (db_t *)calloc(1, sizeof(db_t));
  searchResult->Regdatabase =
      (dataReg_t **)calloc(DB_BUFFER, sizeof(dataReg_t *));
  int pos_res = 0;
  // busca sequencial
  while (database->Regdatabase[pos] != NULL) {
    if (database->Regdatabase[pos]->removido == '1' &&
        database->Regdatabase[pos]->aceitaCartao == flagAceitaCartao) {
      searchResult->Regdatabase[pos_res] = database->Regdatabase[pos];
      pos_res++;
    }
    pos++;
  }
  // Caso não tenha encontrado nada, retorna nulo
  if (pos_res == 0) {
    free(searchResult->Regdatabase);
    free(searchResult);
    searchResult = NULL;
  }
  return searchResult;
}

// A busca pelo nome da linha é uma busca sequencial que vai até o fim dos
// registros visto que pode haver duplicidade de dados neste campo
db_t *searchnomeLinha(db_t *database, char *nomeLinha) {
  int pos = 0;
  db_t *searchResult = (db_t *)calloc(1, sizeof(db_t));
  searchResult->Regdatabase =
      (dataReg_t **)calloc(DB_BUFFER, sizeof(dataReg_t *));
  int pos_res = 0;
  // Busca sequencial
  while (database->Regdatabase[pos] != NULL) {
    if (database->Regdatabase[pos]->removido == '1' &&
        strcmp(database->Regdatabase[pos]->nomeLinha, nomeLinha) == 0) {
      searchResult->Regdatabase[pos_res] = database->Regdatabase[pos];
      pos_res++;
    }
    pos++;
  }
  // Caso não tenha encontrado nada, ele retorna nulo
  if (pos_res == 0) {
    free(searchResult->Regdatabase);
    free(searchResult);
    searchResult = NULL;
  }
  return searchResult;
}

// A busca pela cor da linha é uma busca sequencial que termina após verificar
// todos os registros visto que pode haver duplicidade de dados nesse campo
db_t *searchcorLinha(db_t *database, char *corLinha) {
  int pos = 0;
  db_t *searchResult = (db_t *)calloc(1, sizeof(db_t));
  searchResult->Regdatabase =
      (dataReg_t **)calloc(DB_BUFFER, sizeof(dataReg_t *));
  int pos_res = 0;
  // Busca sequencial
  while (database->Regdatabase[pos] != NULL) {
    if (database->Regdatabase[pos]->removido == '1' &&
        strcmp(database->Regdatabase[pos]->corLinha, corLinha) == 0) {
      searchResult->Regdatabase[pos_res] = database->Regdatabase[pos];
      pos_res++;
    }
    pos++;
  }
  // Caso não tenha encontrado nada, retorna nulo
  if (pos_res == 0) {
    free(searchResult->Regdatabase);
    free(searchResult);
    searchResult = NULL;
  }
  return searchResult;
}

// A função de busca de atributo é uma função que reune e seleciona qual função
// será utilizada
db_t *searchAttrib(db_t *database, char *attribute, char *find_key) {
  db_t *searchResult = NULL;
  if (strcmp(attribute, "codLinha") == 0)
    searchResult = searchCodLinha(database, atoi(find_key));
  else if (strcmp(attribute, "aceitaCartao") == 0)
    searchResult = searchAceitaCartao(database, find_key[0]);
  else if (strcmp(attribute, "nomeLinha") == 0)
    searchResult = searchnomeLinha(database, find_key);
  else if (strcmp(attribute, "corLinha") == 0)
    searchResult = searchcorLinha(database, find_key);
  return searchResult;
}

// Essa função mostra na tela todos os resultados encontrados com exceção dos
// excluidos
void printSearchResult(db_t *searchResult) {
  if (searchResult == NULL) {
    printf("Registro inexistente.\n");
    return;
  }

  int pos = 0;
  // Loop para mostrar os resultados
  while (searchResult->Regdatabase[pos] != NULL) {
    if (searchResult->Regdatabase[pos]->removido == '1') {
      // Printa o codLinha
      printf("%s: %d\n", searchResult->header->descreveCodigo, searchResult->Regdatabase[pos]->codLinha);
      // Printa o nomeLinha
      printf("%s: ", searchResult->header->descreveNome);
      if (searchResult->Regdatabase[pos]->tamNome == 0)
        printf("campo com valor nulo\n");
      else
        printf("%s\n", searchResult->Regdatabase[pos]->nomeLinha);
      // Printa o corLinha
      printf("%s: ", searchResult->header->descreveLinha);
      if (searchResult->Regdatabase[pos]->tamCor == 0)
        printf("campo com valor nulo\n");
      else
        printf("%s\n", searchResult->Regdatabase[pos]->corLinha);
      // Printa o aceita cartão
      printf("%s: ", searchResult->header->descreveCartao);
      switch (searchResult->Regdatabase[pos]->aceitaCartao) {
      case 'S':
        printf("PAGAMENTO SOMENTE COM CARTAO SEM PRESENCA DE COBRADOR\n");
        break;
      case 'N':
        printf("PAGAMENTO EM CARTAO E DINHEIRO\n");
        break;
      case 'F':
        printf("PAGAMENTO EM CARTAO SOMENTE NO FINAL DE SEMANA\n");
        break;
      }
      printf("\n");
    }
    pos++;
  }
}

void printRegistro(header_t *header, dataReg_t *registro){
  if (registro->removido == '1')
  {
    // Printa o codLinha
    printf("%s: %d\n", header->descreveCodigo, registro->codLinha);
    // Printa o nomeLinha
    printf("%s: ", header->descreveNome);
    if (registro->tamNome == 0)
      printf("campo com valor nulo\n");
    else
      printf("%s\n", registro->nomeLinha);
    // Printa o corLinha
    printf("%s: ", header->descreveLinha);
    if (registro->tamCor == 0)
      printf("campo com valor nulo\n");
    else
      printf("%s\n", registro->corLinha);
    // Printa o aceita cartão
    printf("%s: ", header->descreveCartao);
    switch (registro->aceitaCartao)
    {
    case 'S':
      printf("PAGAMENTO SOMENTE COM CARTAO SEM PRESENCA DE COBRADOR\n");
      break;
    case 'N':
      printf("PAGAMENTO EM CARTAO E DINHEIRO\n");
      break;
    case 'F':
      printf("PAGAMENTO EM CARTAO SOMENTE NO FINAL DE SEMANA\n");
      break;
    }
    //printf("\n");
  }
}

// Função de ComparaRegistro para o Qsort

int comparaRegistro(const void *elem1, const void *elem2)
{
  int CodLinharegistro1 = ((dataReg_t *) elem1)->codLinha;
  int CodLinharegistro2 = ((dataReg_t *) elem2)->codLinha;
  if(CodLinharegistro1 < CodLinharegistro2) return 1;
  if(CodLinharegistro1 > CodLinharegistro2) return -1;
  return 0;
}

s_file_t *createSortedLinhaFile(s_file_t *filesrc, char *filenamedest){

  // Abrindo o arquivo Ordenado  
  s_file_t *filedest = openfile(filenamedest, "wb+");
  if(filedest == NULL || filedest->fp == NULL) return NULL;

  // Lendo do arquivo binário de origem
  db_t *db = readDBfromBIN(filesrc);

  db->header->nroRegRemovidos = 0;

   // Algoritmo de Ordenação Bubble Sort para ordenar registros

  dataReg_t *tmp = NULL;

  for(int i = 0; i < db->header->nroRegistros; i++){
    for(int j = 0; j < db->header->nroRegistros - 1; j++){
      if(db->Regdatabase[j]->codLinha > db->Regdatabase[j+1]->codLinha){
        tmp = db->Regdatabase[j];
        db->Regdatabase[j] = db->Regdatabase[j+1];
        db->Regdatabase[j+1] = tmp;
      }
    }
  }

  db->header->status = '0';

  // Escrita dos registros ordenados no arquivo filedest
  writeDB(filedest, db, 0);
  return filedest;
}

