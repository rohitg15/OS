#include<stdio.h>
#include<stdlib.h>
#include<pthread.h>
#include<semaphore.h>


#define MAXSIZE 32


sem_t sem_odd,sem_even;

int current_odd = 1,current_even = 0,current_index = 0;

void initialize()
{
	sem_init(&sem_odd,0,0);
	sem_init(&sem_even,0,1);
}



void* print_odd(void *data)
{

	while(current_index < MAXSIZE)
	{
		sem_wait(&sem_odd);

		/* we reach this point if an odd number is supposed to be written */

		printf("current odd number is : %d\n",current_odd);
		current_odd+=2;
		current_index++;

		/* signal even function's turn */
		sem_post(&sem_even);
	}

	
}


void* print_even(void *data)
{
	while(current_index < MAXSIZE)
	{
		sem_wait(&sem_even);

		/* we reach this point if an even number is supposed to be written */

		printf("current even number is : %d\n",current_even);
		current_even+=2;

		current_index++;

		/* signal odd function's turn */
		sem_post(&sem_odd);
	}
}



int main(int argc,char* argv[])
{
	initialize();

	pthread_t print_odd_thread,print_even_thread;

	/* create the two threads */

	pthread_create(&print_odd_thread,NULL,print_odd,NULL);

	pthread_create(&print_even_thread,NULL,print_even,NULL);

	/* wait for the two threads to complete */

	pthread_join(print_odd_thread,NULL);
	pthread_join(print_even_thread,NULL);


	return 0;
}