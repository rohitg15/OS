#include<stdio.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<unistd.h>


#define FOR(i,x,n) for(i=x;i<n;i++

int main(int argc,char *argv[])
{

  if (argc != 3)
    {
      fprintf(stderr,"ERROR: usage %s ipaddress port_number\n",argv[0]);
      exit(0);
    }

  int server_fd,client_fd;
  struct socketaddr_in server_addr,client_addr;

  /*
    update the properties of the socket_addr struct
    which refers to the address
   */
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(atoi(argv[2]));
  server_addr.sin_addr.s_addr = htonl(argv[1]);
  return 0;
}
