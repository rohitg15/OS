#include <stdio.h>
#include <stdlib.h>
#include <sys/un.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>


#define UNIX_PATH_MAX 255

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        fprintf(stderr, "Error: %s\n", strerror(errno));
        return -1;
    }

    struct sockaddr_un addr;
    int socket_fd = 0;
    const int buf_size = 256;
    char buf[buf_size + 1];
    char *sock_name = argv[1];

    memset(&addr, 0, sizeof(addr));
    memset(buf, 0, sizeof(buf));

    // create a unix domain socket
    socket_fd = socket(PF_UNIX, SOCK_STREAM, 0);
    if (socket_fd < 0)
    {
        fprintf(stderr, "Error creating client socket: %s\n", strerror(errno));
        return -1;
    }

    // connect to the server using the domain socket
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, sock_name, strlen(sock_name));
    if (connect(socket_fd, (struct sockaddr *) &addr, sizeof(struct sockaddr_un) ))
    {
        fprintf(stderr, "Error connecting: %s\n", strerror(errno));
        goto cleanup;
    }

    int nbytes = snprintf(buf, buf_size, "Hello world from client");
    if (write(socket_fd, buf, nbytes) < 0)
    {
        fprintf(stderr, "Error writing to server: %s\n", strerror(errno));
        goto cleanup;
    }

    nbytes = read(socket_fd, buf, buf_size);
    if (nbytes < 0)
    {
        fprintf(stderr, "Error reading from server: %s\n", strerror(errno));
        goto cleanup;
    }
    fprintf(stdout, "nbytes : %d\n", nbytes);
    buf[nbytes] = 0x0;
    fprintf(stdout, "Server says: %s\n", buf);

cleanup:
    close(socket_fd);
    return 0;
}