#include<stdio.h>
#include<stdlib.h>
#include<pthread.h>
#include<semaphore.h>
#include<queue>

using namespace std;


#define MAX_THREADS 20
#define QUEUE_SIZE 10
#define FOR(i,x,n) for(i=x;i<n;i++)

sem_t barber_ready,customer_ready;
pthread_mutex_t mutex_queue;

queue<int> customers;

void initialize()
{
  sem_init(&barber_ready,1,0);
  sem_init(&customer_ready,0,0);
  pthread_mutex_init(&mutex_queue,0);

}


void* barber(void *data)
{

  int customer = -1;
  
  while (1)
    {
  // wait for atleast one customer
  sem_wait(&customer_ready);

  // acquire mutex to remove first customer from customer queue
  pthread_mutex_lock(&mutex_queue);
  customer = customers.front();
  customers.pop();
  pthread_mutex_unlock(&mutex_queue);

  // server customer here

  printf("customer served by barber is : %d\n",customer);

  // signal that barber is ready
  sem_post(&barber_ready);
    }
}


void* customer(void *data)
{
  int customer_id = (int)data;
  
  // check if customers wait queue has enough space for one more customer
  if (customers.size() >= QUEUE_SIZE)
    {
      // exit this thread
      printf("reached wait queue's maximum size. cannot add a new customer with id %d\n",customer_id);
      pthread_exit(NULL);
    }

  // acquire mutex to add customer to the wait queue
  pthread_mutex_lock(&mutex_queue);
  customers.push(customer_id);
  pthread_mutex_unlock(&mutex_queue);
  
  printf("added customer %d to the wait queue\n",customer_id);
  // signal that customers are ready and waiting
  sem_post(&customer_ready);
  
  // wait for the barber to get free
  sem_post(&barber_ready);

}


int main(int argc,char *argv[])
{

  pthread_t barber_thread,customer_thread[MAX_THREADS];
  int i=0;
  
  FOR(i,0,MAX_THREADS)
    {
      pthread_create(&customer_thread[i],NULL,customer,(void*)i);
      if (i == 0)
	{
	  pthread_create(&barber_thread,NULL,barber,NULL);
	}
    }


  FOR(i,0,MAX_THREADS)
    {
      pthread_join(customer_thread[i],NULL);
    }

  pthread_join(barber_thread,NULL);
  

  return 0;
}
