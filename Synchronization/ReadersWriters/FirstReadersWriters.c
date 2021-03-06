#include<stdio.h>
#include<stdlib.h>
#include<pthread.h>
#include<semaphore.h>


/*
  This program solves the Readers-Writers problem using Semaphores and Mutexes
  This is the first version where readers are given higher priority
  This might lead to starvation of writers
 */

int read_count = 1;

#define MAX_THREADS 20
#define FOR(i,x,n) for (i=x;i<=n;i++)

/*
  Declare Mutexes, Semaphores
 */

pthread_mutex_t read_count_mutex;
sem_t sem_write_access;


/*
  initialixe mutex for protecting read_count
  initialize the semaphore for coordinating access
 */
void initialize()
{
  sem_init(&sem_write_access,0,1);
  pthread_mutex_init(&read_count_mutex,0);
  
}

int shared_data = 0;

/*
  thread function for readers
 */
void* reader(void *data)
{

  pthread_mutex_lock(&read_count_mutex);
  read_count++;
  if (read_count == 1)
    {
      //wait for all readers to finish even if 
      sem_wait(&sem_write_access);
    }
  pthread_mutex_unlock(&read_count_mutex);


  //read the data here

  printf("reading shared data : %d\n",shared_data);


  pthread_mutex_lock(&read_count_mutex);
  read_count--;
  if (read_count == 0)
    {
      sem_post(&sem_write_access);
    }
  pthread_mutex_unlock(&read_count_mutex);
  

}



void* writer(void *data)
{
  sem_wait(&sem_write_access);


  //write to the shared the variable

  shared_data = (int)data;
  

  sem_post(&sem_write_access);
  printf("writing to data : %d\n",shared_data);

}



int main(int argc,char *argv[])
{

  
  

  if (argc != 3)
    {
      fprintf(stderr,"Error: correct format is <executable> number_of_readers number_of_writers\n");
      return -1;
    }

  int i=0,num_readers = 0, num_writers = 0;
  num_readers = atoi(argv[1]);
  num_writers = atoi(argv[2]);

  num_readers = (num_readers > (MAX_THREADS >> 1)) ? MAX_THREADS >> 1 : num_readers;

  num_writers = (num_writers > (MAX_THREADS >> 1)) ? MAX_THREADS >> 1 : num_writers;

  /*
    initialize values for the semaphores and mutexes
   */
  initialize();
  pthread_t reader_threads[num_readers];
  pthread_t writer_threads[num_writers];

  // create reader threads
  FOR(i,0,num_readers)
    {
      pthread_create(&reader_threads[i],NULL,reader,NULL);
    }


  // create writer threads
  FOR(i,0,num_writers)
    {
      pthread_create(&writer_threads[i],NULL,writer,(void*)(i+1));
    }

  pthread_t more_readers[num_readers];

  FOR(i,0,num_readers)
    {
      pthread_create(&more_readers[i],NULL,reader,NULL);
    }

  //wait for all threads to complete
  FOR(i,0,num_readers)
    {
      //passing NULL as we dont care about the returned value
      pthread_join(reader_threads[i],NULL);
      pthread_join(more_readers[i],NULL);
    }

  FOR(i,0,num_writers)
    {
      // passing NULL as we dont care about the returned value
      pthread_join(writer_threads[i],NULL);
    }
    


  return 0;
}
