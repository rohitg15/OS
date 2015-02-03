#include<stdio.h>
#include<stdlib.h>
#include<pthread.h>
#include<semaphore.h>

/*  This is a solution to the first readers writers problem. Here Readers are given higher priority, resulting in writers starving */
int shared_var = 0;
/* multiple readers can read at the same time. But only one writer can write at a time. So we use a mutex to restrict writing. Since readers are given 
   preference we must keep track of the number of readers
 */



int readcount = 0;


sem_t sem_order,sem_access;
pthread_mutex_t mutex_readcount;	//use this mutex to update the number of readers without race conditions

void initialize()
{
	sem_init(&sem_order,0,1);
	sem_init(&sem_access,0,1);
	pthread_mutex_init(&mutex_readcount,0);
}

void* reader(void *data)
{
	sem_wait(&sem_order);	//obtain access for the current thread

	pthread_mutex_lock(&mutex_readcount); // lock mutex for updating readcount
	
	if (readcount == 0)
	{
		//if this is the first reader, then obtain exclusive access for readers
		sem_wait(&sem_access);
	}
	readcount++;

	pthread_mutex_unlock(&mutex_readcount);	//release the lock for updating readcount

	//read the resource here . there may be multiple readers here
	printf("reading shared var : %d\n",shared_var);

	pthread_mutex_lock(&mutex_readcount);
	readcount--;
	if (readcount == 0)
	{
		//if there are no more readers, then it is safe to write now
		sem_post(&sem_access);
	}
	pthread_mutex_unlock(&mutex_readcount);

}	


void* writer(void *data)
{
	int val = (int)data;

	sem_wait(&sem_order);	//obtain access for the current thread
	sem_wait(&sem_access);	//obtain exclusive access from readers and other writers
	sem_post(&sem_order);	// allow other threads to compete for access 

	/* perform writing here */

	shared_var = val;
	printf("shared var : %d\n", shared_var);

	sem_post(&sem_access);	//release the write lock

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