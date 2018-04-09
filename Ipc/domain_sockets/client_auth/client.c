#include <stdio.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s socket_filename\n", argv[0]);
        return -1;
    }

    const unsigned int BUF_LEN = 256;
    char buf[BUF_LEN + 1];
    struct sockaddr_un addr;
    memset(buf, 0x0, sizeof(buf));

    int sock_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sock_fd < 0)
    {
        fprintf(stderr, "Error creating socket: %s\n", strerror(errno));
        return -1;
    }
    
    memset(&addr, 0, sizeof(struct sockaddr_un));
    snprintf(addr.sun_path, sizeof(addr.sun_path), "%s", argv[1]);
    addr.sun_family = AF_UNIX;
    if (connect(sock_fd, (struct sockaddr*) &addr, sizeof(struct sockaddr_un)))
    {
        fprintf(stderr, "Error connecting to: %s\n", strerror(errno));
        goto cleanup;
    }

    snprintf(buf, BUF_LEN, "hello world from client");
    if (write(sock_fd, buf, BUF_LEN) < 0)
    {
        fprintf(stderr, "Error: %s\n", strerror(errno));
        goto cleanup;
    }

    memset(buf, 0x0, sizeof(buf));
    if (read(sock_fd, buf, BUF_LEN) < 0)
    {
        fprintf(stderr, "Error: %s\n", strerror(errno));
        goto cleanup;
    }
    printf("Server > %s\n", buf);

cleanup:
    close(sock_fd);
    return 0;
}