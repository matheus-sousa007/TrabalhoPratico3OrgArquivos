#include "../include/veiculo.h"
#include "../include/funcoes-gerais.h"

struct header_veic
{
    char status;// '0' -> arquivo em risco '1' -> arquivo íntegro
    int64_t byteProxReg;// offset
    int nroRegistros;
    int nroRegRemovidos;
    char descrevePrefixo[18];
    char descreveData[35];
    char descreveLugares[42];
    char descreveLinha[26];
    char descreveModelo[17];
    char descreveCategoria[20];
};



int updateHeader(int status,int64_t offset,int regnum,int regrm,FILE*fp)//atualiza o header, se o valor for diferente de -1 mudamos o campo correspndente.
{
	if (status!=-1)
	{
		fseek(fp,0,SEEK_SET);
		char set = status + 48;
		fwrite(&set,1,1,fp);
	}
	if (offset!=-1)
	{
		fseek(fp,1,SEEK_SET);
		fwrite(&offset,8,1,fp);
	}
	if (regnum!=-1)
	{
		fseek(fp,9,SEEK_SET);
		fwrite(&regnum,4,1,fp);
	}
	if (regrm!=-1)
	{
		fseek(fp,13,SEEK_SET);
		fwrite(&regrm,4,1,fp);
	}
	return 0;
}

void setStatus(int status,FILE*fp)//funcao para facilitar a sintaxe e leitura!
{
	updateHeader(status,-1,-1,-1,fp);
	return;
}

int64_t checkOffset(FILE*fp)//checa no header o offset para o proximo registro
{
	int64_t offset;
	fseek(fp,1,SEEK_SET);
	fread(&offset,8,1,fp);
	return offset;
}

int checkRegnum(FILE*fp)//checa o numero de registros no header
{
	int regnum;
	fseek(fp,9,SEEK_SET);
	fread(&regnum,4,1,fp);
	return regnum;
}

int checkregnumRm(FILE*fp)//checa o numero de removidos no header
{
	int regnum;
	fseek(fp,13,SEEK_SET);
	fread(&regnum,4,1,fp);
	return regnum;
}

int escreveHeader(FILE* fp_csv,FILE*fp_bin)
{
	headv_t header;//inicializamos um header



	header.status='0';//0 enquanto mexemos no arquivo
	fwrite(&header.status,sizeof(char),1,fp_bin);

	header.byteProxReg=175;//colocamos o offset padrao do header
	fwrite(&header.byteProxReg,8,1,fp_bin);

	header.nroRegistros=0;//zeramos o nro de registros
	fwrite(&header.nroRegistros,4,1,fp_bin);

	header.nroRegRemovidos=0;//zeramos o removidos
	fwrite(&header.nroRegRemovidos,4,1,fp_bin);

	rewind(fp_csv);
	fscanf(fp_csv,"%[^,]",header.descrevePrefixo);			//coletamos o prefixo do csv...
	fseek(fp_csv,1,SEEK_CUR);								//pulando a virgula...
	fwrite(header.descrevePrefixo,18,1,fp_bin);

	fscanf(fp_csv,"%[^,]",header.descreveData);				//agora a data...
	fseek(fp_csv,1,SEEK_CUR);
	fwrite(header.descreveData,35,1,fp_bin);

	fscanf(fp_csv,"%[^,]",header.descreveLugares);			//os lugares...
	fseek(fp_csv,1,SEEK_CUR);
	fwrite(header.descreveLugares,42,1,fp_bin);

	fscanf(fp_csv,"%[^,]",header.descreveLinha);			//a linha...
	fseek(fp_csv,1,SEEK_CUR);
	fwrite(header.descreveLinha,26,1,fp_bin);

	fscanf(fp_csv,"%[^,]",header.descreveModelo);			//o modelo...
	fseek(fp_csv,1,SEEK_CUR);
	fwrite(header.descreveModelo,17,1,fp_bin);

	fscanf(fp_csv,"%[^\n]",header.descreveCategoria);		//e finalmente a categoria
	fseek(fp_csv,1,SEEK_CUR);
	fwrite(header.descreveCategoria,20,1,fp_bin);

	setStatus(1,fp_bin);									//ao terminar de escrever no documento mudamos o status para 1
	return 0;
}

