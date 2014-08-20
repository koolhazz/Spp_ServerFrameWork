
#ifndef _COMM_LOCK_H_
#define _COMM_LOCK_H_

#include <stdarg.h>
#include <pthread.h>
#include <sys/sem.h>

namespace comm
{
namespace lock
{

//锁类型
enum Lock_Type
{
    LOCK_NONE = 0,			//当不需要
    LOCK_THREAD_SAFE,		//确保多线程安全写日志
    LOCK_MPROCESS_SAFE,	//确保多进程写日志安全
    LOCK_INVALID
};

////////////////////////////////
union logsemun
{
    int val;					//<= value for SETVAL
    struct semid_ds *buf;		//<= buffer for IPC_STAT & IPC_SET
    unsigned short int *array;	//<= array for GETALL & SETALL
    struct seminfo *__buf;		//<= buffer for IPC_INFO
};


class CLock
{
public:
    CLock(){};
    virtual ~CLock(){}

    virtual int Lock() = 0;
    virtual int UnLock() = 0;
};

class CLockGuard
{
public:
    CLockGuard(CLock * Lock){Lock_ = Lock;if(Lock_ != NULL ) Lock_->Lock();}
    ~CLockGuard(){if( Lock_  != NULL) Lock_->UnLock();}
private:
    CLock * Lock_ ;
};

class CNullLock:public CLock
{
public:
    CNullLock(){}
    virtual ~CNullLock(){}

    virtual int Lock(){return 0;}
    virtual int UnLock(){return 0;}
};


class CThreadLock:public CLock
{
public:
    CThreadLock(){pthread_mutex_init( &mutex_, NULL );}
    ~CThreadLock(){pthread_mutex_destroy( &mutex_ );}

    virtual int Lock(){return pthread_mutex_lock(&mutex_);}
    virtual int UnLock(){return pthread_mutex_unlock(&mutex_);}
private:
    pthread_mutex_t mutex_;
};

class CSemLock:public CLock
{
public:
    CSemLock(){}
    ~CSemLock(){}

    void sem_attach(int semid){semid_ = semid;}
    int sem_init(int key);
    virtual int Lock(){return sem_lock();}
    virtual int UnLock(){return sem_unlock();}
	
private:

    int sem_lock();
    int sem_unlock();
    int semid_;
};


}

}

#endif

