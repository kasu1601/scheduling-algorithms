/* 	FILE NAME: simulator.c
	AUTHOR : Harshi Kasundi Bandaranayake
	DATE : 07/05/2022
	INCLUDES : fcfs.h, scan.h, cscan.h, look.h, clook.h, sstf.h	*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>  

/*DEFINING CONSTANTS*/
#define BUFFERSIZE 1
#define THREAD_COUNT 6
#define TRUE 1
#define FALSE !TRUE
#define VISITED -1
#define WAITING !VISITED

/*DEFINING DATA SHARED BY PARENT AND CHILD THREADS*/
int count = 0; int check = -1; int emptyspaces = BUFFERSIZE; int hasFile = FALSE; int isEnd = FALSE; int threadcount = 0; int canRead = FALSE; int writercount = FALSE;
int fcfs_write = FALSE; int sstf_write = FALSE; int scan_write = FALSE; int cscan_write = FALSE; int look_write = FALSE; int clook_write = FALSE; int bufferReady = FALSE;
int* buffer1; int buffer2[BUFFERSIZE]; 

/*INITIALIZING MUTEXES*/
pthread_mutex_t fcfs_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t sstf_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t scan_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t cscan_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t look_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t clook_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t writer_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t start_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t end_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t input_mutex = PTHREAD_MUTEX_INITIALIZER;

/*INITIALIZING CONDITIONAL VARIABLES*/
pthread_cond_t fcfs_cond = PTHREAD_COND_INITIALIZER;
pthread_cond_t sstf_cond = PTHREAD_COND_INITIALIZER;
pthread_cond_t scan_cond = PTHREAD_COND_INITIALIZER;
pthread_cond_t cscan_cond = PTHREAD_COND_INITIALIZER;
pthread_cond_t look_cond = PTHREAD_COND_INITIALIZER;
pthread_cond_t clook_cond = PTHREAD_COND_INITIALIZER;
pthread_cond_t readbuffer1 = PTHREAD_COND_INITIALIZER;
pthread_cond_t readbuffer2 = PTHREAD_COND_INITIALIZER;
pthread_cond_t terminate = PTHREAD_COND_INITIALIZER;
pthread_cond_t empty = PTHREAD_COND_INITIALIZER;
pthread_cond_t got_filename = PTHREAD_COND_INITIALIZER;

/*FUNCTION DECLARATION*/
void countData(char* filename);
void *fcfs(void *arg);
void *sstf(void *arg);
void *scan(void *arg);
void *cscan(void *arg);
void *look(void *arg);
void *clook(void *arg);