regv_t scanReg(FILE* fp)	//	dado um ponteiro para um csv na posição correta, lemos um registro. Retorna o registro lido
{
	regv_t registro;
	char temp[30];
	fscanf(fp,"%[^,]",registro.prefixo);
	if (registro.prefixo[0]=='*')//  caso o registro seja logicamente removido precisamos coletar apenas o prefixo sem o *
	{
		registro.removido = '0';// indicamos que é logicamente removido
		for (int i = 1; i < 5; i++)
		{
			registro.prefixo[i-1]=registro.prefixo[i];//movemos a string 1 para a esquerda
		}
		registro.prefixo[4]='\0';//ultimo valor como \0 assim como orienta o pdf
	}else
	{
		registro.removido = '1';// caso contrário tudo corre normal
	}
	fseek(fp,1,SEEK_CUR);// pulamos a virgula
	fscanf(fp,"%[^,]",registro.data);
	if (strcmp(registro.data,"NULO")==0)// caso a data seja nula precisamos tratar o lixo com '\0' @ @ @ @ @ @
	{
		registro.data[0]='\0';
		for (int i = 1; i < 10; i++)
		{
			registro.data[i]='@';
		}
		
	}
	fseek(fp,1,SEEK_CUR);
	fscanf(fp,"%[^,]",temp);
	registro.quantidadeLugares=atoi(temp);// coletamos tudo no csv como char, portanto, precisamos dar um atoi no vetor temporário "temp"
	fseek(fp,1,SEEK_CUR);
	fscanf(fp,"%[^,]",temp);
	if (strcmp("NULO",temp)==0)
	{
		registro.codLinha=-1; // caso o campo seja NULL como temos um inteiro a convenção é setarmos como -1
	}else
	{
		registro.codLinha=atoi(temp);
	}
	fseek(fp,1,SEEK_CUR);
	fscanf(fp,"%[^,]",temp);
	if (strcmp("NULO",temp)==0)	
	{
		registro.tamanhoModelo=0; // caso nao haja string a ser lida o tamanho é posto como 0 e o ponteiro como NULL
		registro.modelo = NULL;
	}else
	{
		registro.tamanhoModelo = strlen(temp);// caso contrário pegamos o tamanho da varável temporária e alocamos espaço para o ponteiro
		registro.modelo = (char*)calloc(registro.tamanhoModelo,sizeof(char));
		strcpy(registro.modelo,temp);
	}
	fseek(fp,1,SEEK_CUR);
	fscanf(fp,"%[^\n]",temp);
	fseek(fp,1,SEEK_CUR);
	if (strcmp("NULO",temp)==0)	
	{
		registro.tamanhoCategoria=0;
		registro.categoria = NULL;
	}else
	{
		registro.tamanhoCategoria = strlen(temp);
		registro.categoria = (char*)calloc(registro.tamanhoCategoria,sizeof(char));
		strcpy(registro.categoria,temp);
	}
	registro.tamanhoRegistro = 36 + registro.tamanhoCategoria + registro.tamanhoModelo;//aqui geramos o tamanho do registro como 5 bytes  A MAIS ,
	return registro;                                                                   //já que no insert vamos usar o tamanho como offset e depois escrevemos o tamanho -5 adequadamente
}

int insertBin(regv_t registro,FILE*fp)
{
	setStatus(0,fp);									//status 0 pois estamos mexendo no bin
    

	int64_t offset = checkOffset(fp);					//olhamos o offset
	int regnum = checkRegnum(fp);						//olhamos o numero de registros
	int regrm = checkregnumRm(fp);						//olhamos o numero de removidos


	fseek(fp,offset,SEEK_SET);							//pelo offset sabemos onde colocar o proximo registro
	fwrite(&registro.removido,1,1,fp);					//primeiro o removido
	fwrite(&registro.tamanhoRegistro,4,1,fp);			//depois o tamanho do registro
	fwrite(registro.prefixo,5,1,fp);					//prefixo
	fwrite(registro.data,10,1,fp);						//data de entrada
	fwrite(&registro.quantidadeLugares,4,1,fp);			//quantidade de lugares
	fwrite(&registro.codLinha,4,1,fp);					//codigo da linha
	fwrite(&registro.tamanhoModelo,4,1,fp);				//tamanho do campo modelo
	if(registro.tamanhoModelo!=0)
		fwrite(registro.modelo,registro.tamanhoModelo,1,fp); //se o tamanho n for 0 colocamos o modelo
	fwrite(&registro.tamanhoCategoria,4,1,fp);
	if(registro.tamanhoCategoria!=0)
		fwrite(registro.categoria,registro.tamanhoCategoria,1,fp);
	if(registro.removido == '0') regrm++;					//se for removido atualizamos tambem
	if(registro.removido == '1') regnum++;
	offset += registro.tamanhoRegistro+5;					//atualizamos o offset e regnum
	updateHeader(1,offset,regnum,regrm,fp);				//atualizamos o header e setamos o status para 1

	return 0;
}

