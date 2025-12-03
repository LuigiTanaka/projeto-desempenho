#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include "mede_time.h"

#define SIZE 100
#define NOME_ARQ_SIZE   50
#define TRUE 1
#define FALSE 0
#define ALPHABET_SIZE 256

// Boyer-Moore-Horspool bad character shift table
void build_bad_char_table(const char *pattern, int pattern_len, int bad_char[ALPHABET_SIZE]) {
    int i;

    // Initialize all positions with pattern length (default shift)
    for (i = 0; i < ALPHABET_SIZE; i++) {
        bad_char[i] = pattern_len;
    }

    // Fill in the actual shift values for characters in the pattern
    // Skip the last character as simplification
    for (i = 0; i < pattern_len - 1; i++) {
        bad_char[(unsigned char)pattern[i]] = pattern_len - 1 - i;
    }
}

// Boyer-Moore-Horspool search
int search_improved(const char *text, int text_len, const char *pattern, int pattern_len, int bad_char[ALPHABET_SIZE]) {
    int occurrences = 0;
    int i = 0;
    int j;

    while (i <= text_len - pattern_len) {
        // Compare from right to left
        j = pattern_len - 1;
        while (j >= 0 && pattern[j] == text[i + j]) {
            j--;
        }

        if (j < 0) {
            // Match found
            occurrences++;
            // Move past this match
            i += pattern_len;
        } else {
            // Mismatch - use bad character rule
            // Always shift based on the character at text[i + pattern_len - 1]
            i += bad_char[(unsigned char)text[i + pattern_len - 1]];
        }
    }

    return occurrences;
}

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
    int size_seq;
    int i_seq;
    int bad_char[ALPHABET_SIZE];


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

    TIMER_CLEAR;
    TIMER_START;

    printf("INICIO\n");
    fflush(stdout);

    fseek(f2,0L,SEEK_END);
    arq2_size=ftell(f2);
    fseek(f2,0L,SEEK_SET);

    fgets(sequencia_busca,SIZE,f1);
    size_sequencia_busca=strlen(sequencia_busca);

    printf("tamanho da sequencia buscada = %d\n",size_sequencia_busca);

    build_bad_char_table(sequencia_busca, size_sequencia_busca, bad_char);

    ocorrencias=0;
    i_seq=0;

    while(i_seq < arq2_size){
        // Leitura de uma linha
        fgets(buff, SIZE, f2);
        size_seq=strlen(buff);
        i_seq+=size_seq;

        if (size_sequencia_busca <= size_seq){
            ocorrencias += search_improved(buff, size_seq, sequencia_busca, size_sequencia_busca, bad_char);
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
