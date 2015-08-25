#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>
#include<fcntl.h>

int main(int argc,char *argv[])
{
  
  if (argc != 2)
    {
      fprintf(stderr,"usage %s fifo_name\n",argv[0]);
      return -1;
    }

   int  fd = mknod(argv[1],S_IRUSR | S_IWUSR | S_IROTH | S_IWOTH,0);
  printf("fd = %d\n",fd);


  int bytes;
  char buf[100];
  memset(buf,0,sizeof buf);
  sprintf(buf,"hello world from writer process %d",(int)getpid());
 
  FILE *f = fopen(argv[1],"w");
  bytes  = fwrite(buf,1,strlen(buf),f);
  printf("wrote %d bytes of : %s into fifo %s\n",bytes,buf,argv[1]);
  fclose(f);


  return 0;
}
