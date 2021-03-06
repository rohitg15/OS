#include<stdio.h>
#include<pthread.h>
#include<semaphore.h>
#include<stdlib.h>

#define MAX_THREADS 20
#define FOR(i,x,n) for(i=x;i<n;i++)

int shared_data = 1;
int read_count = 0, write_count = 0;

pthread_mutex_t mutex_write_count,mutex_read_count,mutex_write,mutex_read;

sem_t read_ready;

/*
  initialize all semaphores and mutexes
 */

void initialize()
{
  pthread_mutex_init(&mutex_write,0);
  //  pthread_mutex_init(&mutex_read,0);
  pthread_mutex_init(&mutex_write_count,0);
  pthread_mutex_init(&mutex_read_count,0);

  sem_init(&read_ready,0,1);
  
}


void* reader(void *data)
{
  //acquire a lock to update the read count
  pthread_mutex_lock(&mutex_read_count);
  read_count++;
  if (read_count == 1)
    {
      // the first reader has arrived, check if the reader is allowed to read
      sem_wait(&read_ready);
    }
  pthread_mutex_unlock(&mutex_read_count);
  
  //Read the value here.
  printf("reading  - shared data is : %d\n",shared_data);

  pthread_mutex_lock(&mutex_read_count);
  read_count--;
  if (read_count == 0)
    {
      // if all readers are done
      sem_post(&read_ready);
    }
  pthread_mutex_unlock(&mutex_read_count);
  
}


void* writer(void *data)
{

  //acquire mutex to update the number of writers
  pthread_mutex_lock(&mutex_write_count);
  write_count++;
  if (write_count == 1)
    {
      /*

	if this is the first writer, check if there are readers reading the data.
	If so writers must wait. If not, then writer can proceed writing data
      */
      sem_wait(&read_ready);
    }
  pthread_mutex_unlock(&mutex_write_count);
  //release mutex for modifying the number fo writers and acquire the mutex to perform writing
  pthread_mutex_lock(&mutex_write);

  //do actual write operations here

  shared_data = (int)data;

  printf("updated shared data : %d\n",shared_data);
  
  //release write mutex here
  pthread_mutex_unlock(&mutex_write);
  //update the number of writers here
  pthread_mutex_lock(&mutex_write_count);
  write_count--;
  if (write_count == 0)
    {
      //if all writers are done then signal the readers that they can read
      sem_post(&read_ready);
    }
  pthread_mutex_unlock(&mutex_write_count);

}



int main(int argc,char *argv[])
{
  if (argc != 3)
    {
      fprintf(stderr,"Error : incorrect format. use <executable> number_of_readers number_of_writers");
      return -1;
    }
  
  

  int reader_num = atoi(argv[1]);
  int writer_num = atoi(argv[2]);

  reader_num = (reader_num < (MAX_THREADS>>1)) ? reader_num : MAX_THREADS>>1 ;
  writer_num = (writer_num < (MAX_THREADS>>1)) ? writer_num : MAX_THREADS>> 1;

  int i = 0;
  /*
    create reader and writer threads
   */
  pthread_t readers[reader_num];
  pthread_t writers[writer_num];


    initialize();



  FOR(i,0,reader_num)
    {
      pthread_create(&readers[i],NULL,reader,NULL);
    }

  FOR(i,0,writer_num)
    {
      pthread_create(&writers[i],NULL,writer,(void*)i);
    }


  FOR(i,0,reader_num)
    {
      pthread_join(readers[i],NULL);
    }

  FOR(i,0,writer_num)
    {
      pthread_join(writers[i],NULL);
    }
   
  return 0;
}

