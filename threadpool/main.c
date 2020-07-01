#include "threadpool.h"

void* myfun(void* arg){
        int id = *(int*)arg ;
        free(arg) ;
        printf("id = %d , thread_id = %p start work\n",id,pthread_self()) ;
        sleep(5) ;
        printf("id = %d , thread_id = %p exit work\n",id,pthread_self()) ;
}
void main(){
        threadpool_t pool ;

        threadpool_init(&pool , 3 ) ; 
    
        int i ; 
        for(i=0;i<5;++i){
                int* p = malloc(sizeof(int)) ;
                *p = i ; 
                threadpool_add_task(&pool,myfun,(void*)p) ;
        }

        sleep(10) ;
        printf("main exit\n") ;
        threadpool_destroy(&pool) ;
}
