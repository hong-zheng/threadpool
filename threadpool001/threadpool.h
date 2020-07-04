#pragma once 

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>

typedef struct task {
	void* (*run)(void* arg) ;
	void* arg ;
	struct task* next ;	
} task_t ;

typedef struct threadpool {
	pthread_mutex_t mutex ; //操作任务队列的时候需要上锁，防止其他进程同时操作	
	pthread_cond_t 	cond ; //当插入任务队列的时候，插入之后判断有空闲进程就将其唤醒
	task_t*		first ; // 任务队头
	task_t*		last ; // 任务队尾
	int		counter ; //所有存在的线程数量
	int 		idle ; // 空闲着的线程数量
	int 		max_thread ; //当前线程池中能够创建的线程的最大数量
	int		quit ; //当前线程池是否被其他线程销毁
} threadpool_t ;


void threadpool_init( threadpool_t* pool , int max_thread ) ;
void threadpool_add_task( threadpool_t* pool , void* (*run)(void* arg) , void* arg ) ;  
void threadpool_destroy( threadpool_t* pool ) ; 
		

