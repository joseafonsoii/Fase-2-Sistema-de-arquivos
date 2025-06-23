#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sthread.h>
#include <unistd.h>
#include <snfs_api.h>

#define CLI_SOCKET_BASE "/tmp/client_concurrent_%d.socket"
#define SRV_SOCKET "/tmp/server.socket"
#define NUM_THREADS 4
#define FILENAME "conc_file.txt"

void* client_thread(void* arg) {
    int tid = *(int*)arg;
    char cli_sock[64];
    snprintf(cli_sock, sizeof(cli_sock), CLI_SOCKET_BASE, tid);

    if (snfs_init(cli_sock, SRV_SOCKET) < 0) {
        fprintf(stderr, "[Thread %d] Erro ao inicializar o cliente.\n", tid);
        return NULL;
    }

    snfs_fhandle_t root, file;
    unsigned fsize;
    char msg[64];

    if (snfs_lookup("/", &root, &fsize) != STAT_OK) {
        fprintf(stderr, "[Thread %d] Erro ao procurar diretório raiz.\n", tid);
        snfs_finish();
        return NULL;
    }

    // Só o primeiro thread cria o arquivo
    if (tid == 0) {
        if (snfs_create(root, FILENAME, &file) != STAT_OK) {
            fprintf(stderr, "[Thread %d] Erro ao criar ficheiro.\n", tid);
            snfs_finish();
            return NULL;
        }
        printf("[Thread %d] Ficheiro criado.\n", tid);
    }

    // Todos localizam o mesmo ficheiro
    int retries = 5;
    while (retries--) {
        if (snfs_lookup("/" FILENAME, &file, &fsize) == STAT_OK)
            break;
        sleep(1);
    }

    if (retries <= 0) {
        fprintf(stderr, "[Thread %d] Timeout ao procurar ficheiro.\n", tid);
        snfs_finish();
        return NULL;
    }

    // Cada thread escreve uma mensagem no ficheiro em posições diferentes
    snprintf(msg, sizeof(msg), "Thread %d diz oi!\n", tid);
    unsigned int final_size;
    if (snfs_write(file, tid * 64, strlen(msg), msg, &final_size) != STAT_OK) {
        fprintf(stderr, "[Thread %d] Erro ao escrever.\n", tid);
        snfs_finish();
        return NULL;
    }

    printf("[Thread %d] Mensagem escrita com sucesso.\n", tid);
    snfs_finish();
    return NULL;
}

int main() {
    //pthread_t threads[NUM_THREADS];
    //int tids[NUM_THREADS];
    sthread_init();

    for (int i = 0; i < NUM_THREADS; ++i) {
       // tids[i] = i;
        int *id = malloc(sizeof(int));
        *id = i;
        int prioridade = (i == 0) ? 3 : (i == 1) ? 6 : 10;
        sthread_create(client_thread,id, prioridade);
    }

    for (int i = 0; i < NUM_THREADS; ++i) {
        sthread_join(&client_thread, NULL);
    }

    printf("[Main] Teste de concorrência finalizado.\n");
    sthread_exit(NULL);
    return 0;
}
