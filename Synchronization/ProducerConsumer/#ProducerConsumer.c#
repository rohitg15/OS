#include<stdio.h>
#include<pthread.h>
#include<stdlib.h>
#include<semaphore.h>
#include<string.h>


#define MAXSIZE 128

#define PRODUCER_NUMTHREADS 16

#define CONSUMER_NUMTHREADS 8

int shared_buffer[MAXSIZE];
int current_index = 0;


sem_t buffer_empty,buffer_full;
pthread_mutex_t buffer_mutex;

void initialize()
{

	pthread_mutex_init(&buffer_mutex,0);
	sem_init(&buffer_full,0,0);
	sem_init(&buffer_empty,0,MAXSIZE);
}

/* Generate a number into the next available slot of the array */
void* producer(void *data)
{
	
		/* wait on the semaphore describing the number of empty slots */
		sem_wait(&buffer_empty);

		/* we reach here if there is atleast one free slot in te shared_buffer
		   Now we try to acquire the mutex
		 */

		pthread_mutex_lock(&buffer_mutex);

		/* We reach here if this thread has acquired the mutex
		 */
		
		shared_buffer[current_index] = current_index;

		printf("producer produced (index,value) : %d , %d\n",current_index,shared_buffer[current_index]);

		current_index = (current_index + 1) % MAXSIZE;


		/* notify consumer threads blocked on buffer_full, that another slot has been filled */
		sem_post(&buffer_full);

	

		/* release the acquired lock */
		pthread_mutex_unlock(&buffer_mutex);

	//	pthread_exit(0);

}


void* consumer(void *data)
{

	/* block the current thread if the buffer is empty */
		sem_wait(&buffer_full);

		/* try to acquire the mutex */
		pthread_mutex_lock(&buffer_mutex);

		/* if we reach this point in code then we have acquired the mutex. 
	       perform the work here
		 */
	     
	       printf("consumer consumed (index,value) : %d %d\n",current_index,shared_buffer[current_index]);
	       shared_buffer[current_index] = -1;
	       current_index = (current_index - 1) % MAXSIZE;

	    pthread_mutex_unlock(&buffer_mutex);


	     /* signal producer threads that are blocked on buffer_empty */

	    sem_post(&buffer_empty);

	//    pthread_exit(0);

	
}



int main(int argc,char* argv[])
{



	pthread_t producer_thread_array[PRODUCER_NUMTHREADS],consumer_thread_array[CONSUMER_NUMTHREADS];
	

	initialize();

	int i=0;

	memset(shared_buffer,0,MAXSIZE*sizeof(int));
	
	/* create producer threads */
	for(i=0;i<PRODUCER_NUMTHREADS;i++)
	{
		pthread_create(&producer_thread_array[i],NULL,producer,NULL);	
	}

	/* create consumer threads */

	for(i=0;i<CONSUMER_NUMTHREADS;i++)
	{
		pthread_create(&consumer_thread_array[i],NULL,consumer,NULL);
	}

	for(i=0;i<PRODUCER_NUMTHREADS;i++)
	{
		pthread_join(producer_thread_array[i],NULL);
	}

	for(i=0;i<CONSUMER_NUMTHREADS;i++)
	{
		pthread_join(consumer_thread_array[i],NULL);
	}
	
	return 0;
}