/*MAIN WILL BE THE PARENT THREAD THAT CREATES THREADS A, B, C, D, E & F*/
int main(int argc, char* argv[])
{
	char filename[10]; char str[4096]; int seektime[THREAD_COUNT]; char* token; FILE *fptr = NULL; int i = 0;
	
	/*SETTING UP THE THREAD IDs*/
	pthread_t thread_A; pthread_t thread_B; pthread_t thread_C; 
	pthread_t thread_D; pthread_t thread_E; pthread_t thread_F;
	
	/*CREATES THE CHILD THREADS*/
	pthread_create(&thread_A, NULL, fcfs, NULL); 
	pthread_create(&thread_B, NULL, sstf, NULL);
	pthread_create(&thread_C, NULL, scan, NULL);
	pthread_create(&thread_D, NULL, cscan, NULL);
	pthread_create(&thread_E, NULL, look, NULL);
	pthread_create(&thread_F, NULL, clook, NULL);
	
	do
	{
		/*GETS THE FILE DETAILS AS USER INPUT*/
		printf( "\n\nEnter the input file name : ");
		scanf("%s", filename);	
		system("clear");
		
		/*WAKES UP ALL CHILD THREADS WAITING FOR AN INPUT FROM THE PARENT THREAD*/
		pthread_mutex_lock(&input_mutex);
		/*CHECK IF USER HAS ENTERED QUIT*/
		check = strcmp(filename,"QUIT"); 
		hasFile = TRUE;
		pthread_cond_broadcast(&got_filename);
		hasFile = FALSE;
		pthread_mutex_unlock(&input_mutex);
		
		if(check != 0)
		{
			fptr = fopen(filename, "r");
			if(fptr == NULL) 
			{
				perror("Error opening the text file");	
			}else{
				/*FINDS THE NUMBER OF TOTAL DATA PROVIDED IN THE .TXT FILE*/
				countData(filename);
															
		   		/*THERE SHOULD BE MORE AT LEAST ONE DISK REQUEST IN THE .TXT FILE FOR THE ALGORITHMS TO WORK WITH*/
		   		if(count > 3)
		   		{
		   			buffer1 = malloc(count * sizeof(int));
					i  = 0;
					
			   		/*TOKENIZE THE STRING AND MAKE AN ARRAY OF INTEGERS*/
			   		if(fgets(str, 4096, fptr) != NULL) 
			   		{
			   			token = strtok(str, " ");		
						while(token != NULL) 
						{
							buffer1[i] = atoi(token);
							token = strtok(NULL, " ");
							i++;
				    		}
				    	} 
				    	
				    	/*WAKES UP ALL THREADS WAITING ON THIS CONDITION TO PROCEED*/
					pthread_mutex_lock(&start_mutex);
					bufferReady = TRUE;
					pthread_cond_broadcast(&readbuffer1);
					pthread_mutex_unlock(&start_mutex);
										
					/*WAIT TILL BUFFER OF SIZE 1 IS FILLED WITH A SEEK TIME FROM ONE OF THE THREADS*/
					pthread_mutex_lock(&writer_mutex);
					
					/*WAKES UP A BLOCKED FCFS(THREAD A) TO WRITE TO THE BUFFER FIRST*/
					pthread_mutex_lock(&fcfs_mutex);
					fcfs_write = TRUE;
					pthread_cond_signal(&fcfs_cond);
					pthread_mutex_unlock(&fcfs_mutex);
										
					for(i = 0; i < THREAD_COUNT; i++)
					{
						/*IF THERE IS AN UNREAD SEEK TIME INSIDE THE BUFFER THREADS ARE BLOCKED FROM WRITING TO THE BUFFER*/
						if(emptyspaces == BUFFERSIZE)
					    	{
							pthread_cond_wait(&readbuffer2, &writer_mutex);
						}
						
						/*STORES EACH SEEK TIME INTO AN ARRAY OF INTEGERS*/
						seektime[i] = buffer2[0];
						emptyspaces = BUFFERSIZE;
						
						/*WAKES UP ONE OF THE THREADS WAITING ON THE CONDITION*/
						pthread_cond_signal(&empty);
					}
					pthread_mutex_unlock(&writer_mutex);
					
					/*PRINTS OUT THE SEEKTIMES FROM THREAD A TO F*/
					printf("\nFor %s :\n", filename);
					printf("FCFS : %d\n", seektime[0]);
					printf("SSTF : %d\n", seektime[1]);
					printf("SCAN : %d\n", seektime[2]);
					printf("CSCAN : %d\n", seektime[3]);
					printf("LOOK : %d\n", seektime[4]);
					printf("CLOOK : %d\n", seektime[5]);
					
					/*THREADS ARE BLOCKED FROM READING BUFFER 1*/
					bufferReady = FALSE; free(buffer1);
					
				}else{
			    		printf("Insufficient data provided. There should be more than one disk request in the .txt file");
			    	}
			    	
				/*CLOSE THE FILE AFTER READING DATA*/
				fclose(fptr);    
			}
		}
		
	}while(check != 0);

	/*ASKS ALL CHILD THREADS TO TERMINATE*/
	pthread_mutex_lock(&end_mutex);
	isEnd = TRUE;
	pthread_cond_broadcast(&terminate);
	pthread_mutex_unlock(&end_mutex);
	
	/*PARENT THREAD WAITS FOR CHILD THREADS TO TERMINATE*/
	pthread_join(thread_A,NULL);
	pthread_join(thread_B,NULL);
	pthread_join(thread_C,NULL);
	pthread_join(thread_D,NULL);
	pthread_join(thread_E,NULL);
	pthread_join(thread_F,NULL);
	
	/*DESTROYS MUTEXES AND CONDITIONAL VARIABLES*/
	/*pthread_mutex_destroy();*/
		
	/*PARENT THREAD TERMINATE*/
	pthread_exit(NULL);	
}
	
