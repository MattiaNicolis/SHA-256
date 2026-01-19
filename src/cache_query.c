#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "../include/common.h"

int main() {
    char my_fifo[MAX_PATH];
    sprintf(my_fifo, "/tmp/query_%d_fifo", getpid());
    mkfifo(my_fifo, 0666);

    request_t req;
    req.type = REQ_QUERY_CACHE;
    strncpy(req.client_fifo, my_fifo, MAX_PATH);

    int sfd = open(SERVER_FIFO, O_WRONLY);
    if (sfd == -1) {
        perror("Server non attivo");
        unlink(my_fifo);
        return 1;
    }
    write(sfd, &req, sizeof(request_t));
    close(sfd);

    printf("--- Stato della Cache del Server ---\n");
    int mfd = open(my_fifo, O_RDONLY);
    char buffer[1024];
    ssize_t n;
    while ((n = read(mfd, buffer, sizeof(buffer) - 1)) > 0) {
        buffer[n] = '\0';
        printf("%s", buffer);
    }

    close(mfd);
    unlink(my_fifo);
    return 0;
}