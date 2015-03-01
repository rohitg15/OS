#include<stdio.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<unistd.h>
#include<errno.h>
#include<string.h>


#define FOR(i,x,n) for(i=x;i<n;i++

int main(int argc,char *argv[])
{

  if (argc != 2)
    {
      fprintf(stderr,"ERROR: usage %s port_number\n",argv[0]);
      exit(0);
    }

  int server_fd,client_fd;
  struct sockaddr_in server_addr,client_addr;

  /*
    update the properties of the socket_addr struct
    which refers to the address
   */
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(atoi(argv[1]));
  server_addr.sin_addr.s_addr = INADDR_ANY;

  /*
    create a socket with
    1) Internet Socket
    2) Connection-Oriented stream
    3) follows the TCP/IP protocol stack
   */
  server_fd = socket(AF_INET,SOCK_STREAM,IPPROTO_IP);
  if (server_fd == -1)
    {
      fprintf(stderr,"ERROR occured : %d\n",strerror(errno));
      exit(0);
    }

  /*
    now that we have a socket, we must bind it to a host,port
   */
  int bind_status = bind(server_fd,(struct sockaddr *)&server_addr,sizeof(server_addr));
  if (bind_status == -1)
    {
      close(server_fd);
      fprintf(stderr,"Bind error : %d\n",strerror(errno));
      exit(0);
    }

  // start listening for connections
  listen(server_fd,1024);
  socklen_t client_len = sizeof(client_addr);
      
  while(1)
    { 
      // start accepting connections
      client_fd = accept(server_fd,(struct sockaddr *)&client_addr,&client_len);
      
      if (client_fd == -1)
	{
	  fprintf(stderr,"failed with error : %d unable to accept connections\n",strerror(errno));
	  close(server_fd);
	  exit(0);
	}
      
      
      pid_t process_id = fork();
      
      //close the client's file descriptor on the server
      if (process_id)
	{
	  // we are inside the server
	  printf("inside the main server. pid = %d. closing the client's fd.\n",getpid());
	  close(client_fd);
	}
      else
	{
	  // we are in the child process
	  // child has to close its copy of server_fd
	  close(server_fd);
	  pid_t pid = getpid();// this is the thread group id
	  const char *msg = "Hello World! from server";
	  char buf[64];
	  memset(buf,0,sizeof(buf));
	  int len = strlen(msg);
	  sprintf(buf,"%s. Server process id is %d",msg,pid);

	  write(client_fd,buf,strlen(buf));
	  memset(buf,0,sizeof(buf));
	  read(client_fd,buf,sizeof(buf));
  
	  printf("received : %s from client\n",buf);
	  close(client_fd);
	}
          
      
    }
  close(server_fd);


  return 0;
}
