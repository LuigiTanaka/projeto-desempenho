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
    char sequencia_busca[SIZE];
    char arq1[NOME_ARQ_SIZE];
    char arq2[NOME_ARQ_SIZE];
    int ocorrencias;
    FILE *f1;
    int arq2_size;
    int size_sequencia_busca;
    int bad_char[ALPHABET_SIZE];
    int num_threads;

    TIMER_CLEAR;

    // Abrir o arquivo e pegar a palavra
    if (argc == 3){
    	f1 = fopen(argv[1], "r");
    }
    else{
    	printf("Digite o nome do arquivo 1(seqs a serem buscadas): ");
    	scanf("%s",&arq1);
    	printf("Digite o nome do arquivo 2 (em que as seqs serao buscadas):");
    	scanf("%s",&arq2);
    	f1 = fopen(arq1, "r");
    }

    // Verificacao do nome do arquivo
    if(f1 == NULL){
    	printf("Arquivo 1  nao encontrado\n");
    	exit(1);
    }

    fgets(sequencia_busca,SIZE,f1);
    size_sequencia_busca=strlen(sequencia_busca);
    fclose(f1);

    printf("INICIO\n");
    printf("tamanho da sequencia buscada = %d\n",size_sequencia_busca);
    fflush(stdout);

    // Build bad character table once (shared read-only, thread-safe)
    build_bad_char_table(sequencia_busca, size_sequencia_busca, bad_char);

    // Get file size
    if (argc == 3){
        FILE *f2 = fopen(argv[2], "r");
        if(f2 == NULL){
            printf("Arquivo 2  nao encontrado\n");
            exit(1);
        }
        fseek(f2, 0L, SEEK_END);
        arq2_size = ftell(f2);
        fclose(f2);
    }
    else{
        FILE *f2 = fopen(arq2, "r");
        if(f2 == NULL){
            printf("Arquivo 2  nao encontrado\n");
            exit(1);
        }
        fseek(f2, 0L, SEEK_END);
        arq2_size = ftell(f2);
        fclose(f2);
    }

    TIMER_START;

    ocorrencias = 0;

    // Get number of threads
    #pragma omp parallel
    {
        #pragma omp single
        {
            num_threads = omp_get_num_threads();
        }
    }

    // Parallel processing with chunks
    #pragma omp parallel reduction(+:ocorrencias)
    {
        int thread_id = omp_get_thread_num();
        long chunk_size = arq2_size / num_threads;
        long start_pos = thread_id * chunk_size;
        long end_pos = (thread_id == num_threads - 1) ? arq2_size : start_pos + chunk_size;

        FILE *f_local;
        if (argc == 3){
            f_local = fopen(argv[2], "r");
        }
        else{
            f_local = fopen(arq2, "r");
        }

        if(f_local == NULL){
            printf("Erro ao abrir arquivo na thread %d\n", thread_id);
        }
        else{
            char buff[SIZE];

            // Seek to start position
            fseek(f_local, start_pos, SEEK_SET);

            // If not at the beginning, skip to the next newline to align with line boundary
            if (start_pos > 0) {
                fgets(buff, SIZE, f_local);  // Skip partial line
            }

            // Process lines in this chunk
            while(ftell(f_local) < end_pos && fgets(buff, SIZE, f_local) != NULL){
                int size_seq = strlen(buff);

                if (size_sequencia_busca <= size_seq){
                    ocorrencias += search_improved(buff, size_seq, sequencia_busca, size_sequencia_busca, bad_char);
                }
            }

            fclose(f_local);
        }
    }

    TIMER_STOP;
    printf("Tempo: %f \n", TIMER_ELAPSED);
    printf("=======================================\n");
    printf("Total de ocorrencias = %d\n",ocorrencias);
    printf("=======================================\n");
    return 0;
}
