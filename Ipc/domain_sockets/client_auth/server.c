#define _GNU_SOURCE
 
#include <stdio.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <errno.h>


void handle(int client_fd)
{
    const unsigned int BUF_LEN = 256;
    char buf[BUF_LEN + 1];
    struct ucred creds;
    int ucred_length = sizeof(struct ucred);
    if (getsockopt(client_fd, SOL_SOCKET, SO_PEERCRED, &creds, &ucred_length))
    {
        fprintf(stderr, "Error: %s\n", strerror(errno));
        goto client_cleanup;
    }

    printf("Authenticated client : %d, %d, %d\n", creds.pid, creds.uid, creds.gid);
    memset(buf, 0x0, sizeof(buf));
    if (read(client_fd, buf, BUF_LEN) < 0)
    {
        fprintf(stderr, "Error: %s\n", strerror(errno));
        goto client_cleanup;
    }
    printf("Client > %s\n", buf);
    memset(buf, 0x0, sizeof(buf));
    snprintf(buf, BUF_LEN, "hello world from server!");
    if (write(client_fd, buf, BUF_LEN) < 0)
    {
        fprintf(stderr, "Error: %s\n", strerror(errno));
    }

client_cleanup:
    close(client_fd);
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
    const char *sock_name = argv[1];

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
    snprintf(addr.sun_path, sizeof(addr.sun_path),"%s", sock_name);
    if (bind(socket_fd, (struct sockaddr*) &addr, sizeof(struct sockaddr_un)))
    {
        goto error;
    }
    printf("socket bound to %s\n", sock_name);

    // set queue size for incoming connections
    if (listen(socket_fd, 5))
    {
        unlink(sock_name);
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
    if(wait(NULL) < 0)
    {
        goto error;
    }

error:
    
    fprintf(stderr, "Error: %s", strerror(errno));
cleanup:
    if (unlink(sock_name))
    {
        fprintf(stderr, "Error: %s\n", strerror(errno));
    }
    close(socket_fd);
    return 0;
    
}