#include<stdio.h>
#include<stdlib.h>
#include<pthread.h>
#include<semaphore.h>

#define FOR(i,x,n) for(i=x;i<n;i++)

#define MAX_THREADS 5

/*
  we are attempting to solve the dining philosophers problem:
       
     There are N philosophers seated in a circular table in such a way that each philosopher has one fork to his/her left and one to their right side.
     If both the forks are free, then the philosopher sitting in between them can use both and start eating food. otherwise they must wait for their turn.
  Solution:
     
     clearly there are N philosophers and N forks
     p - philosopher
     f - fork
     p0 f0 p1 f1 p2 f2 p3 f3 p4 f4

     the i'th philosopher can pickup the forks at positions i and ( i + 1 ) % N if they are free
     we use an array of N semaphores, that represent the state of the respective forks and signal the threads(philosophers) waiting on them whenever they are
     free.

     this solution might lead to a deadlock if each philosopher picks the i'th fork first . Then each philosopher will start waiting for the ( i + 1 )'st 
     fork to be released by their predecessor. This would lead to a circular dependency and consequently a deadlock

     To work around this, we employ asymmetry while picking up the forks. odd numbered philosophers pick the left fork first while even numbered philosophers choose the right fork first. This prevents the deadlock

 */



sem_t forks[MAX_THREADS];
/*
  we use this function to initialize all semaphores and mutexes used in this solution
 */
void initialize()
{
  /*
    initially all forks are free
   */
  int i=0;
  FOR(i,0,MAX_THREADS)
    {
      sem_init(&forks[i],0,1);
    }
}


/*
  this is the thread function
 */

void* dine(void *data)
{
  int idx, fork1_idx,fork2_idx;
  idx = (int)data;
  fork1_idx = idx;
  fork2_idx = (idx + 1) % MAX_THREADS;

  if (idx & 1)
    {
      /*
	odd numbered thread (philosopher)
	pick fork1 first, then fork2
      */
      sem_wait(&forks[fork1_idx]);
      sem_wait(&forks[fork2_idx]);
      
    }
  else
    {
      /*
	even numbered thread (philosopher)
	pick fork2 first, then fork1
       */
      sem_wait(&forks[fork2_idx]);
      sem_wait(&forks[fork1_idx]);
    }
  
  printf(" Thread group id : ( %d ) Philosopher %d is eating with forks %d and %d\n",(int)getpid(),idx,fork1_idx,fork2_idx);

      /*
	release the forks as this philosopher has finished eating
       */
      sem_post(&forks[fork2_idx]);
      sem_post(&forks[fork1_idx]);
  
      pthread_exit(NULL);
}


int main(int argc,char *argv[])
{

  pthread_t philosophers[MAX_THREADS];
  int i=0;

  initialize();

  FOR(i,0,MAX_THREADS)
    {
      pthread_create(&philosophers[i],NULL,dine,(void*)i);
    } 

  /*
    wait for all the threads to return
    (we hate zombies)
   */

  FOR(i,0,MAX_THREADS)
    {
      pthread_join(philosophers[i],NULL);
    }

  return 0;
}