int countLines(FILE*fp)// funcao para contar as linhas de um csv
{
	char meuChar;
	int count = -1;//começa com -1 para ignorarmos o header
	while (meuChar!=EOF)
	{
		meuChar = getc(fp);
		if(meuChar=='\n') count++;
	}
	return count;
}
regv_t *get_reg_bin(FILE* fp)// dado um ponteiro para um arquivo binario na pos certa, retornamos o registro contido na mesma.
{
	char notEOF = 0;
	regv_t *registro = (regv_t*)malloc(sizeof(regv_t));
	notEOF = fread(&registro->removido,1,1,fp);//			aqui lemos normalmente os campos de tamanho fixo
	if(!notEOF){
		free(registro);
		return NULL;
	}
	fread(&registro->tamanhoRegistro, 4, 1, fp); //			|	|	|	|	|
	fread(&registro->prefixo, 5, 1, fp);		  //				V	V	V	V	V
	fread(&registro->data,10,1,fp);
	fread(&registro->quantidadeLugares,4,1,fp);
	fread(&registro->codLinha,4,1,fp);
	fread(&registro->tamanhoModelo,4,1,fp);			
	if (registro->tamanhoModelo!=0)// se o indicador de tamanho for != de 0 sabemos que existe um campo string
	{
		registro->modelo = (char*)calloc(registro->tamanhoModelo,sizeof(char));// alocamos um espaço
		for (int i = 0; i < registro->tamanhoModelo; i++)
		{
			char meuChar = getc(fp);//		e coletamos do arquivo binario a string
			registro->modelo[i]=meuChar;
		}
		
	}
	fread(&registro->tamanhoCategoria,4,1,fp);
	if (registro->tamanhoCategoria!=0)//		mesmo procedimento do "modelo"
	{
		registro->categoria = (char*)calloc(registro->tamanhoCategoria,sizeof(char));
		for (int i = 0; i < registro->tamanhoCategoria; i++)
		{
			char meuChar = getc(fp);
			registro->categoria[i]=meuChar;
		}
		
	}
	return registro;
}
void printReg(regv_t registro) //	Dado um registro, imprimimos de acordo com o padrao requisitado
{
	//	Definimos este vetor com os nomes dos meses para que seja mais fácil de escrever qual o mes a partir do seu numero (INDEXADO EM 0)
	char meses[12][20] = {
		{"janeiro"},{"fevereiro"},{"março"},{"abril"},{"maio"},{"junho"},{"julho"},{"agosto"},{"setembro"},{"outubro"},{"novembro"},{"dezembro"}
	};
	printf("Prefixo do veiculo: ");// Como o campo não possui \0 imprimimos char por char
	for (int i = 0; i < 5; i++)
	{
		printf("%c",registro.prefixo[i]);
	}
	printf("\n");
	printf("Modelo do veiculo: ");
	if(registro.tamanhoModelo!=0)
	{
		for (int i = 0; i < registro.tamanhoModelo; i++)
		{
			printf("%c",registro.modelo[i]);
		}
		printf("\n");
	}else
	{
		printf("campo com valor nulo\n"); //	tratando dos possiveis campos nulos
	}
	printf("Categoria do veiculo: ");
	if(registro.tamanhoCategoria!=0)
	{
		printf("%s\n",registro.categoria);
	}else
	{
		printf("campo com valor nulo\n");
	}
	printf("Data de entrada do veiculo na frota: ");
	if (registro.data[0]!='\0')
	{
		char dia[3]={registro.data[8],registro.data[9],'\0'}; // 				pegamos da string data o mes, dia e ano para facilitar a impressao
		char ano[5]={registro.data[0],registro.data[1],registro.data[2],registro.data[3],'\0'};
		char mes[3]={registro.data[5],registro.data[6],'\0'};
		int num_mes = atoi(mes);// atoi no numero do mes (INDEXADO EM 1)
		//					______________________|
		//					|
		printf("%s de %s de %s\n",dia,meses[num_mes-1],ano);
	}else
	{
		printf("campo com valor nulo\n");
	}
	printf("Quantidade de lugares sentados disponiveis: ");
	if (registro.quantidadeLugares!=0)
	{
		printf("%d\n",registro.quantidadeLugares);
	}else
	{
		printf("campo com valor nulo\n");
	}
	//printf("\n");
	return;
}
void printBin(char*filename)// dado um arquivo binario imprimimpos todos seus registros nao removidos
{
	FILE* fp = fopen(filename,"rb");//so vamos ler e printar
	int regnum = checkRegnum(fp);
	int cont = 0;
	fseek(fp,175,SEEK_SET);//vamos para o começo dos registros
	if (regnum==0)
	{
		printf("Registro inexistente.\n");
		return;
	}
	while (cont<regnum)//imprimimos o numero existente de registros
	{
		regv_t* registro = get_reg_bin(fp);//coletamos o registro
		if (registro->removido=='1')// se n for removido imprimimos
		{
			printReg(*registro);
			cont++;
		}
		
	}

	return;
}


