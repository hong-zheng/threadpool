#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
// 任务节点
typedef struct task {
        void* (*run)(void*arg) ; //任务回调函数
        void* arg ; //毁掉函数的参数
        struct task* next ; //下一个任务节点
} task_t ;


typedef struct threadpool{
        pthread_mutex_t mutex ;
        pthread_cond_t  cond ;
        task_t*         first ; //任务队列对头
        task_t*         last ;//任务队列队尾
        int             counter ; // 线程池中有多少个线程
        int             idle ;  //空闲线程个数  
        int             max_thread ; // 线程池中线程符阀值
        int             quit ; //线程池默认为0销毁标志
} threadpool_t ;
void threadpool_init( threadpool_t* pool , int max_thread) ;

void threadpool_add_task( threadpool_t* pool , void* (*run)(void* arg) , void* arg ) ; 

void threadpool_destroy( threadpool_t* pool ) ;
