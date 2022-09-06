#include<stdio.h>
#include <sys/types.h>
#include <sys/ipc.h> 
#include <sys/shm.h> 
#include <stdlib.h>
#include<unistd.h>
#include<sys/wait.h>

typedef struct _process_data {
double *A;
double *B;
double *C;
int veclen, i, j,column;
} ProcessData;

void mult(ProcessData *arg)
{
	
	int m= arg->veclen;
	double sum=0;
	for(int k=0;k<m;k++)
	{
			//printf("%lf\n",arg->A[(arg->i*m)+k]);
			//printf("%lf,%d,%d\n",arg->B[(k*(m-1))+arg->j]);

			sum+=((arg->A[(arg->i*m)+k])*(arg->B[(k*(arg->column))+arg->j]));

	}
	
	arg->C[(arg->i*m)+arg->j]=sum;
	//printf("%lf,%lf\n",sum,arg->C[(arg->i*m)+arg->j]);

	return ;
}

int main()
{
	int r1,c1,r2,c2;
	int shmid,shmid1,shmid2,shmid3;
	//key_t key1=35,key2=1035,key3=2035;

	pid_t child;
	int i,status;
	printf("Enter the rows and columns of matrix 1:\n");
	scanf("%d,%d",&r1,&c1);
	printf("Enter the rows and columns of matrix 2:\n");
	scanf("%d,%d",&r2,&c2);

	if(c1!=r2){
		printf("c1 and r2 should be equal\n");
		exit(0);
	}
	ProcessData *arg;

	shmid1=shmget(IPC_PRIVATE,r1*c1*sizeof(double),0666|IPC_CREAT);
	arg->A = (double*)shmat(shmid1,(void*)0,0);

	shmid2=shmget(IPC_PRIVATE,r2*c2*sizeof(double),0666|IPC_CREAT);
	arg->B = (double*)shmat(shmid2,(void*)0,0);

	shmid3=shmget(IPC_PRIVATE,r1*c2*sizeof(double),0666|IPC_CREAT);
	arg->C = (double*)shmat(shmid3,(void*)0,0);

	printf("Enter the values for matrix 1:\n");
	for(int i=0;i<r1;i++){
		for(int j=0;j<c1;j++){
			scanf("%lf",&(arg->A[(i*c1)+j]));

		}
		
	}
	
	printf("Enter the values for matrix 2:\n");
	for(int i=0;i<r2;i++){
		for(int j=0;j<c2;j++){
			scanf("%lf",&(arg->B[(i*c2)+j]));
		}
	}

	arg->veclen= c1;
	for(int k=0;k<r1;k++){
		for(int l=0;l<c2;l++){
			arg->i=k;
			arg->j=l;
			arg->column=c2;
			child=fork();
            if(child==0{
				mult(arg);
				printf("%lf\n",arg->C[(arg->i*c1)+arg->j] );
				exit(EXIT_SUCCESS);
			}
		}
	}
	
	while(wait(NULL)>0);

  	for(int i=0;i<r1;i++){
  		for (int j = 0; j < c2; j++){
  			printf("%lf parent\n",arg->C[(i*c1)+j] );
  		}
  	}
   
	shmdt(arg->A);
	shmdt(arg->B);
	shmdt(arg->C);

	shmctl(shmid1,IPC_RMID,NULL);
	shmctl(shmid2,IPC_RMID,NULL);
	shmctl(shmid3,IPC_RMID,NULL);
	
return 0;

}