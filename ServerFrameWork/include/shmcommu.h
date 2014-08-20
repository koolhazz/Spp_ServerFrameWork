/********************************************
//文件名:shmcommu.h
//功能:共享内存通讯类
//作者:钟何明
//创建时间:2009.06.11
//修改记录:

*********************************************/
#ifndef _COMM_COMMUNICATION_SHMCOMMU_H_
#define _COMM_COMMUNICATION_SHMCOMMU_H_

#include <map>

#ifndef SLK_LINUX
#include <pthread.h>
#else					 //slackware中linuxthread线程库不支持线程锁跨进程共享，所以要改用信号量
#include <sys/sem.h>
union shmsemun
{
    int val;					//<= value for SETVAL
    struct semid_ds *buf;		//<= buffer for IPC_STAT & IPC_SET
    unsigned short int *array;	//<= array for GETALL & SETALL
    struct seminfo *__buf;		//<= buffer for IPC_INFO
};
#endif


#include "commu.h"


//#define LOAD_CHECK_ENABLE
#ifdef LOAD_CHECK_ENABLE
#include "load.h"
#endif


using namespace comm::commu;

namespace comm 
{
namespace commu 
{
namespace shmcommu 
{
#define LOCK_TYPE_NONE			0x0		//不锁
#define LOCK_TYPE_PRODUCER		0x1		//写锁
#define LOCK_TYPE_COMSUMER		0x2		//读锁

#define COMMU_ERR_SHMGET	-101		//获取共享内存出错
#define COMMU_ERR_SHMNEW	-102		//创建共享内存出错
#define COMMU_ERR_SHMMAP	-103		//共享内存映射出错
#define COMMU_ERR_SEMGET	-104		//获取信号量出错
#define COMMU_ERR_SEMLOCK	-105		//信号量锁失败
#define COMMU_ERR_SEMUNLOCK	-106		//信号量解锁失败
#define COMMU_ERR_FILEOPEN 	-111 	//打开文件出错
#define COMMU_ERR_FILEMAP	-112		//文件映射出错
#define COMMU_ERR_MQFULL	-121		//管道满
#define COMMU_ERR_MQEMPTY  	-122		//管道空
#define COMMU_ERR_OTFBUFF	-123		//缓冲区溢出

//共享内存管道统计
typedef struct tagMQStat
{       
    unsigned usedlen_;
    unsigned freelen_;
    unsigned totallen_;
    unsigned shmkey_;
    unsigned shmid_;
    unsigned shmsize_;
}TMQStat;


//共享内存管道
class CShmMQ
{
public: 
    CShmMQ();
    ~CShmMQ();

    /******************************************
    //初始化
    //[in] shmkey:共享内存KEY
    //[in] shmsize:共享内存大小
    //返回值:
    //成功:0,失败:-1
    *****************************************/
    int init(int shmkey, int shmsize);

    /********************************************
    //功能:将数据放入共享内存管道
    //参数说明:
    //[in] arg1:数据块blob对象
    //[in] flow:通讯唯一标识
    //[in] msgtype:消息类型
    //返回值:
    //0成功，<0:失败
    ********************************************/
    int enqueue(const void * arg1, unsigned flow,unsigned char msgtype);

    /***************************************************
    //功能:从共享内存管道中读取数据
    //参数说明:
    //[out] arg1:数据块blob对象
    //[out] flow:通讯唯一标识
    //[out] type:数据包类型
    //返回值:
    //0成功，<0:失败
    ***************************************************/
    int dequeue(void* arg1, unsigned int & flow,unsigned char &msgtype);
    int dequeue_i(void* arg1, unsigned int & flow,unsigned char &msgtype,struct timeval & time_stamp);

    /****************************************************
    //功能:返回内存的地址
    //参数说明:无
    //返回值:共享内存的起始地址
    ****************************************************/
    inline void* memory() {return shmmem_;}

    /***************************************************
    //功能:获取共享内存管道的统计信息
    //参数说明:
    //[out] mq_stat:返回共享内存管道的统计信息
    //返回值:无
    ***************************************************/
    inline void getstat(TMQStat& mq_stat)
    {
        unsigned int head = *head_; 
        unsigned int tail = *tail_; 

        mq_stat.usedlen_ = (tail >= head) ? tail - head : tail + blocksize_ - head;
        mq_stat.freelen_ = head > tail ? head - tail: head + blocksize_ - tail;
        mq_stat.totallen_ = blocksize_;
        mq_stat.shmkey_ = shmkey_; 
        mq_stat.shmid_ = shmid_;
        mq_stat.shmsize_ = shmsize_;
    }
protected:
    int shmkey_;//共享内存的key
    int shmsize_;//共享内存的大小
    int shmid_;//共享内存的ID
    void* shmmem_;//共享内存的起始地址	