/*METHOD TO COUNT THE NO.OF DATA FIELDS IN THE .TXT FILE PROVIDED BY THE USER*/
void countData(char* filename)
{
	FILE *fptr = NULL; char str[4096]; char* token;
	
	/*OPEN FILE TO READ IN DATA*/
	fptr = fopen(filename, "r");
	
	if(fptr != NULL) 
	{
   		 if(fgets(str, 4096, fptr) != NULL) 
   		 {
        		count = 0;
        		token = strtok(str," ");
			while(token != NULL) 
			{
				token = strtok(NULL," ");
				count++;
	    		}
    		 }
	}
	
	/*CLOSE THE FILE AFTER READING DATA*/
	fclose(fptr);
}

void *fcfs(void *arg)
{ 
	int movements = 0; int difference = 0; int i = 0; int currPos = 0; int length = 0; int* array;
	
	do
	{	
		/*CHILD THREAD IS BLOCKED UNTIL CHILD IS ALLOWED TO READ INPUT FROM THE BUFFER*/
		pthread_mutex_lock(&start_mutex);
		if(check != 0)
		{
			/*IF BUFFER 1 IS NOT READY FOR READING, CHILD THREAD IS BLOCKED*/
			if(bufferReady == FALSE)
			{
				pthread_cond_wait(&readbuffer1, &start_mutex);
			}
			
			/*COUNT - 3 SINCE THE FIRST THREE DATA STORED IN THE BUFFER ARE NOT DISK REQUESTS*/
			length  = count - 3;
			
			/*CREATING A MALLOC SIZED ARRAY*/
			array = malloc((length * sizeof(int)));
			
			currPos = buffer1[1];
			
			/*READING DISK REQUESTS FROM BUFFER INTO AN ARRAY*/
			for(i = 0;  i < length; i++)
			{
				array[i] = buffer1[i+3];
			}
			
			/*CALCULATE FCFS SEEK TIME*/
			for(i = 0;  i < length; i++)
			{
				difference = abs(array[i] - currPos);
				movements = movements + difference;
				currPos = array[i];
			}
		}
		pthread_mutex_unlock(&start_mutex);

		if(check != 0)
		{
			/*CHILD THREAD IS BLOCKED UNTIL PARENT THREAD SIGNALS ITS READY TO READ SEEK TIMES*/
			pthread_mutex_lock(&fcfs_mutex);
			if(fcfs_write == FALSE)
			{
				pthread_cond_wait(&fcfs_cond,&fcfs_mutex);
			}
			fcfs_write = FALSE;
			pthread_mutex_unlock(&fcfs_mutex);
			
			pthread_mutex_lock(&writer_mutex);
			if(emptyspaces == 0)
			{
				/*CONDITION WILL NEVER BE REACHED BY THE FCFS THREAD AS IT WILL ALWAYS BE THE FIRST THREAD TO FILL OUT BUFFER 2 IN AN ITERATION*/
				pthread_cond_wait(&empty,&writer_mutex);	
			}
			
			/*WRITE SEEK TIME TO BUFFER 2*/
			buffer2[0] = movements;
			emptyspaces = 0;
			
			/*WAKES UP PARENT THREAD TO READ THE WRITTEN SEEK TIME*/
			pthread_cond_signal(&readbuffer2);
			sstf_write = TRUE;
			
			/*AFTER FCFS WRITES TO BUFFER 2, THEN SSTF (THREAD B) IS SIGNALED TO WRITE TO BUFFER 2*/
			pthread_cond_signal(&sstf_cond);
			pthread_mutex_unlock(&writer_mutex);
		}
		
		free(array); movements = 0; difference = 0;
		
		/*THREAD IS BLOCKED UNTIL CHILD THREAD GETS A NEW FILENAME FROM THE PARENT THREAD*/
		pthread_mutex_lock(&input_mutex);
		if(hasFile == FALSE)
		{
			pthread_cond_wait(&got_filename, &input_mutex);
		}
		pthread_mutex_unlock(&input_mutex);
		
	}while(check != 0);
	
	/*CHILD THREAD WAITS FOR A SIGNAL FROM PARENT THREAD TO TERMINATE*/
	pthread_mutex_lock(&end_mutex);
	if(isEnd == FALSE)
	{
		pthread_cond_wait(&terminate, &end_mutex);
	}
	printf("%ld has terminated\n",pthread_self());
	pthread_mutex_unlock(&end_mutex);
	
	/*CHILD THREAD TERMINATES*/
	pthread_exit(NULL);
}	