int getReg(regv_t *registro)// ler um registro da entrada padrao (stdin) para inserir. 
{
	char aux[45];
	scan_quote_string(registro->prefixo);//scan no prefixo com a mesma regra de tratamento do * da função scanReg
	if (registro->prefixo[0]=='*')
	{
		registro->removido = '0';
		for (int i = 1; i < 5; i++)
		{
			registro->prefixo[i-1]=registro->prefixo[i];
		}
		registro->prefixo[4]='\0';
	}else
	{
		registro->removido = '1';
	}
	scan_quote_string(registro->data);//scan da data tratando do campo nulo com \0@@@@@@
	if (strcmp(registro->data,"")==0)
	{
		registro->data[0]='\0';
		for (int i = 1; i < 10; i++)
		{
			registro->data[i]='@';
		}
		
	}
	scan_quote_string(aux);
	registro->quantidadeLugares = atoi(aux);
	scan_quote_string(aux);
	if(strcmp(aux,"")!=0)
	{
		registro->codLinha = atoi(aux);
	}else
	{
		registro->codLinha = -1;
	}
	scan_quote_string(aux);
	if(strcmp(aux,"")==0)
	{
		registro->modelo = NULL;
		registro->tamanhoModelo = 0;
	}else
	{
		registro->tamanhoModelo = strlen(aux);
		registro->modelo = (char*)calloc(registro->tamanhoModelo,sizeof(char));
		strcpy(registro->modelo,aux);
	}
	scan_quote_string(aux);
	if(strcmp(aux,"")==0)
	{
		registro->categoria = NULL;
		registro->tamanhoCategoria = 0;
	}else
	{
		registro->tamanhoCategoria = strlen(aux);
		registro->categoria = (char*)calloc(registro->tamanhoCategoria,sizeof(char));
		strcpy(registro->categoria,aux);
	}
	getchar(); //pulando o \n...
	registro->tamanhoRegistro = 36 + registro->tamanhoCategoria + registro->tamanhoModelo;
	return 0;
}