    unsigned int * head_;//the head of data section
    unsigned int * tail_;//the tail of data section
    char* block_; //data section base address
    unsigned int blocksize_;//data section length

    /*********************************************
    //功能:取得共享内存
    //参数说明:
    //[in] shmkey:共享内存的key
    //[in] shmsize:共享内存的大小
    //返回值:
    //0:表示已存在的共享内存，1:表示新创建的共享内存
    // <0表示失败
    *********************************************/
    int getmemory(int shmkey, int shmsize);

     /*****************************************
     //功能:析构资源
     //参数说明:无
     //返回值:无
     *****************************************/	
    void fini();

    public:
        int msg_timeout_;//超时时间        
};

//生产者（不带锁）
class CShmProducer
{
public:
    CShmProducer();
    virtual ~CShmProducer();

    /******************************************
    //初始化
    //[in] shmkey:共享内存KEY
    //[in] shmsize:共享内存大小
    //返回值:
    //成功:0,失败:-1
    *****************************************/
    virtual int init(int shmkey, int shmsize);

    /********************************************
    //功能:将数据放入共享内存管道
    //参数说明:
    //[in] arg1:数据块对象
    //[in] flow:通讯唯一标识
    //[in] type:数据包类型
    //返回值:
    //0成功，<0:失败
    ********************************************/
    virtual int produce(const void* arg1, unsigned flow,unsigned char type);

    /***************************************************
    //功能:获取共享内存管道的统计信息
    //参数说明:
    //[out] mq_stat:返回共享内存管道的统计信息
    //返回值:无
    ***************************************************/
    virtual void getstat(TMQStat& mq_stat)
    {
    	mq_->getstat(mq_stat);
    }

     /*****************************************
         //功能:析构资源
         //参数说明:无
         //返回值:无
         *****************************************/	
    virtual void fini();
public:
    CShmMQ* mq_;//共享内存管道对象指针
};


//生产者（带锁）
class CShmProducerL : public CShmProducer
{
public:
    CShmProducerL();
    ~CShmProducerL();

    /******************************************
    //初始化
    //[in] shmkey:共享内存KEY
    //[in] shmsize:共享内存大小
    //返回值:
    //成功:0,失败:-1
    *****************************************/
    int init(int shmkey, int shmsize);

    /********************************************
    //功能:将数据放入共享内存管道
    //参数说明:
    //[in] arg1:数据块blob对象
    //[in] flow:通讯唯一标识
    //[in] type:数据包类型
    //返回值:
    //0成功，<0:失败
    ********************************************/
    int produce(const void* arg1, unsigned flow,unsigned char type);
protected:
#ifndef SLK_LINUX			
    pthread_mutex_t* mutex_;
    int mfd_;
#else
    int semid_;//信号量ID

    /*****************************************
    //功能:信号量初始化
    //参数说明:
    //[in] key:信号量的key
    //返回值
    //成功0,失败-1
    *****************************************/
    int sem_init(int key);

    int sem_lock();

    int sem_unlock();
#endif	

    /*****************************************
    //功能:析构资源
    //参数说明:无
    //返回值:无
    *****************************************/	
    void fini();
};

//消费者（不带锁）	
class CShmComsumer
{
public:
    CShmComsumer();
    virtual ~CShmComsumer();

    /******************************************
    //初始化
    //[in] shmkey:共享内存KEY
    //[in] shmsize:共享内存大小
    //返回值:
    //成功:0,失败:-1
    *****************************************/
    virtual int init(int shmkey, int shmsize);

    /***************************************************
    //功能:从共享内存管道中读取数据
    //参数说明:
    //[out] arg1:数据块对象
    //[out] flow:通讯唯一标识
    //[out] type:数据包类型
    //[in] block:阻塞标志，true:阻塞调用,flase:非阻塞调用
    //返回值:
    //0成功，<0:失败
    ***************************************************/
    virtual int comsume(void* arg1, unsigned int& flow, unsigned char &msgtype,bool block = false);

