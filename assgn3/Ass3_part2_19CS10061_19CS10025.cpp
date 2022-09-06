#include <iostream>
#include <vector>
#include <stdio.h>
#include <unistd.h> 
#include <stdlib.h> 
#include <time.h>
#include <pthread.h>
#include <sys/ipc.h> 
#include <sys/shm.h> 
#include <sys/time.h>
#include <chrono>
#define max_queue 8
// #define max_size 8
#define N 1000
using namespace std;

typedef struct{
	vector<vector<int>> mat_;
	int prod_num, matrix_id, size_x, size_y, time;
} matrix;


typedef struct{
	matrix matrix_q[max_queue];
	int max_size, size, job_created, matrices_gen;
	pthread_mutex_t lock;
} _queue;

matrix create_matrix(int prod_num, int matrix_id);
_queue * create_q(int max_size, int shmid1);
void print_matrix(matrix mat);
void insert_Q(_queue* q, matrix mat);
matrix remove(_queue* q);
matrix multiply(matrix m1, matrix m2);

void delay(float number_of_seconds) {
    clock_t start_time = clock(); 
    while (clock() < start_time + number_of_seconds*CLOCKS_PER_SEC); 
}


int main(){
	srand(time(0));
    int NP,NW,NM,max_jobs;
	int max_size = 8;
	cout<<"Producers: ";
	cin>>NP;
	cout<<"Consumers: ";
	cin>>NW;
	cout<<"No of matrices: ";
	cin>>NM;

    key_t key1 = 250;
	// create shared memory
	int shmid1 = shmget(key1,sizeof(_queue),0660|IPC_CREAT);
	if (shmid1<0) {
		cout<<"Failed to allocate shared memory!\n";
		exit(1);
	}
    _queue *Qu = create_q(max_size, shmid1);


//
	pid_t pid;
	cout<<"ProdNum    Status    Matrix_size    Matrix_Id"<<endl;
	// creating  producers
	auto start = std::chrono::high_resolution_clock::now();
	for(int i=1;i<=NP;++i){
		pid = fork();
		if(pid<0) 
			cout<<"ERROR!! PRODUCER CREATION FAILED.";
		else if(pid==0){
			//differed seed for each child process
			srand(time(0) ^ i*7);
			int mat_id=getpid();
			while(1){
				// exit
				if(Qu->job_created>=max_jobs) 
					break;
				// random delay
				delay((float) (rand()%4));
				while(1){
					// locking shared memory for job insertion
					pthread_mutex_lock(&Qu->lock);
					if(Qu->job_created>=max_jobs){
						pthread_mutex_unlock(&Qu->lock);
						break;
					}
					// create job
					matrix mat = create_matrix(i, mat_id);
					if((Qu->size)<(Qu->max_size)){
						Qu->job_created++;					// increment job counter
						insert_Q(Qu,mat);						// insert job
						cout<<"Created\t"<<Qu->job_created<<"\t\t"<<Qu->matrices_gen<<"\t"<<Qu->size<<endl;
						// unlocking shared memory
						pthread_mutex_unlock(&Qu->lock);
						break;
					}
					// unlocking shared memory
					pthread_mutex_unlock(&Qu->lock);
				}
			}
			return 0;
		}
	}

	// creating consumers(workers)
	for(int i=1;i<=NW;++i){
		pid = fork();
		if(pid<0) 
			cout<<"ERROR!! CONSUMER CREATION FAILED.";
		else if(pid==0){
			//different seed for each child process
			srand(time(0) ^ i);
			int proc_id=getpid();
			while(1){
				// exit 
				if(Qu->job_created==max_jobs) 
					break;
				// random delay
				delay((float) (rand()%4));
				while(1){
					// locking shared memory for job removal
					pthread_mutex_lock(&Qu->lock);
					if((Qu->job_created)==max_jobs){
						pthread_mutex_unlock(&Qu->lock);
						break;
					}
					if((Qu->size)>0){
						matrix mat = remove(Qu);
						delay((float)mat.time);
						cout<<mat.prod_num<<"\tcreated  "<<mat.size_x<<"*"<<mat.size_y<<"\t-\t"<<mat.matrix_id<<"\t"<<mat.time<<endl;
						(Qu->job_created) = (Qu->job_created) + 1;
						// releasing the lock on shared memory
						pthread_mutex_unlock(&Qu->lock);
						break;
					}
					// releasing the lock on shared memory
					pthread_mutex_unlock(&Qu->lock);
				}	
			}
			return 0;
		}		
	}
    return 0;
}


matrix create_matrix(int prod_num, int matrix_id){
	matrix mat;
	mat.prod_num = prod_num;
	mat.time = rand()%4 + 1;
	mat.matrix_id = rand()%100000+1;
	for(int i=0; i<N; i++){
		vector<int> temp;
		for(int j=0; j<N; j++){
			int x = rand()%2;
			if(x==2)
				temp.push_back( rand()%10);
			else
				temp.push_back(-1 * rand()%10);
		}
		mat.mat_.push_back(temp);
	}
	return mat;
}


_queue* create_q(int max_size, int shmid1){
	_queue* Qu = (_queue*)shmat(shmid1,(void*)0,0);
	Qu->size=0;
	Qu->job_created=0;
	Qu->matrices_gen=0;
	Qu->max_size=max_size;

	// initialising mutex locking
	pthread_mutexattr_t lock_attr;
	pthread_mutexattr_init(&lock_attr);
	pthread_mutexattr_setpshared(&lock_attr, PTHREAD_PROCESS_SHARED);
	pthread_mutex_init(&Qu->lock, &lock_attr);
	return Qu;
}


void print_matrix(matrix mat){
	cout<<"\nMatrix ID:"<<mat.matrix_id;
	cout<<"\nProducer Number:"<<mat.prod_num;
	cout<<"\nTime:"<<mat.time;

	for(int i=0; i<N; i++){
		for(int j=0; j<N; j++){
			cout<<mat.mat_[i][j]<<" ";
		}
		cout<<endl;
	}
}

void insert_Q(_queue* Qu, matrix mat){
	Qu->size++;
	(Qu->matrix_q)[Qu->size] = mat;
	// int i=Qu->size;
	
	return;
}

matrix remove(_queue* q){
	matrix mat = create_matrix(1,1);
	return mat;
}