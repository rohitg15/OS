#include <stdio.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#define UNIX_MAX_PATH 255


void handle(int conn_fd)
{
    const int buf_size = 256;
    unsigned char buf[buf_size + 1];
    int nbytes = 0;
    memset(buf, 0, sizeof(buf));

    nbytes = read(conn_fd, buf, buf_size);
    if (nbytes < 0)
    {
        fprintf(stderr, "Error handling connection: %s", strerror(errno));
        goto cleanup;
    }
    buf[nbytes] = 0x0;
    
    printf("client says: %s\n", buf);

    snprintf(buf, buf_size, "Hello from server");
    if (write(conn_fd, buf, strlen(buf)) < 0)
    {
        fprintf(stderr, "Error handling connection: %s", strerror(errno));
        goto cleanup;
    }
cleanup:
    close(conn_fd);
}

int main(int argc, char **argv)
{

    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s socket_filename\n", argv[0]);
        return -1;
    }

    struct sockaddr_un addr;
    socklen_t addr_len;
    int socket_fd = 0, conn_fd = 0;
    pid_t child = 0;
    char *sock_name = argv[1];

    // create a unix domain socket
    socket_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (socket_fd < 0)
    {
        fprintf(stderr, "Error: %s", strerror(errno));
        return -1;
    }

    // bind domain socket
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, sock_name, strlen(sock_name));

    // if unlink fails then there is no such file
    unlink(sock_name);
    if (bind(socket_fd, (struct sockaddr*) &addr, sizeof(struct sockaddr_un)))
    {
        fprintf(stderr, "Error: %s\n", strerror(errno));
        close(socket_fd);
        return -1;
    }
    printf("socket bound to %s\n", sock_name);

    // set queue size for incoming connections
    if (listen(socket_fd, 5))
    {
        goto error;
    }

    // accept incoming connections
    memset(&addr_len, 0, sizeof(addr_len));
    while ((conn_fd = accept(socket_fd, (struct sockaddr *) &addr, &addr_len)) > -1)
    {
        // fork a new process to handle connection
        child = fork();
        if (child < 0)
        {   
            fprintf(stderr, "Error: %s", strerror(errno));
            goto wait_and_error;
        }
        else if (child == 0)
        {
            // inside child process
            handle(conn_fd);
            exit(0);
        }
        else
        {
            // inside parent process
            close(conn_fd);
        }
    }

wait_and_error:
    if(wait(NULL) >= 0)
    {
        goto cleanup;
    }
error:
    fprintf(stderr, "Error: %s", strerror(errno));
cleanup:
    if (unlink(sock_name) < 0)
    {
        fprintf(stderr, "Error: %s\n", strerror(errno));
    }
    close(socket_fd);
    return 0;
}