#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<fcntl.h>

int main(int argc,char *argv[])
{
  
  if (argc != 2)
    {
      fprintf(stderr,"usage %s fifo_name\n",argv[0]);
      return -1;
    }

  int fd,bytes;
  char buf[100];
  memset(buf,0,sizeof buf);

  FILE *f = fopen(argv[1],"r");
  bytes  = fread(buf,1,sizeof buf,f);
  printf("read %d bytes of : %s from fifo %s\n",bytes,buf,argv[1]);
  fclose(f);


  return 0;
}
