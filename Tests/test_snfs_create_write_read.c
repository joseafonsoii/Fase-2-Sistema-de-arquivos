#include <stdio.h>
#include <string.h>
#include <snfs_api.h>

#define CLI "/tmp/test_create_client.socket"
#define SRV "/tmp/server.socket"

int main() {
    snfs_fhandle_t root, file;
    unsigned fsize;
    int nread;
    char buffer[256];

    snfs_init(CLI, SRV);

    if (snfs_lookup("/", &root, &fsize) != STAT_OK) {
        printf("Lookup root failed\n");
        return 1;
    }

    if (snfs_create(root, "file1.txt", &file) != STAT_OK) {
        printf("Create failed\n");
        return 1;
    }

    char msg[] = "Testing SNFS write/read";
    unsigned new_fsize;
    if (snfs_write(file, 0, strlen(msg), msg, &new_fsize) != STAT_OK) {
        printf("Write failed\n");
        return 1;
    }

    if (snfs_read(file, 0, sizeof(buffer), buffer, &nread) != STAT_OK) {
        printf("Read failed\n");
        return 1;
    }

    buffer[nread] = '\0';
    printf("Read success: %s\n", buffer);

    snfs_finish();
    return 0;
}
