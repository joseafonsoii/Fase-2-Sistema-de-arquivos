#include <stdio.h>
#include <string.h>
#include <snfs_api.h>

#define CLI "/tmp/test_mkdir_client.socket"
#define SRV "/tmp/server.socket"

int main() {
    snfs_fhandle_t root, newdir;
    snfs_dir_entry_t entries[10];
    unsigned fsize, count;

    snfs_init(CLI, SRV);

    if (snfs_lookup("/", &root, &fsize) != STAT_OK) {
        printf("Lookup root failed\n");
        return 1;
    }

    if (snfs_mkdir(root, "mydir", &newdir) != STAT_OK) {
        printf("mkdir failed\n");
        return 1;
    }

    if (snfs_readdir(root, 10, entries, &count) != STAT_OK) {
        printf("readdir failed\n");
        return 1;
    }

    printf("Entries in /:\n");
    for (unsigned i = 0; i < count; i++) {
        printf("  %s\n", entries[i].name);
    }

    snfs_finish();
    return 0;
}
