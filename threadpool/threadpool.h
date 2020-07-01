#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
// ����ڵ�
typedef struct task {
        void* (*run)(void*arg) ; //����ص�����
        void* arg ; //�ٵ������Ĳ���
        struct task* next ; //��һ������ڵ�
} task_t ;


typedef struct threadpool{
        pthread_mutex_t mutex ;
        pthread_cond_t  cond ;
        task_t*         first ; //������ж�ͷ
        task_t*         last ;//������ж�β
        int             counter ; // �̳߳����ж��ٸ��߳�
        int             idle ;  //�����̸߳���  
        int             max_thread ; // �̳߳����̷߳���ֵ
        int             quit ; //�̳߳�Ĭ��Ϊ0���ٱ�־
} threadpool_t ;
void threadpool_init( threadpool_t* pool , int max_thread) ;

void threadpool_add_task( threadpool_t* pool , void* (*run)(void* arg) , void* arg ) ; 

void threadpool_destroy( threadpool_t* pool ) ;
