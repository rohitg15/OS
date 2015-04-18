#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<fcntl.h>
#include<unistd.h>

int main(int argc,char *argv[])
{
  int fd;
  if (argc != 4)
    {
      fprintf(stderr,"usage : %s device_name buffer_size 0/1(r/w)\n",argv[0]);
      exit(-1);
    }
  const char *device = argv[1];
  int size = atoi(argv[2]);
  int ch = atoi(argv[3]);
  int ret = 0;

  char buf[size];
  memset(buf,0,sizeof(buf));

  fd = open(device,O_RDWR);
  if (ch == 0)
    {
      ret = read(fd,buf,size);
      printf("read : %s\n",buf);
    }
  else
    {
      printf("enter data:");
      fgets(buf,size,stdin);
      write(fd,buf,size);
    }
  close(fd);

}