int insert_num_reg(int num_ins,char*filenamebin)
{
	FILE *fp_bin = fopen(filenamebin,"rb+");//escrite e leitura em binario
	if(fp_bin==NULL){
		printf("Falha no processamento do arquivo.\n");
	}
	if (fgetc(fp_bin)=='0')
	{
		printf("Falha no processamento do arquivo.\n");
		return 0;
	}
	rewind(fp_bin);
	for (int i = 0; i < num_ins; i++)
	{
		regv_t registro;//definimos um registro temporario
		getReg(&registro);//coletamos do stdin
		insertBin(registro,fp_bin);//inserimos
	}
	fclose(fp_bin);
	binarioNaTela(filenamebin);
	return 0;
}
int checkRem(FILE*fp)//checando o campo 'removido' de um registro
{
	char meuChar;
	meuChar = getc(fp);// usamos getc para que retorne EOF caso o final do arquivo seja encontrado
	if (meuChar==EOF)
	{
		return -1;
	}
	return atoi(&meuChar);
}
int checkTam(FILE*fp)// checando o campo 'tamanho' de um registro
{
	int tam;
	fread(&tam,4,1,fp);
	return tam;
}
int getCampo(char*campo,char* valor)// com a string dada no stdin indentificamos qual campo ela se trata
{
	int qual = -1;
	char valores[5][50]={
		{"prefixo"},{"data"},{"modelo"},{"categoria"},{"quantidadeLugares"}
	};
	for (int i = 0; i < 5; i++)
	{
		if (strcmp(valores[i],campo)==0)
		{
			qual = i;
			break;
		}
	}
	scan_quote_string(valor);
	return qual;
}
int64_t searchOffset(FILE* fp,int qual,char* valor)// dado um ponteiro para um arquivo binario na posição correta, o codigo de qual campo é desejado e o valor de dito campo 
{							//		retorne o offset do próximo registro que é compatível, caso contrário (EOF), retorne -1
	int rem = 0;// se o registro for logicamente removido nao vamos ler
	int64_t start = ftell(fp);// pegamos nossa posicao para retornar caso seja o valor certo
	while (rem != -1)
	{
		rem = checkRem(fp);//pegamos o valor do campo 'removido'
		int tam = checkTam(fp);// 	//            'tamanho'
		if (rem==1)
		{
			switch (qual)
			{
			case 0://caso prefixo
			{
				char prefixo[5];
				fread(prefixo,5,1,fp);
				int k=0;
				//strcmp nao estava funcionando (suspeto pelo fato do campo nao possuir \0)
				for (int i = 0; i < 5; i++)
				{
					if (prefixo[i]==valor[i])
					{
						k++;
					}
					
				}
				if (k==5)
				{
					return start;
				}
				break;	
			}
			case 1://caso data
			{
				fseek(fp,5,SEEK_CUR);//pulamos prefixo
				char data[10];
				fread(data,10,1,fp);
				if (strcmp(valor,data)==0)
				{
					return start;
				}
				break;
			}	
			case 2://caso modelo
			{
				fseek(fp,23,SEEK_CUR);
				int tam_modelo;
				fread(&tam_modelo,4,1,fp);
				if (tam_modelo>0)
				{
					char*modelo = (char*)calloc(tam_modelo,sizeof(char));
					for (int i = 0; i < tam_modelo; i++)
					{
						char meuChar = getc(fp);
						modelo[i]=meuChar;
					}
					if(strcmp(valor,modelo)==0)
					{
						return start;
					}
				}
				break;
			}	
			case 3://caso categoria
			{
				fseek(fp,23,SEEK_CUR);//PRECISAMOS PEGAR O TAMANHO DO MODELO PRIMEIRO
				int tam_modelo;
				fread(&tam_modelo,4,1,fp);
				fseek(fp,tam_modelo,SEEK_CUR);
				int tam_categoria;
				fread(&tam_categoria,4,1,fp);
				if (tam_categoria>0)
				{
					char*categoria = (char*)calloc(tam_categoria,sizeof(char));
					for (int i = 0; i < tam_categoria; i++)
					{
						char meuChar = getc(fp);
						categoria[i]=meuChar;
					}
					if(strcmp(valor,categoria)==0)
					{
						return start;
					}
				}
				break;
			}	
			case 4://caso quantidadeLugares
			{
				fseek(fp,15,SEEK_CUR);
				int num = atoi(valor);
				int quantLug;
				fread(&quantLug,4,1,fp);
				if (num==quantLug)
				{
					return start;
				}
				break;
			}	
			default:
				printf("Falha no processamento do arquivo.\n");// o campo nao existe
				break;
			}
		}
		start = start + tam + 5;//selecionamos o offset do novo registro
		fseek(fp,start,SEEK_SET);//vamos ate la
	}
	return -1;
}
	