void *sstf(void *arg)
{ 
	int movements = 0; int difference = 0; int stepNum = 0; int cylinderNum = 0;
	int index = 0; int i = 0; int j = 0; int currPos = 0; int length = 0; int* array;
		
	do
	{	
		pthread_mutex_lock(&start_mutex);
		if(check != 0)
		{
			if(bufferReady == FALSE)
			{
				pthread_cond_wait(&readbuffer1, &start_mutex);
			}
			
			length  = count - 3;
			array = malloc((length * sizeof(int)));
			
			cylinderNum = buffer1[0];
			currPos = buffer1[1];
			
			for(i = 0;  i < length; i++)
			{
				array[i] = buffer1[i+3];
			}
			
			for(i = 0; i < length; i++)
			{
				/*DIFFERENCE BETWEEN TWO CYLINDER DISK INDICES CAN NEVER BE GREATER THAN THE NO.OF CYLINDER DISKS*/
				difference = cylinderNum;
				stepNum = 0;
				
				for(j = 0;  j < length; j++)
				{
					if(array[j] != VISITED)
					{
						stepNum = abs(array[j] - currPos);				
						if(stepNum < difference)
						{
							difference =  stepNum;
							index = j;
						}
					}
				}
				
				currPos = array[index];
				array[index] = VISITED;
				movements = movements + difference;
			}
		}
		pthread_mutex_unlock(&start_mutex);
		
		if(check !=0)
		{
			/*SSTF WAITS UNTIL FCFS FINISHED WRITING SEEKTIME INTO BUFFER 2*/
			pthread_mutex_lock(&sstf_mutex);
			if(sstf_write == FALSE)
			{
				pthread_cond_wait(&sstf_cond,&sstf_mutex);
			}
			sstf_write = FALSE;
			pthread_mutex_unlock(&sstf_mutex);
			
			pthread_mutex_lock(&writer_mutex);
			if(emptyspaces == 0)
			{
				/*IF PARENT THREAD IS STILL READING FCFS SEEK TIME, BLOCK*/
				pthread_cond_wait(&empty,&writer_mutex);
			}
			buffer2[0] = movements;
			emptyspaces = 0;
			pthread_cond_signal(&readbuffer2);
			scan_write = TRUE;
			
			/*WAKES UP SCAN (THREAD C) TO WRITE DATA INTO BUFFER 2*/
			pthread_cond_signal(&scan_cond);
			pthread_mutex_unlock(&writer_mutex);
		}
				
		free(array); movements = 0; difference = 0; stepNum = 0; index = 0;
	
		pthread_mutex_lock(&input_mutex);
		if(hasFile == FALSE)
		{
			pthread_cond_wait(&got_filename, &input_mutex);
		}
		pthread_mutex_unlock(&input_mutex);
		
	}while(check != 0);
	
	pthread_mutex_lock(&end_mutex);
	if(isEnd == FALSE)
	{
		pthread_cond_wait(&terminate, &end_mutex);
	}
	printf("%ld has terminated\n",pthread_self());
	pthread_mutex_unlock(&end_mutex);
	pthread_exit(NULL);
}

