#include <stdio.h>
#include <string.h>
#include <snfs_api.h>

#define CLI "/tmp/test_ping_client.socket"
#define SRV "/tmp/server.socket"

int main() {
    char outmsg[128];

    if (snfs_init(CLI, SRV) < 0) {
        printf("INIT FAILED\n");
        return 1;
    }

    if (snfs_ping("ping-test", 9, outmsg, sizeof(outmsg)) == STAT_OK) {
        printf("Ping success: %s\n", outmsg);
    } else {
        printf("Ping failed\n");
    }

    snfs_finish();
    return 0;
}
