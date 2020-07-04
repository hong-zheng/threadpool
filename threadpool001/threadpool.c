#include "threadpool.h"
#include <time.h>
#include <errno.h>
// 手撸线程池
/*
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
*/

void threadpool_init( threadpool_t* pool , int max_thread ) {
	pthread_mutex_init(&pool->mutex , NULL ) ;
	pthread_cond_init(&pool->cond , NULL ) ;
	pool->first = NULL ;
	pool->last  = NULL ;
	pool->counter = 0 ;
	pool->idle    = 0 ;
	pool->max_thread = max_thread ;
	pool->quit    = 0 ; //默认为0，没有销毁，否则为销毁
} 

void* routine( void* arg ) {
	//传过来的参数为pool,是一个线程池	
	threadpool_t* pool = (threadpool_t*)arg ;
//不同的线程进来之后就直接分路使用，因此是死循环
	
	int timeout = 0 ;
	printf("%p thread come\n",pthread_self()) ;
	while(1){
		timeout = 0 ;
		// 由于此处要操作任务队列，所有要上锁	
		pthread_mutex_lock(&pool->mutex) ;
// 才创建量一个线程，还没有被调用，因此空闲线程需要idle++ 
		pool->idle++ ;
// 如果线程池中没有任务且没有被销毁的时候就等待在此处，等待任务的来临,当有新任务插入时就signal此处的wait
		while( pool->first == NULL && pool->quit == 0 ) {
			struct timespec abstime ;
			clock_gettime( CLOCK_REALTIME ,&abstime) ;
			abstime.tv_sec += 5 ;
			int ret = pthread_cond_timedwait(&pool->cond , &pool->mutex,&abstime) ;
			if( ret == ETIMEDOUT ){
				timeout = 1 ;
				printf("%p thread timeout\n",pthread_self()) ;
				break ;
			}
		}
// 此时获得一个空闲线程，因此空闲线程需要减少一个
		pool->idle-- ;

// 需要确保里面还有任务可以执行，防止被其他线程取走
		if( pool->first != NULL ) {
			// 取出头任务
			task_t* tmp = pool->first ;
			// 更新头节点所在的任务
			pool->first = tmp->next ;
	//防止run函数执行时间过长，而导致插入任务队列而阻塞，因此此处解锁	
			pthread_mutex_unlock(&pool->mutex) ;
			// 执行当前任务节点
			tmp->run(tmp->arg) ;
			// 释放当前任务节点
			free(tmp) ;
			//解锁和上锁必须配对使用
			pthread_mutex_lock(&pool->mutex) ;
		}

// 防止阻塞的时候添加任务
// 如果超时且没有任务可以处理，就退出一个线程
		if( pool->first == NULL && timeout == 1) {
			pool->counter-- ;
		// 跳出循环得先解锁
			pthread_mutex_unlock(&pool->mutex) ;
			break ;
		}

// 销毁一个线程，已经退出，无任务
		if( pool->first == NULL && pool->quit == 1 ){
			pool->counter-- ;
// 最后一个线程销毁之后，整个线程池就真正销毁
// 最后一个线程退出，向destroy发通知
			if( pool->counter == 0 ) {
				pthread_cond_signal(&pool->cond) ;
			}
			pthread_mutex_unlock(&pool->mutex) ;
			break ;
		}
		pthread_mutex_unlock(&pool->mutex) ;
	}

// 线程离开循环就表示线程退出
	printf("%p thread exit\n",pthread_self()) ;
}
void threadpool_add_task( threadpool_t* pool , void* (*run)(void* arg) , void* arg ) {
	// 创建一个任务节点	
	task_t* new_task = malloc(sizeof(task_t)) ; 
	// 给当前任务节点填充内容
	new_task->run = run ;
	new_task->arg = arg ;
	
	// 向任务队列中添加任务
	// 需要改动任务队列，就需要上锁
	// 有上锁必须有解锁，上锁和解锁同时使用
	pthread_mutex_lock(&pool->mutex) ;
	if( pool->first == NULL ) {
		pool->first = new_task ;
	} else {
		task_t* last = pool->last ;
		last->next  = new_task ;
	}
// 无论是插在头节点还是插在尾节点后面，都需要更新任务队列尾
	pool->last  = new_task ;

	// 此时任务队列中已经有任务存在
	// 需要观测是否有空闲线程，如果有就唤醒
	if( pool->idle > 0 ) {
		pthread_cond_signal(&pool->cond) ;
// 当已有所有线程数量没有到达上线时才会创建，否则不会创建
	} else  if ( pool->counter < pool->max_thread ) {
	// 如果没有就创建
		pthread_t tid ;
// 创建线程，执行routine , 将参数pool传过去，因为创建这个线程属于pool线程池中
		pthread_create( &tid , NULL , routine , pool ) ;
		pool->counter++ ;
	}
	pthread_mutex_unlock(&pool->mutex) ;
}  
void threadpool_destroy( threadpool_t* pool ) {
// 可能被其他进程销毁线程池，这种情况就直接返回,防止阻塞
	if( pool->quit == 1 ) {
		return ;
	}
// 改动任务队列，就需要上锁
	pthread_mutex_lock(&pool->mutex) ;
	pool->quit = 1 ;
	printf("kill threadpool\n") ;
	if( pool->counter > 0 ) {
// 还有一些任务在执行，没有收到结束线程
// 正在执行的任务，后再通知
//
// 当线程池中空闲线程>0
		
		if ( pool->idle > 0 ) {
// 唤醒空闲线程
			pthread_cond_broadcast(&pool->cond) ;
		}
// 线程池中最后一个线程离开时就唤醒
// 如果销毁线程池时，线程池中的线程正在执行任务
// 不会收到broadcast通知，就阻塞在此，等待最后一个
		while( pool->counter > 0 ) {
			pthread_cond_wait(&pool->cond,&pool->mutex) ;
		}
	} 
	pthread_mutex_unlock(&pool->mutex) ;
	pthread_cond_destroy(&pool->cond) ;
	pthread_mutex_destroy(&pool->mutex) ;
} 
		

