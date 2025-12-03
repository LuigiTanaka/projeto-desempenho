#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <omp.h>
#include "mede_time.h"

#define SIZE 100
#define NOME_ARQ_SIZE   50
#define TRUE 1
#define FALSE 0

int main(int argc, char *argv[])
{
    char buff[SIZE];
    char sequencia_busca[SIZE];
    char arq1[NOME_ARQ_SIZE];
    char arq2[NOME_ARQ_SIZE];
    int ocorrencias;
    FILE *f1;
    FILE *f2;
    int arq2_size;
    int size_sequencia_busca;

    TIMER_CLEAR;

    // Abrir o arquivo e pegar a palavra
    if (argc == 3){
    	f1 = fopen(argv[1], "r");
    	f2 = fopen(argv[2], "r");
    }

    else{
    	printf("Digite o nome do arquivo 1(seqs a serem buscadas): ");
    	scanf("%s",&arq1);
    	printf("Digite o nome do arquivo 2 (em que as seqs serao buscadas):");
    	scanf("%s",&arq2);
    	f1 = fopen(arq1, "r");
    	f2 = fopen(arq2, "r");
    }

    // Verificacao do nome do arquivo
    if(f1 == NULL){
    	printf("Arquivo 1  nao encontrado\n");
    	exit(1);
    }
    if(f2 == NULL){
    	printf("Arquivo 2  nao encontrado\n");
    	exit(1);
    }


    // Leitura do arquivo 1 linha por linha e busca pela sequencia lida
    // Leitura do arquivo 2 linha por linha ao buscar a sequencia lida do arquivo 1
    TIMER_START;
    printf("INICIO\n");
    fflush(stdout);
    fseek(f2,0L,SEEK_END);
    arq2_size=ftell(f2);
    fseek(f2,0L,SEEK_SET);

    fgets(sequencia_busca,SIZE,f1);
    size_sequencia_busca=strlen(sequencia_busca);
    printf("tamanho da sequencia buscada = %d\n",size_sequencia_busca);

    // Pre-alocacao de memoria para armazenar linhas
    int max_lines = arq2_size / 10 + 1000;  // estimativa do numero de linhas
    char **lines = (char **)malloc(max_lines * sizeof(char *));
    int *line_sizes = (int *)malloc(max_lines * sizeof(int));
    int num_lines = 0;

    // Leitura de todas as linhas do arquivo 2
    while(fgets(buff, SIZE, f2) != NULL && num_lines < max_lines){
        int size_seq = strlen(buff);
        lines[num_lines] = (char *)malloc((size_seq + 1) * sizeof(char));
        strcpy(lines[num_lines], buff);
        line_sizes[num_lines] = size_seq;
        num_lines++;
    }

    ocorrencias = 0;

    // Paralelizacao da busca usando schedule dynamic para balanceamento
    #pragma omp parallel for reduction(+:ocorrencias) schedule(dynamic, 1000)
    for(int idx = 0; idx < num_lines; idx++){
        char *linha = lines[idx];
        int size_seq = line_sizes[idx];
        int achou, j, i;

        if (size_sequencia_busca <= size_seq){
            i = 0;
            while (i <= (size_seq - size_sequencia_busca)) {   //busca da sequencia em uma linha
                achou = TRUE;
                for(j = 0; j < (size_sequencia_busca - 1); j++){
                    if(sequencia_busca[j] != linha[i + j]){
                        i++;
                        achou = FALSE;
                        break;
                    }
                }
                if (achou == TRUE){
                    ocorrencias++;
                    i += size_sequencia_busca - 1;
                }
            }
        }
    }

    TIMER_STOP;
    printf("Tempo: %f \n", TIMER_ELAPSED);
    printf("=======================================\n");
    printf("Total de ocorrencias = %d\n",ocorrencias);
    printf("=======================================\n");
    return 0;

}
