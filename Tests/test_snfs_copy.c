#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <snfs_api.h>

#define CLI_SOCKET "/tmp/cli_copy.socket"
#define SRV_SOCKET "/tmp/server.socket"

int main() {
    if (snfs_init(CLI_SOCKET, SRV_SOCKET) < 0) {
        perror("snfs_init");
        return 1;
    }

    // Certifique-se de que "teste.txt" existe
    if (snfs_copy("/teste.txt", "/teste_copia.txt") == STAT_OK) {
        printf("[copy] Arquivo copiado com sucesso: teste.txt -> teste_copia.txt\n");
    } else {
        fprintf(stderr, "[copy] Falha ao copiar o arquivo.\n");
    }

    snfs_finish();
    return 0;
}