void *scan(void *arg)
{
	int movements = 0; int difference = 0; int temp = 0;  int cylinderNum = 0; int currPos = 0;
	int prevPos = 0; int index = 0; int i = 0; int j = 0; int length = 0; int* array;
		
	do
	{
		pthread_mutex_lock(&start_mutex);
		if(check != 0)
		{
			if(bufferReady == FALSE)
			{
				pthread_cond_wait(&readbuffer1, &start_mutex);
			}
			
			length  = count - 3;
			array = malloc((length * sizeof(int)));
			
			cylinderNum = buffer1[0];
			currPos = buffer1[1];
			prevPos = buffer1[2];
			
			for(i = 0;  i < length; i++)
			{
				array[i] = buffer1[i+3];
			}
			
			for(i = 0; i < length; i++)
			{
				for(j = 0; j < length-i-1; j++)
				{
					if(array[j] > array[j+1])
					{
						temp = array[j];
						array[j] = array[j+1];
						array[j+1] = temp;
					}
				}
			}
			
			for(i = 0; i < length; i++)
			{
				if(currPos < array[i])
				{
					index = i;
					break;
				}
			}
			
			/*HEAD MOVES TOWARDS SMALLER DISK NUMBERS*/
			if(prevPos - currPos > 0) 
			{
				for(i = index-1 ; i >= 0; i--)
				{
					difference = abs(array[i] - currPos);
					movements = movements + difference;
					currPos = array[i];
				}
				
				difference = array[i+1]; 
				movements =  movements + difference;
				
				currPos = 0; 
				
				for(i = index; i < length; i++)
				{
					difference = abs(array[i] - currPos);
					movements = movements + difference;
					currPos = array[i];
				}
			
			/*HEAD MOVES TOWARDS LARGER DISK NUMBERS*/
			}else{
			
				for(i = index; i < length; i++)
				{
					difference = abs(array[i] - currPos);
					movements = movements + difference;
					currPos = array[i];
				}
				
				movements = movements + abs(cylinderNum - array[i-1]-1);
				currPos = cylinderNum - 1;
				
				for(i = index - 1; i >= 0; i--)
				{
					difference = abs(array[i] - currPos);
					movements = movements + difference;
					currPos = array[i];
				}
			}
			
		}
		pthread_mutex_unlock(&start_mutex);
		
		if(check != 0)
		{
			/*SCAN WAITS UNTIL SSTF(THREAD B) FINISHES WRITING TO BUFFER 2*/
			pthread_mutex_lock(&scan_mutex);
			if(scan_write == FALSE)
			{
				pthread_cond_wait(&scan_cond,&scan_mutex);
			}
			scan_write = FALSE;
			pthread_mutex_unlock(&scan_mutex);
			
			pthread_mutex_lock(&writer_mutex);
			if(emptyspaces == 0)
			{
				pthread_cond_wait(&empty,&writer_mutex);
			}
			buffer2[0] = movements;
			emptyspaces = 0;
			pthread_cond_signal(&readbuffer2);
			cscan_write = TRUE;
			
			/*WAKES UP CSCAN (THREAD D) TO WRITE TO BUFFER*/
			pthread_cond_signal(&cscan_cond);
			pthread_mutex_unlock(&writer_mutex);
		}
		
		free(array); movements = 0; difference = 0; temp = 0; index = 0;
		
		pthread_mutex_lock(&input_mutex);
		if(hasFile == FALSE)
		{
			pthread_cond_wait(&got_filename, &input_mutex);
		}
		pthread_mutex_unlock(&input_mutex);
		
	}while(check != 0);
	
	pthread_mutex_lock(&end_mutex);
	if(isEnd == FALSE)
	{
		pthread_cond_wait(&terminate, &end_mutex);
	}
	printf("%ld has terminated\n",pthread_self());
	pthread_mutex_unlock(&end_mutex);
	pthread_exit(NULL);
}
	
