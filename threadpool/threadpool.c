#include "threadpool.h"
// ����ڵ�
/*
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
//��δ�õ������ʱ�����������һ�������̣߳���Ϊ��û������ִ��
                pool->idle++ ;
                while( pool->first == NULL && pool->quit == 0 ) {
                        pthread_cond_wait(&pool->cond,&pool->mutex) ;
                }
// ���õ�����ִ�е�ʱ��Ͳ���һ�������̣߳��õ�����
                pool->idle-- ;

//ִ������
                if( pool->first != NULL ) { //������ִ������ص�����
//ȡ����������е�����ڵ�,�������еĺ�����������лص�ʵ�ֺ���
                                 task_t* tmp = pool->first ;
                        pool->first = tmp->next ;
//�����ǰִ������ʱ���ر𳤣����ǻ��������棬���������޷��Ž�������У�����ɳ�ͻ������Ϊ��ִ�д˺���
�ڼ�û��ռ�ù�����Դ����˿����ڴ�֮ǰ������Ȼ�������������������Ӧ��ͬʱ����    
                        pthread_mutex_unlock(&pool->mutex) ;
                        tmp->run(tmp->arg) ;
//ִ������оͽ��ڵ��ͷŵ�ǰ�ڵ�����
                        free(tmp) ;
//�������֮�����ڵû��ػ�ȥ
                        pthread_mutex_lock(&pool->mutex) ;
                }
                pthread_mutex_unlock(&pool->mutex) ;
        }
}
void threadpool_add_task( threadpool_t* pool , void* (*run)(void* arg) , void* arg ){
        task_t *new_task = malloc(sizeof(task_t)) ; //�����µ�����ڵ�
        new_task->run = run ;
        new_task->arg = arg ;

  pthread_mutex_lock(&pool->mutex) ;

// ������������������,��Ҫ��������Ϊ�����ǹ�����Դ�������ߺ������߶���ı����
        if( pool->first == NULL ) {
                pool->first = new_task ;
        } else {
                pool->last->next = new_task ;
        }
        pool->last = new_task ;

        if ( pool->idle > 0 ) {
//����п����̣߳���ֱ�ӻ��ѿ����̣߳�ִ�е�ǰ����      
                pthread_cond_signal(&pool->cond) ;
        } else if ( pool->counter < pool->max_thread ) {
//���û�п����߳̿�������ִ�е�ǰ���񣬲������û�дﵽ���ߣ��������߳�
                pthread_t tid ;
//����һ���µ��̷߳����̳߳���,���̳߳ش���ȥ������ߣ���ܶ��̳߳��е���Ϣ
                pthread_create(&tid,NULL,routine,pool) ;
                pool->counter++ ;
        }
        pthread_mutex_unlock(&pool->mutex) ;
}

void threadpool_destroy( threadpool_t* pool ){
}
