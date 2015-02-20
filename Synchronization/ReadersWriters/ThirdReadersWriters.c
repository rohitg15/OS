#include<stdio.h>
#include<stdlib.h>
#include<pthread.h>
#include<semaphore.h>

#define MAX_THREADS 30
#define FOR(i,x,n) for(i=x;i<n;i++)


/*  This is a solution to the thrid readers writers problem. This solution prevents starvation of both readers and writers, thereby providing a fair opportunity for both depending on the exact order of arrival */
int shared_var = 0;
/* multiple readers can read at the same time. But only one writer can write at a time. So we use a mutex to restrict writing. Since readers are given 
   preference we must keep track of the number of readers
 */



int readcount = 0;


sem_t sem_readers_done;
sem_t sem_access;
pthread_mutex_t mutex_readcount;	//use this mutex to update the number of readers without race conditions
void initialize()
{
	sem_init(&sem_readers_done,0,1);
	sem_init(&sem_access,0,1);
	pthread_mutex_init(&mutex_readcount,0);
}

void* reader(void *data)
{

      /* obtain access for the current thread. This ensures that readers and writers are inter-leaved in the order in which they arrive
	 as opposed to the previous two versions of the problem, where the first reader/writer manages to let subsequent readers/writers
	 obtain access to the shared resource without respecting the opposite party
       */
	sem_wait(&sem_access);	
	pthread_mutex_lock(&mutex_readcount); // lock mutex for updating readcount
	
	readcount++;

	if (readcount == 1)
	{
		//if this is the first reader, then obtain exclusive access for readers. If there is a writer already writing, then wait
	  // This would block all readers until an existing writer gives up.
		sem_wait(&sem_readers_done);
	}

	pthread_mutex_unlock(&mutex_readcount);	//release the lock for updating readcount
	/*
	  allow other readers/writers to perform their job now by signalling the access semaphore
        */
	sem_post(&sem_access);

	//read the resource here . there may be multiple readers here
	printf("reading shared var : %d\n",shared_var);

	pthread_mutex_lock(&mutex_readcount);
	readcount--;
	if (readcount == 0)
	{
		//if there are no more readers, then it is safe to write now
		sem_post(&sem_readers_done);
	}
	pthread_mutex_unlock(&mutex_readcount);

}	


void* writer(void *data)
{
  
	sem_wait(&sem_access);	//obtain access for the current thread
	sem_wait(&sem_readers_done); // ensure that there are no readers reading the shared resource now
	

	/* perform writing here */

	shared_var = (int)data;
	printf("writing shared var : %d\n", shared_var);

	sem_post(&sem_access);	// signal the semaphore to allow other threads(readers/writers) to perform their job
	sem_post(&sem_readers_done); // signal this semaphore to allow blocked readers (of any) to continue their job

}
int  main(int argc,char *argv[])
{
	
	if (argc !=3)
	{
		fprintf(stderr, "Error : use the following format <executable> num_readers num_writers. eg : ./a.out 3 4\n");
		return 0;
	}

	initialize();
	int num_readers = atoi(argv[1]);
	int num_writers = atoi(argv[2]);

	num_readers = (num_readers < (MAX_THREADS>>1)) ? num_readers : MAX_THREADS>>1;
	num_writers = (num_writers < (MAX_THREADS>>1)) ? num_writers : MAX_THREADS>>1;

	pthread_t reader_threads[num_readers];
	pthread_t writer_threads[num_writers];
	int i=0;


	for(i=0;i<num_writers;i++)
	{
		pthread_create(&writer_threads[i],NULL,writer,(void*)i);
	}

	for(i=0;i<num_readers;i++)
	{
		pthread_create(&reader_threads[i],NULL,reader,NULL);
	}



	for(i=0;i<num_readers;i++)
	{
		pthread_join(reader_threads[i],NULL);
	}


	for(i=0;i<num_writers;i++)
	{
		pthread_join(writer_threads[i],NULL);
	}


	return 0;
}