void *cscan(void *arg)
{
	int movements = 0; int difference = 0; int temp = 0;  int cylinderNum = 0; int currPos = 0;
	int prevPos = 0; int index = 0; int i = 0; int j = 0; int length = 0; int* array;
		
	do
	{	
		pthread_mutex_lock(&start_mutex);
		if(check != 0)
		{
			if(bufferReady == FALSE)
			{
				pthread_cond_wait(&readbuffer1, &start_mutex);
			}
						
			length  = count - 3;
			array = malloc((length * sizeof(int)));
			
			cylinderNum = buffer1[0];
			currPos = buffer1[1];
			prevPos = buffer1[2];
			
			for(i = 0;  i < length; i++)
			{
				array[i] = buffer1[i+3];
			}
			
			for(i = 0; i < length; i++)
			{
				for(j = 0; j < length-i-1; j++)
				{
					if(array[j] > array[j+1])
					{
						temp = array[j];
						array[j] = array[j+1];
						array[j+1] = temp;
					}
				}
			}
			
			for(i = 0; i < length; i++)
			{
				if(currPos < array[i])
				{
					index = i;
					break;
				}
			}
			
			/*HEAD MOVES TOWRDS SMALLER DISK NUMBERS*/
			if(prevPos - currPos > 0) 
			{
				for(i = index-1 ; i >= 0; i--)
				{
					difference = abs(array[i] - currPos);
					movements = movements + difference;
					currPos = array[i];
				}
				
				difference = array[i+1]; 
				movements =  movements + difference;
				
				difference = cylinderNum - 1; 
				movements =  movements + difference;
				
				currPos = cylinderNum - 1;
				
				for(i = length-1; i >= index; i--)
				{
					difference = abs(array[i] - currPos);
					movements = movements + difference;
					currPos = array[i];
				}
			
			/*HEAD MOVES TOWARDS LARGER DISK NUMBERS*/
			}else{
			
				for(i = index; i < length; i++)
				{
					difference = abs(array[i] - currPos);
					movements = movements + difference;
					currPos = array[i];
				}
				
				movements = movements + abs(cylinderNum - array[i-1]-1);
				
				difference = cylinderNum - 1; 
				movements =  movements + difference;
				
				currPos = 0;
				
				for(i = 0; i < index; i++)
				{
					difference = abs(array[i] - currPos);
					movements = movements + difference;
					currPos = array[i];
				}
			}
		}
		pthread_mutex_unlock(&start_mutex);
		
		if(check != 0)
		{	
			/*CSCAN WAITS UNTIL SCAN(THREAD C) FINISHES WRITING TO BUFFER 2*/
			pthread_mutex_lock(&cscan_mutex);
			if(cscan_write == FALSE)
			{
				pthread_cond_wait(&cscan_cond,&cscan_mutex);
			}
			cscan_write = FALSE;
			pthread_mutex_unlock(&cscan_mutex);
			
			pthread_mutex_lock(&writer_mutex);
			if(emptyspaces == 0)
			{
				pthread_cond_wait(&empty,&writer_mutex);
			}
			buffer2[0] = movements;
			emptyspaces = 0;
			pthread_cond_signal(&readbuffer2);
			look_write = TRUE;
			
			/*WAKES UP LOOK (THREAD E) TO WRITE TO BUFFER*/
			pthread_cond_signal(&look_cond);
			pthread_mutex_unlock(&writer_mutex);
		}
				
		free(array); movements = 0; difference = 0; temp = 0;
		
		pthread_mutex_lock(&input_mutex);
		if(hasFile == FALSE)
		{
			pthread_cond_wait(&got_filename, &input_mutex);
		}
		pthread_mutex_unlock(&input_mutex);
	
	}while(check != 0);
	
	pthread_mutex_lock(&end_mutex);
	if(isEnd == FALSE)
	{
		pthread_cond_wait(&terminate, &end_mutex);
	}
	printf("%ld has terminated\n",pthread_self());
	pthread_mutex_unlock(&end_mutex);
	pthread_exit(NULL);
}
	