    /***************************************************
    //功能:获取共享内存管道的统计信息
    //参数说明:
    //[out] mq_stat:返回共享内存管道的统计信息
    //返回值:无
    ***************************************************/
    virtual void getstat(TMQStat& mq_stat)
    {
        mq_->getstat(mq_stat);
    }

    /***************************************************
    //功能:从共享内存管道中读取数据
    //参数说明:
    //[out] arg1:数据块blob对象
    //[out] flow:通讯唯一标识
    //[out] msgtype:数据包类型
    //返回值:
    //0成功，<0:失败
    ***************************************************/
    virtual inline int comsume_i(void* arg1, unsigned& flow, unsigned char & type);

    /*****************************************
    //功能:析构资源
    //参数说明:无
    //返回值:无
    *****************************************/	
    virtual void fini();
public:
    CShmMQ* mq_;
	
};

//消费者（带锁）
class CShmComsumerL : public CShmComsumer
{
public:
    CShmComsumerL();
    ~CShmComsumerL();
    int init(int shmkey, int shmsize);
protected:
#ifndef SLK_LINUX			
    pthread_mutex_t* mutex_;
    int mfd_;
#else
    int semid_;
    int sem_init(int key);
    int sem_lock();
    int sem_unlock();
#endif

    /***************************************************
    //功能:从共享内存管道中读取数据
    //参数说明:
    //[out] arg1:数据块blob对象
    //[out] flow:通讯唯一标识
    //[out] msgtype:消息类型
    //返回值:
    //0成功，<0:失败
    ***************************************************/
    inline int comsume_i(void* arg1, unsigned& flow, unsigned char & msgtype);


    /*****************************************
    //功能:析构资源
    //参数说明:无
    //返回值:无
    *****************************************/	
    void fini();
};

//共享内存通讯组件
//一个发数据管道，称为生产者producer
//一个收数据管道，称为消费者comsumer
//回调函数类型：typedef int (*cb_func) (unsigned flow, void* arg1, void* arg2);(参考commu.h)
//flow: 数据包唯一标示；arg1: 数据blob；arg2: 用户自定义参数
//必须注册CB_RECVDATA回调函数
typedef struct 
{
    int shmkey_producer_;	           //发数据共享内存key
    int shmsize_producer_;          //发数据共享内存size
    int shmkey_comsumer_;        //收数据共享内存key
    int shmsize_comsumer_;       //收数据共享内存size
    int locktype_;		          //锁类型，不锁、写锁、读锁
    int expiretime_;                       //超时时间，如果为0表示不检查超时
    int maxpkg_;                            //每秒最大包量, 如果为0表示不检查
    int msg_timeout_;                      //消息在队列最长时间,expiretime_无用!
}TShmCommuConf;

class CTShmCommu : public CTCommu
{
public:
    CTShmCommu();
    ~CTShmCommu();

    /***************************************
    //功能:初始化共享内存通讯组件
    //参数说明
    //[in] config:配置信息的内存地址
    //返回值:
    //0:成功,-1:失败
    ***************************************/
    int init(const void* config);

    /**************************************
    //功能:收数据
    //参数说明:
    //[in] block:阻塞标记，true:阻塞收，false:非阻塞收
    //返回值:0,成功，<0:失败
    ***************************************/
    int poll(bool block = false);

    /***************************************************
    //功能:发数据
    //参数说明
    //[in] flow:数据包唯一标识
    //[in] arg1:数据blob指针
    //[in] arg2:暂时保留
    ****************************************************/
    int sendto(unsigned flow, void* arg1, void* arg2 /*暂时保留*/);

    int ctrl(unsigned flow, ctrl_type type, void* arg1, void* arg2);

    int get_msg_count();
protected:
    CShmProducer* producer_;//生产者
    CShmComsumer* comsumer_;//消费者
    unsigned int buff_size_;//数据块buff大小
    unsigned short buff_ext_size_;//扩展数据块buff大小
    blob_type buff_blob_;//数据块对象
    int locktype_;//锁类型
    std::map<unsigned, unsigned> expiremap_;
    int expiretime_;
    int lastchecktime_;
    //int msg_timeout_;    

#ifdef LOAD_CHECK_ENABLE
    CLoad myload_;
#endif

    /************************
    //功能:析构资源
    //参数说明:无
    //返回值:无
    ****************************/
    void fini();

    /************************************
    //功能:检查超时
    //参数说明:无
    //返回值:无
    *************************************/
    void check_expire();
};

}
}
}

#endif

