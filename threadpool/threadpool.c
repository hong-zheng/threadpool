#include "threadpool.h"
// 任务节点
/*
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

pthread_mutex_t mutex ;
pthread_cond_t cond ;
*/
void threadpool_init( threadpool_t* pool , int max_thread){
        pthread_mutex_init(&pool->mutex,NULL) ;
        pthread_cond_init(&pool->cond,NULL) ;
        pool->first      = NULL ;
        pool->last       = NULL ;
        pool->counter    = 0 ;
        pool->idle       = 0 ;
        pool->max_thread = max_thread ;
        pool->quit       = 0 ;
}


void* routine( void* arg ){
        threadpool_t* pool = (threadpool_t*)arg ;

        while(1){
                pthread_mutex_lock(&pool->mutex) ;
//还未得到任务的时候进来量就是一个空闲线程，因为还没有任务执行
                pool->idle++ ;
                while( pool->first == NULL && pool->quit == 0 ) {
                        pthread_cond_wait(&pool->cond,&pool->mutex) ;
                }
// 当拿到任务执行的时候就不是一个空闲线程，得到任务
                pool->idle-- ;

//执行任务
                if( pool->first != NULL ) { //有任务，执行任务回调函数
//取出任务队列中的任务节点,用任务中的函数与参数进行回掉实现函数
                                 task_t* tmp = pool->first ;
                        pool->first = tmp->next ;
//如果当前执行任务时间特别长，但是还在锁里面，后来任务无法放进任务队列，就造成冲突，诱因为在执行此函数
期间没回占用共享资源，因此可以在次之前解锁，然后再上锁，上锁与解锁应该同时出现    
                        pthread_mutex_unlock(&pool->mutex) ;
                        tmp->run(tmp->arg) ;
//执行完队列就将节点释放当前节点任务
                        free(tmp) ;
//上面解锁之后现在得还回回去
                        pthread_mutex_lock(&pool->mutex) ;
                }
                pthread_mutex_unlock(&pool->mutex) ;
        }
}
void threadpool_add_task( threadpool_t* pool , void* (*run)(void* arg) , void* arg ){
        task_t *new_task = malloc(sizeof(task_t)) ; //创建新的任务节点
        new_task->run = run ;
        new_task->arg = arg ;

  pthread_mutex_lock(&pool->mutex) ;

// 向任务队列中添加任务,需要上锁，因为队列是共享资源，消费者和生产者都会改变队列
        if( pool->first == NULL ) {
                pool->first = new_task ;
        } else {
                pool->last->next = new_task ;
        }
        pool->last = new_task ;

        if ( pool->idle > 0 ) {
//如果有空闲线程，就直接唤醒空闲线程，执行当前任务      
                pthread_cond_signal(&pool->cond) ;
        } else if ( pool->counter < pool->max_thread ) {
//如果没有空闲线程可以用来执行当前任务，并且如果没有达到上线，创建新线程
                pthread_t tid ;
//创建一个新的线程放在线程池中,将线程池床过去，可以撸到很多线程池中的信息
                pthread_create(&tid,NULL,routine,pool) ;
                pool->counter++ ;
        }
        pthread_mutex_unlock(&pool->mutex) ;
}

void threadpool_destroy( threadpool_t* pool ){
}
