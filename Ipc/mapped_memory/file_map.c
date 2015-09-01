#include<stdio.h>
#include<string.h>
#include<sys/mman.h>
#include<fcntl.h>
#include<unistd.h>
#include<errno.h>

#define FOR(i,x,n) for(i=x;i<n;i++)
#define MEMORY_SIZE 0x100


int main(int argc,char *argv[])
{

  if (argc != 2)
    {
      fprintf(stderr,"usage : %s file_name\n",argv[0]);
      return -1;
    }
  int ret;
  int fd = open(argv[1],O_CREAT | O_RDWR , S_IRUSR | S_IWUSR);
  if (fd < 0)
    {
      fprintf(stderr,"could not open the file %s. exit with error %s\n",argv[1],strerror(errno));
      return -1;
    }
  // write to the file to commit memory on the fs
  if (lseek(fd,MEMORY_SIZE,SEEK_SET) < 0)
    {
      fprintf(stderr,"lseek() failed with error %s for file %s\n",strerror(errno),argv[1]);
      close(fd);
      return -1;
    }
  ret = write(fd,"",1);
  if (ret < 0)
    {
      fprintf(stderr,"write() with error %s for file %s\n",strerror(errno),argv[1]);
      close(fd);
      return -1;
    }

  char *mbuf = mmap(NULL,MEMORY_SIZE,PROT_READ | PROT_WRITE ,MAP_SHARED,fd,0);
  if (mbuf == MAP_FAILED)
    {
      fprintf(stderr,"mmap() failed with error %s for file %s\n",strerror(errno),argv[1]);
      close(fd);
      return -1;
    }

  sprintf(mbuf,"hello world from %d",(int)getpid());

  munmap(mbuf,MEMORY_SIZE);
  close(fd);
}