void *look(void *arg)
{
	int movements = 0; int difference = 0; int temp = 0; int currPos = 0;
	int prevPos = 0; int index = 0; int i = 0; int j = 0; int length = 0; int* array;
		
	do
	{	
		pthread_mutex_lock(&start_mutex);
		if(check != 0)
		{
			if(bufferReady == FALSE)
			{
				pthread_cond_wait(&readbuffer1, &start_mutex);
			}
			
			length  = count - 3;
			array = malloc((length * sizeof(int)));
	
			currPos = buffer1[1];
			prevPos = buffer1[2];

			for(i = 0;  i < length; i++)
			{
				array[i] = buffer1[i+3];
			}
		
			for(i = 0; i < length; i++)
			{
				for(j = 0; j < length-i-1; j++)
				{
					if(array[j] > array[j+1])
					{
						temp = array[j];
						array[j] = array[j+1];
						array[j+1] = temp;
					}
				}
			}
			
			for(i = 0; i < length; i++)
			{
				if(currPos < array[i])
				{
					index = i;
					break;
				}
			}
			
			/*HEAD MOVES TOWRDS SMALLER DISK NUMBERS*/
			if(prevPos - currPos > 0) 
			{
				for(i = index-1 ; i >= 0; i--)
				{
					difference = abs(array[i] - currPos);
					movements = movements + difference;
					currPos = array[i];
				}
					
				for(i = index; i < length; i++)
				{
					difference = abs(array[i] - currPos);
					movements = movements + difference;
					currPos = array[i];
				}
					
			/*HEAD MOVES TOWARDS LARGER DISK NUMBERS*/
			}else{
			
				for(i = index; i < length; i++)
				{
					difference = abs(array[i] - currPos);
					movements = movements + difference;
					currPos = array[i];
				}
				
				for(i = index - 1; i >= 0; i--)
				{
					difference = abs(array[i] - currPos);
					movements = movements + difference;
					currPos = array[i];
				}
			}
		}
		pthread_mutex_unlock(&start_mutex);
		
		if(check != 0)
		{
			/*LOOK WAITS UNTIL CSCAN(THREAD D) FINISHES WRITING TO BUFFER 2*/
			pthread_mutex_lock(&look_mutex);
			if(look_write == FALSE)
			{
				pthread_cond_wait(&look_cond,&look_mutex);
			}
			look_write = FALSE;
			pthread_mutex_unlock(&look_mutex);
			
			pthread_mutex_lock(&writer_mutex);
			if(emptyspaces == 0)
			{
				pthread_cond_wait(&empty,&writer_mutex);
			}
			buffer2[0] = movements;
			emptyspaces = 0;
			pthread_cond_signal(&readbuffer2);
			
			/*WAKES UP CLOOK (THREAD E) TO WRITE TO BUFFER*/
			clook_write = TRUE;
			pthread_cond_signal(&clook_cond);
			pthread_mutex_unlock(&writer_mutex);
		}
				
		free(array); movements = 0; difference = 0; temp = 0;
		
		pthread_mutex_lock(&input_mutex);
		if(hasFile == FALSE)
		{
			pthread_cond_wait(&got_filename, &input_mutex);
		}
		pthread_mutex_unlock(&input_mutex);
		
	}while(check != 0);
	
	pthread_mutex_lock(&end_mutex);
	if(isEnd == FALSE)
	{
		pthread_cond_wait(&terminate, &end_mutex);
	}
	printf("%ld has terminated\n",pthread_self());
	pthread_mutex_unlock(&end_mutex);
	pthread_exit(NULL);
}	

