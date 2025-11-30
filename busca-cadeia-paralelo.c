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
    int i_seq;
    FILE *f1;
    FILE *f2;
    int arq2_size;
    int size_sequencia_busca;
    int size_seq; 

    TIMER_CLEAR;
//    TIMER_START;

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
    ocorrencias=0;
    i_seq=0;
    
    #pragma omp parallel reduction(+:ocorrencias)
    {
        #pragma omp single
        {
            while(i_seq < arq2_size){
                // Leitura de uma linha
                fgets(buff, SIZE, f2);
                // Comparacao da sequencia na linha em arq2 com a seq buscada
                size_seq=strlen(buff);
                i_seq+=size_seq;

                char *linha = strdup(buff);

                #pragma omp task shared(ocorrencias)
                {
                    int achou, j, i;
                    if (size_sequencia_busca <= size_seq){
                        i=0; 
                        while (i<=(size_seq-size_sequencia_busca)) {   //busca da sequencia em uma linha 
                            achou=TRUE;
                            for(j = 0; j < (size_sequencia_busca-1); j++){
                                if(sequencia_busca[j] != linha[i+j]){
                                    i++;
                                    achou = FALSE;
                                    break;
                                }
                            }
                            if (achou == TRUE){
                                ocorrencias++;
                                i+=size_sequencia_busca-1;
                            }
                        }
                    }
                }
                #pragma omp taskwait
            }
        }
    }
    // Fecha os arquivos
    fclose(f1);
    fclose(f2);

    TIMER_STOP;
    printf("Tempo: %f \n", TIMER_ELAPSED);
    printf("=======================================\n");
    printf("Total de ocorrencias = %d\n",ocorrencias);
    printf("=======================================\n");
    return 0;
    
}
