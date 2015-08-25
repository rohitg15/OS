#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<errno.h>

#define FOR(i,x,n) for(i=x;i<n;i++)


/*

  sample program to demonstrate IPC in linux using pipes
  we fork off a child process, which writes data into the pipe
  the parent process waits for the child to finish writing and reads from the pipe

 */

static int ret;

int main(int argc,char *argv[])
{

  int pipe_fd[2],p;
  ret = pipe(pipe_fd);
  if(ret)
    {
      fprintf(stderr,"pipe() returned error : %s\n",strerror(errno));
      return -1;
    }

  p = fork();
  if (p < 0)
    {
      fprintf(stderr,"fork failed with error : %s\n",strerror(errno));
      close(pipe_fd[0]);
      close(pipe_fd[1]);
      return -1;
    }
  else if (!p)
    {
      // child writes into the pipe, so we first close the read end

      close(pipe_fd[0]);
      const char *msg = "hello world from child";
      
      // this shows that the read call in the parent blocks until some data arrives in the pipe
      // the same is true of write
      sleep(10);
      ret = write(pipe_fd[1],msg,strlen(msg));
      if (ret < 0)
	{
	  fprintf(stderr,"write failed. exit with error %s\n",strerror(errno));
	  close(pipe_fd[1]);
	  exit(-1);
	}
      
       close(pipe_fd[1]);
    }
  else
    {

      // parent waits for the child and reads whatever was written by the child
      
      int status;
      close(pipe_fd[1]);
      printf("in parent\n");
      
      wait(&status);
     
      if (WIFEXITED(status))
	{
	  printf("child has exited with status %d\n",WEXITSTATUS(status));
	}
      
      char buf[100];
      memset(buf,0,sizeof buf);
      ret = read(pipe_fd[0],buf,sizeof buf);
      if (ret < 0)
	{
	  fprintf(stderr,"could not read from the pipe . exit with error %s\n",strerror(errno));
	  close(pipe_fd[0]);
	  exit(-1);
	}
      close(pipe_fd[0]);
      printf("read data from the pipe : %s\n",buf);
      
    }
  

  return 0;
}

