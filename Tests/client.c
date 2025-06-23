#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <snfs_api.h>
#include <sthread.h>

#define CLI_SOCKET "/tmp/client.socket"
#define SRV_SOCKET "/tmp/server.socket"
#define THREADS 3

sthread_mutex_t print_mutex;

void* tarefa(void* arg) {
    int id = *(int*)arg;
    char nome_ficheiro[64];
    char nome_copia[64];
    char buffer[512];
    int nread;
    unsigned fsize;
    snfs_fhandle_t root, file;

    fprintf(nome_ficheiro, "teste_%d.txt", id);
    fprintf(nome_copia, "copia_%d.txt", id);

    sthread_mutex_lock(print_mutex);
    printf("[Thread %d] Início da tarefa\n", id);
    sthread_mutex_unlock(print_mutex);

    if (snfs_lookup("/", &root, &fsize) != STAT_OK) {
        sthread_mutex_lock(print_mutex);
        fprintf(stderr, "[Thread %d] Erro ao procurar root\n", id);
        sthread_mutex_unlock(print_mutex);
        return NULL;
    }

    if (snfs_create(root, nome_ficheiro, &file) != STAT_OK) {
        sthread_mutex_lock(print_mutex);
        fprintf(stderr, "[Thread %d] Erro ao criar ficheiro\n", id);
        sthread_mutex_unlock(print_mutex);
        return NULL;
    }

    char mensagem[128];
    fprintf(mensagem, "Olá do SNFS, thread %d!", id);
    unsigned newsize;

    if (snfs_write(file, 0, strlen(mensagem), mensagem, &newsize) != STAT_OK) {
        sthread_mutex_lock(print_mutex);
        fprintf(stderr, "[Thread %d] Erro na escrita\n", id);
        sthread_mutex_unlock(print_mutex);
        return NULL;
    }

    if (snfs_read(file, 0, sizeof(buffer) - 1, buffer, &nread) != STAT_OK) {
        sthread_mutex_lock(print_mutex);
        fprintf(stderr, "[Thread %d] Erro na leitura\n", id);
        sthread_mutex_unlock(print_mutex);
        return NULL;
    }
    buffer[nread] = '\0';

    sthread_mutex_lock(print_mutex);
    printf("[Thread %d] Conteúdo lido: %s\n", id, buffer);
    sthread_mutex_unlock(print_mutex);

    if (snfs_copy(nome_ficheiro, nome_copia) != STAT_OK) {
        sthread_mutex_lock(print_mutex);
        fprintf(stderr, "[Thread %d] Erro ao copiar ficheiro\n", id);
        sthread_mutex_unlock(print_mutex);
        return NULL;
    }

    sthread_mutex_lock(print_mutex);
    printf("[Thread %d] Ficheiro copiado com sucesso!\n", id);
    sthread_mutex_unlock(print_mutex);

    return NULL;
}

int main() {
    snfs_init(CLI_SOCKET, SRV_SOCKET);

    sthread_init();
    print_mutex = sthread_mutex_init();

    sthread_t threads[THREADS];
    int ids[THREADS];

    for (int i = 0; i < THREADS; i++) {
        ids[i] = i + 1;
        threads[i] = sthread_create(tarefa, &ids[i], 7);
    }

    for (int i = 0; i < THREADS; i++) {
        sthread_join(threads[i], NULL);
    }

    snfs_finish();
    sthread_mutex_free(print_mutex);
    return 0;
}
