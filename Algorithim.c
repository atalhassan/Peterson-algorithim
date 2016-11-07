/*
Description:
   using shared memory this program implements the Peterson's algorithms
   that prevents two processes entring a critical section at the same time.
*/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <ctype.h>
#include <sys/ipc.h>
#include <sys/shm.h>


void parent(int* turn,int* pr_0,int* pr_1,int time_crit_sect, int time_non_crit_sect, int shmid_t, int shmid_0, int shmid_1);
void child(int* turn,int* pr_0,int* pr_1,int time_crit_sect, int time_non_crit_sect);
void non_cs(int time_non_crit_sect);
void cs(char process, int time_crit_sect);


int main(int argc, char* argv[])
{
  //check for proper arguments
  if(argc != 1)
    {
      if(argc != 5)
	{
	  fprintf(stderr,"usage: a.out <time_parent> <time_child> <time_parent_non_cs> <time_child_non_cs>\n");
	  return -1;
	}
    }

  //default input values
  int time_parent = 3;
  int time_child = 3;
  int time_parent_non_cs = 3;
  int time_child_non_cs = 3;
  
  //check arguments types
  if(argc != 1)
    {
      int inputs[4];
      int i;
      for(i = 1; i < argc ; i++)
	{
	  if(!atoi(argv[i]))
	    {
	      fprintf(stderr,"%s is not a number \n",argv[i]);
	      return -1;
	    }
	  else
	    {
	      inputs[i-1] = atoi(argv[i]);
	    }
	}
      //move the values from the input array that was collected from argv
      time_parent = inputs[0];
      printf("time_parent = %d\n",inputs[0]);
      time_child = inputs[1];
      printf("time_child = %d\n",inputs[1]);
      time_parent_non_cs = inputs[2];
      printf("time_parent_non_cs = %d\n",inputs[2]);
      time_child_non_cs = inputs[3];
      printf("time_child_non_cs = %d\n",inputs[3]);
    }

  int shmid_t;
  int shmid_0;
  int shmid_1;
    
  int* turn;
  int* pr_0 = 0;
  int* pr_1 = 0;
  

  //create a shared memory segment
  shmid_t = shmget(0,1,0777 | IPC_CREAT);
  shmid_0 = shmget(0,1,0777 | IPC_CREAT);
  shmid_1 = shmget(0,1,0777 | IPC_CREAT);

  //attach it to the process, cast its address
  turn = shmat(shmid_t,0,0);
  pr_0 = shmat(shmid_0,0,0);
  pr_1 = shmat(shmid_1,0,0); 
  
  
  //fork here
  pid_t pid; // will carry the fork id
  pid = fork();
  
  if( pid < 0) // fork failed
    {
      fprintf(stderr,"Fork Error");
      return -1;
    }

  if(pid == 0) //child process
    {
      child(turn,pr_0,pr_1,time_child, time_child_non_cs);
    }
  else    //Parent process
    {
      //wait for child process
      parent(turn,pr_0,pr_1,time_parent, time_parent_non_cs, shmid_t, shmid_0,shmid_1);
      wait(NULL);
    }
  
  return 0;
}
  
void parent(int* turn,int* pr_0,int* pr_1,int time_crit_sect, int time_non_crit_sect, int shmid_t, int shmid_0, int shmid_1)
{
  for (int i = 0; i < 10; i++)
    {
      *pr_0 = 1;
      *turn = 1;
      while(*pr_1 && *turn);


      //protect this
      cs('p', time_crit_sect);
      *pr_0  = 0;
      non_cs(time_non_crit_sect);
      
    }
  
  //detach shared memory from the process
  shmdt(turn);
  shmdt(pr_0);
  shmdt(pr_1);

  //remove it
  shmctl(shmid_t,IPC_RMID,0);
  shmctl(shmid_0,IPC_RMID,0);
  shmctl(shmid_1,IPC_RMID,0);
}
  
void child(int* turn,int* pr_0,int* pr_1,int time_crit_sect, int time_non_crit_sect)
{
  for (int i = 0; i < 10; i++)
    {
      *pr_1 = 1;
      *turn = 0;
      while(*pr_0 && !*turn);
      //protect this
      cs('c', time_crit_sect);
      *pr_1  = 0;
      non_cs(time_non_crit_sect);
    }
  
  //detach shared memory from the process
  shmdt(turn);
  shmdt(pr_0);
  shmdt(pr_1);

}

void cs(char process, int time_crit_sect)
{
  if (process == 'p')
    {
      printf("parent in critical section\n");
      sleep(time_crit_sect);
      printf("parent leaving critical section\n");
    }

  else
    {
      printf("child in critical section\n");
      sleep(time_crit_sect);
      printf("child leaving critical section\n");
    }
}

void non_cs(int time_non_crit_sect)
{
  sleep(time_non_crit_sect);
}