void *clook(void *arg)
{
	int movements = 0; int difference = 0; int temp = 0; int currPos = 0;
	int prevPos = 0; int index = 0; int i = 0; int j = 0; int length = 0; int* array;
		
	do
	{
		pthread_mutex_lock(&start_mutex);
		if(check != 0)
		{
			if(bufferReady == FALSE)
			{
				pthread_cond_wait(&readbuffer1, &start_mutex);
			}
			
			length  = count - 3;
			array = malloc((length * sizeof(int)));
	
			currPos = buffer1[1];
			prevPos = buffer1[2];

			for(i = 0;  i < length; i++)
			{
				array[i] = buffer1[i+3];
			}
			
			for(i = 0; i < length; i++)
			{
				for(j = 0; j < length-i-1; j++)
				{
					if(array[j] > array[j+1])
					{
						temp = array[j];
						array[j] = array[j+1];
						array[j+1] = temp;
					}
				}
			}
			
			for(i = 0; i < length; i++)
			{
				if(currPos < array[i])
				{
					index = i;
					break;
				}
			}
			
			/*HEAD MOVES TOWRDS SMALLER DISK NUMBERS*/
			if(prevPos - currPos > 0) 
			{
				for(i = index-1 ; i >= 0; i--)
				{
					difference = abs(array[i] - currPos);
					movements = movements + difference;
					currPos = array[i];
				}
				
				for(i = length-1; i >= index; i--)
				{
					difference = abs(array[i] - currPos);
					movements = movements + difference;
					currPos = array[i];
				}
			
			/*HEAD MOVES TOWARDS LARGER DISK NUMBERS*/
			}else{
			
				for(i = index; i < length; i++)
				{
					difference = abs(array[i] - currPos);
					movements = movements + difference;
					currPos = array[i];
				}
				
				for(i = 0; i < index; i++)
				{
					difference = abs(array[i] - currPos);
					movements = movements + difference;
					currPos = array[i];
				}
			}
			
		}
		pthread_mutex_unlock(&start_mutex);
		
		if(check != 0)
		{
			/*CLOOK WAITS UNTIL LOOK(THREAD E) FINISHES WRITING TO BUFFER 2*/
			pthread_mutex_lock(&clook_mutex);
			if(clook_write == FALSE)
			{
				pthread_cond_wait(&clook_cond,&clook_mutex);
			}
			clook_write = FALSE;
			pthread_mutex_unlock(&clook_mutex);
			
			pthread_mutex_lock(&writer_mutex);
			if(emptyspaces == 0)
			{
				pthread_cond_wait(&empty,&writer_mutex);
			}
			buffer2[0] = movements;
			emptyspaces = 0;
			pthread_cond_signal(&readbuffer2);
			pthread_mutex_unlock(&writer_mutex);
		}
		
		free(array); movements = 0; difference = 0; temp = 0; 
		
		pthread_mutex_lock(&input_mutex);
		if(hasFile == FALSE)
		{
			pthread_cond_wait(&got_filename, &input_mutex);
		}
		pthread_mutex_unlock(&input_mutex);
		
	}while(check != 0);
	
	pthread_mutex_lock(&end_mutex);
	if(isEnd == FALSE)
	{
		pthread_cond_wait(&terminate, &end_mutex);
	}
	printf("%ld has terminated\n",pthread_self());
	pthread_mutex_unlock(&end_mutex);
	pthread_exit(NULL);
}	
