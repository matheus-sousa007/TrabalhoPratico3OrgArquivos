CC=gcc
LIBS=-Wall -Wextra -Wpedantic -pedantic-errors
CFLAGS=-c -g -O0
VFLAGS=--leak-check=full --track-origins=yes --show-leak-kinds=all --main-stacksize=8388608
BINARY=./database
DEP= ./objects/veiculo.o ./objects/linha.o ./objects/main.o ./objects/funcoes-gerais.o ./objects/btree.o

all: createDirObjects $(DEP)
	$(CC) $(DEP) -o $(BINARY) 

createDirObjects:
	mkdir -p ./objects

./objects/veiculo.o: source/veiculo.c include/veiculo.h include/funcoes-gerais.h
	$(CC) $(CFLAGS) source/veiculo.c $(LIBS)
	mv veiculo.o ./objects/


./objects/linha.o: source/linha.c include/linha.h include/funcoes-gerais.h
	$(CC) $(CFLAGS) source/linha.c $(LIBS)
	mv linha.o ./objects/

./objects/main.o: source/main.c include/veiculo.h include/linha.h include/funcoes-gerais.h
	$(CC) $(CFLAGS) source/main.c $(LIBS)
	mv main.o ./objects/

./objects/funcoes-gerais.o: source/funcoes-gerais.c include/funcoes-gerais.h
	$(CC) $(CFLAGS) source/funcoes-gerais.c $(LIBS)
	mv funcoes-gerais.o ./objects/

./objects/btree.o: include/btree.h source/btree.c
	$(CC) $(CFLAGS) source/btree.c $(LIBS)
	mv btree.o ./objects/

run:
	$(BINARY)

valgrind:
	valgrind $(VFLAGS) $(BINARY)

clear:
	rm -rf ./objects $(BINARY)