int searchPrint(char* filename, char* campo)
{
	int64_t offset = 175;
	FILE *fp = fopen(filename,"rb");//somente leitura em binario
	if(fp==NULL)
	{
		printf("Falha no processamento do arquivo.\n");//arquivo nao existe
		return -1;
	} 
	char valor[40];//	existem varios tipos de dados que o campo pode assumir mas todos eles podem ser resgatados a partir de um vetor
	int qual_campo = getCampo(campo,valor);//identificamos o campo e colocamos o valor no vetor 'valor[40]'
	fseek(fp,offset,SEEK_SET);//colocamos o ponteiro na posicao correta
	while (offset!=-1)// enquanto !EOF...
	{
		offset = searchOffset(fp,qual_campo,valor);//achamos o offset
		if(offset!=-1)
		{
			fseek(fp,offset,SEEK_SET);//vamos ao offset
			regv_t *registro = get_reg_bin(fp);//pegamos um registro do binario
			printReg(*registro);
		} 
	}
	return 0;
}


int writeBin(char* filenamecsv,char* filenamebin)
{
	FILE *fp_csv = fopen(filenamecsv,"r");// so precisamos ler o csv
	if(fp_csv==NULL)
	{
		printf("Falha no processamento do arquivo.\n");
		return -1;
	}
	FILE *fp_bin = fopen(filenamebin,"wb+");// ler e escrever um binario inexistente
	if(fp_bin==NULL)
	{
		printf("Falha no processamento do arquivo.\n");
		return -1;
	}
	int regnum = countLines(fp_csv);// contamos as linhas do csv
	escreveHeader(fp_csv,fp_bin);//escrevemos o header com a descrição dos campos do csv
	int i =0;
	while (i<regnum)//enquanto todos os arquivos nao foram lidos 
	{
		regv_t registro = scanReg(fp_csv);//ler um registro do csv
		insertBin(registro,fp_bin);//inserir
		i++;
	}
	fclose(fp_bin);//fechamos pois acabou a manipulação
	fclose(fp_csv);// idem
	binarioNaTela(filenamebin);
	return 0;
}


void writeBinOrd(FILE* fpOrd,regv_t** binRAM,int regnum)
{
	rewind(fpOrd);
	headv_t h;
	char descPr[18] = "Prefixo do veiculo";
	char descDat[35] = "Data de entrada do veiculo na frota";
	char descLug[42] = "Quantidade de lugares sentados disponiveis";
	char descLinh[26] = "Linha associada ao veiculo";
	char descMod[17] = "Modelo do veiculo";
	char descCat[20] = "Categoria do veiculo";
	h.status = '1';
	h.byteProxReg = 175;
	h.nroRegRemovidos = 0;
	h.nroRegistros = 0;
	fwrite(&h.status,1,1,fpOrd);
	fwrite(&h.byteProxReg,8,1,fpOrd);
	fwrite(&h.nroRegistros,4,1,fpOrd);
	fwrite(&h.nroRegRemovidos,4,1,fpOrd);
	fwrite(descPr,18,1,fpOrd);
	fwrite(descDat, 35, 1, fpOrd);
	fwrite(descLug, 42, 1, fpOrd);
	fwrite(descLinh, 26, 1, fpOrd);
	fwrite(descMod, 17, 1, fpOrd);
	fwrite(descCat, 20, 1, fpOrd);

	int count = 0;
	while (count<regnum)
	{
		insertBin(*binRAM[count],fpOrd);
		count++;
	}
	
//	fclose(fpOrd);
	return;
}

FILE *createSortedFile(FILE *filesrc, char *filenamedest){
	
	FILE *filedest = fopen(filenamedest, "wb+");
	if(filedest == NULL) return NULL;

	int cont = 0;
	int regnum = checkRegnum(filesrc);
	fseek(filesrc, 175, SEEK_SET);

	regv_t **binRAM = (regv_t **)malloc(regnum * sizeof(regv_t *));

	while (cont < regnum)
	{
		regv_t *registro = get_reg_bin(filesrc); //coletamos o registro
		if (registro->removido == '1')				 // é removido?
		{
			binRAM[cont] = registro;
			cont++;
		}
	}

	regv_t *temp;

	for (int i = 0; i < regnum; i++)
	{
		for (int j = 0; j < regnum - 1; j++)
		{
			if (binRAM[j]->codLinha > binRAM[j + 1]->codLinha)
			{
				temp = binRAM[j];
				binRAM[j] = binRAM[j + 1];
				binRAM[j + 1] = temp;
			}
		}
	}
	writeBinOrd(filedest, binRAM, regnum);
	return filedest;
}