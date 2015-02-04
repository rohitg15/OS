#include<stdio.h>
#include<errno.h>
#include<wait.h>

#define MAX_PROCESS_NUM 10
#define FOR(i,x,n) for(i=x;i<n;i++)

int main(int argc,char *argv[])
{
  
  pid_t pids[MAX_PROCESS_NUM];
  int i=0;
  FOR(i,0,MAX_PROCESS_NUM)
    {
      pids[i] = fork();
      if (pids[i] == 0)
  {
    //we are inside the child process

    sleep(10);
    exit(100+i);
  }
    }
  // reap the children by waiting for them individually
  FOR(i,0,MAX_PROCESS_NUM)
    {
      int retpid,status;
      retpid = waitpid(-1,&status,0);
      if (retpid <= 0)
      {
        fprintf(stderr,"Error occured , could not reap child. error - %s\n",strerror(errno));
        continue;
      }

      if (WIFEXITED(status))
      {
        printf("child process with id %d has exited normally with status %d\n",retpid,WEXITSTATUS(status));
      }
      else
      {
        printf("child process with id %d has exited abnormally\n",retpid);
      }
  }

  

  return 0;
}
