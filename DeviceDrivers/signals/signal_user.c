#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<errno.h>
#include<signal.h>
#include<fcntl.h>


#define SIZE 6
#define SIG_TEST 44


/*
  user mode program to catch signals sent from the kernel
  invoke as follows : ./signal_user /dev/kernel_signal
  where kernel_signal is the name of the device file as registered with the kernel under /dev
  
 */

static char buf[SIZE];


void signal_handler(int num,siginfo_t *info,void *ctx)
{
  printf("signal number %d received from the kernel\n",num);
  printf("goodbye\n");
  exit(0);
}


int main(int argc,char *argv[])
{
  
  struct sigaction act;
 
  // get the process id
  int pid = (int)getpid();
  
  // register the signal handler
  memset(&act,0,sizeof act);
  
  act.sa_flags = SA_SIGINFO;
  act.sa_sigaction = signal_handler;
  sigaction(SIG_TEST,&act,NULL);


  int fd = open(argv[1],O_WRONLY);
  if (fd <= 0)
    {
      fprintf(stderr,"could not open %s for writing, error: %s\n",argv[1],strerror(errno));
      return -1;
    }

  sprintf(buf,"%d",pid);
  int ret = write(fd,buf,strlen(buf));
  if (ret < 0)
    {
      fprintf(stderr,"could not write to %s, error : %s\n",argv[1],strerror(errno));
      close(fd);
      return -1;
    }

  write(1,buf,strlen(buf));
  
  printf("\n");

  close(fd);

  printf("going into sleep! waiting for a possible signal from the kernel!\n");
  sleep(10000);
  
  return 0;
}
