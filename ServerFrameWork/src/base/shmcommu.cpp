#include <sys/shm.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/syscall.h>
#include <sys/mman.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>
#include <errno.h>
#include <asm/atomic.h>
#include <stdlib.h>
#include "shmcommu.h"
#include "misc.h"
#include "likelydef.h"


using namespace comm::base;

#define C_TM_PRIVATE_SIZE sizeof(struct timeval) //时间戳
#define C_PUB_HEAD_SIZE  8//包括totallen(4)+flow(4)
#define C_TYPE_HEAD_SIZE 3 //msg Type(1) + ext_len(2) 
#define C_HEAD_SIZE  (C_PUB_HEAD_SIZE+C_TM_PRIVATE_SIZE + C_TYPE_HEAD_SIZE)


using namespace comm::commu;
using namespace comm::commu::shmcommu;

CShmMQ::CShmMQ():shmkey_(0), shmsize_(0), shmid_(0), shmmem_(NULL), head_(NULL), tail_(NULL), block_(NULL), blocksize_(0)
{
    msg_timeout_ = 0;
}
CShmMQ::~CShmMQ()
{
    fini();
}
int CShmMQ::getmemory(int shmkey, int shmsize)
{
    if((shmid_ = shmget(shmkey, shmsize, 0)) != -1)  
    {
        shmmem_ = shmat(shmid_, NULL, 0);
        if(likely(shmmem_ != MAP_FAILED)) 
        {       
            return 0;
        }
        else
        {	
            return COMMU_ERR_SHMMAP;
        }
    }
    else
    {		
        shmid_ = shmget(shmkey, shmsize, IPC_CREAT | 0666);
        if(likely(shmid_ != -1))  
        {	
            shmmem_ = shmat(shmid_, NULL, 0);
            if(likely(shmmem_ != MAP_FAILED)) 
            {       
                return 1;
            }
            else
            {	
                return COMMU_ERR_SHMMAP;
            }	
        }
        else
        {
            return COMMU_ERR_SHMNEW;
        }
    }
}

void CShmMQ::fini()
{
    if(shmmem_)
        shmdt((const void*)shmmem_);	
}
int CShmMQ::init(int shmkey, int shmsize)
{
    fini();

    shmkey_ = shmkey;
    shmsize_ = shmsize;
    int ret = 0;
    if((ret = getmemory(shmkey_, shmsize_)) > 0)
    {
        memset(shmmem_, 0x0, 2* sizeof(unsigned int ));//初始化 head_ tail_的地址内容
    }
    else if(ret < 0)
    {
        return ret;
    }

    //the head of data section
    head_ = (unsigned int *)shmmem_;
    //the tail of data section
    tail_ = head_ + 1;
    //data section base address
    block_ = (char*)(tail_ + 1);
    //data section length
    blocksize_ = shmsize_ - 2 * sizeof(unsigned int ) ; 	

    return 0;
}

int CShmMQ::enqueue(const void * arg1, unsigned flow, unsigned char msgtype)
{
    unsigned int head = *head_;
    unsigned int tail = *tail_;
    unsigned int free_len = head>tail ? head - tail : head + blocksize_- tail;
    unsigned int tail_len = blocksize_ - tail;

    struct timeval tval;

    char sHead[C_HEAD_SIZE] = {0};

    blob_type * blob = (blob_type *) arg1;

    //写入的数据总长度=头信息长度+数据长度+扩展数据长度
    unsigned total_len = C_HEAD_SIZE + blob->len + blob->ext_len;
	
    // if no enough space?
    if(unlikely(free_len <= total_len))
        return COMMU_ERR_MQFULL;

        //Write TimeStamp
    get_timeofday(&tval);
    memcpy(sHead, &tval, C_TM_PRIVATE_SIZE);

    //Write TotalLen
    memcpy(sHead + C_TM_PRIVATE_SIZE, &total_len, sizeof(unsigned int));

    //Write Flow
    memcpy(sHead+C_TM_PRIVATE_SIZE + sizeof(unsigned int), &flow, sizeof(unsigned int));

    //Write Msg Type
    memcpy(sHead + C_TM_PRIVATE_SIZE + C_PUB_HEAD_SIZE,&msgtype,sizeof(unsigned char));	

    //write ext_len
    memcpy(sHead + C_TM_PRIVATE_SIZE + C_PUB_HEAD_SIZE + sizeof(unsigned char),&blob->ext_len,sizeof(unsigned short) );



    // if tail space > total_len
    //  copy C_HEAD_SIZE byte, copy data,copy extdata
    if (tail_len >= total_len)
    {
        memcpy(block_ + tail, sHead, C_HEAD_SIZE );//Write head
        memcpy(block_ + tail + C_HEAD_SIZE , blob->data,blob->len);//Write data
        memcpy(block_ + tail + C_HEAD_SIZE + blob->len,blob->extdata,blob->ext_len);//Write extdata
        *tail_ +=total_len;
    }
    //if tail space >= C_HEAD_SIZE + blob->len && < total_len
    else if( tail_len >= C_HEAD_SIZE + blob->len && tail_len < total_len )
    {
        //copy C_HEAD_SIZE byte
        memcpy(block_ + tail,sHead,C_HEAD_SIZE);

        //copy data
        memcpy(block_ + tail + C_HEAD_SIZE,blob->data,blob->len );

        unsigned int first_len = tail_len - C_HEAD_SIZE - blob->len;
        unsigned int second_len = blob->ext_len - first_len;

        //copy first data
        memcpy(block_ + tail + C_HEAD_SIZE + blob->len,blob->extdata,first_len );

        //copy left data
        memcpy(block_,((char *) blob->extdata) + first_len,second_len);

              unsigned int tmp_tail = tail + total_len - blocksize_ ;
        //*tail_ += total_len;
        //*tail_ -= blocksize_;
        *tail_ = tmp_tail;

    }
    //if tail space >=C_HEAD_SIZE && < C_HEAD_SIZE + blob->len
    else if(tail_len >= C_HEAD_SIZE && tail_len < C_HEAD_SIZE + blob->len )
    {
        //copy C_HEAD_SIZE byte
        memcpy(block_ + tail,sHead,C_HEAD_SIZE);

        //copy tail - C_HEAD_SIZE
        unsigned int first_len = tail_len - C_HEAD_SIZE;
        memcpy(block_ + tail + C_HEAD_SIZE,blob->data,first_len);

        //copy left data
        unsigned int second_len = blob->len - first_len;
        memcpy(block_,((char *) blob->data ) + first_len,second_len );

        //copy ext data
        memcpy(block_ + second_len,blob->extdata,blob->ext_len);

              unsigned int tmp_tail = tail + total_len - blocksize_;
        //*tail_ += total_len;
        //*tail_ -= blocksize_;
        *tail_ = tmp_tail;
    }
    //   if tail space < C_HEAD_SIZE
    else
    {
        //  copy tail byte
        memcpy(block_ + tail, sHead, tail_len);

        //  copy C_HEAD_SIZE-tail byte
        unsigned second_len = C_HEAD_SIZE - tail_len;
        memcpy(block_, sHead + tail_len, second_len);

        //  copy data
        memcpy(block_ + second_len, blob->data,blob->len);

        // copy ext data
        memcpy(block_ + second_len + blob->len,blob->extdata,blob->ext_len);

        *tail_ = second_len + blob->len + blob->ext_len;
    }
    return 0;
}

int CShmMQ::dequeue(void * arg1, unsigned int & flow, unsigned char & msgtype)
{

    struct timeval time_stamp;
    int ret = 0;
    while(1)
    {
        ret = dequeue_i(arg1,flow,msgtype,time_stamp);
        if(msg_timeout_ && ret == 0 )
        {
            //检查超时
            if( (CMisc::getdelay( time_stamp ) > (unsigned int )msg_timeout_ ))
            {
                continue;
            }
        }

        break;
    }

    return ret;

}

int CShmMQ::dequeue_i(void * arg1, unsigned int & flow, unsigned char & msgtype,struct timeval & time_stamp)
{
    unsigned int head = *head_;
    unsigned int tail = *tail_;

    blob_type *blob = (blob_type *) arg1;

    if(head == tail)
    {
        //没有数据
        blob->len = blob->ext_len = 0;
        return COMMU_ERR_MQEMPTY;
    }

    unsigned int used_len = tail > head ? tail - head : tail + blocksize_ - head;
    char sHead[C_HEAD_SIZE];

    unsigned int data_pos;

    //  if head + C_HEAD_SIZE > block_size
    if(head + C_HEAD_SIZE > blocksize_)
    {
        //不够一个header
        unsigned int first_size = blocksize_ - head;
        unsigned int second_size = C_HEAD_SIZE - first_size;
        memcpy(sHead, block_ + head, first_size);
        memcpy(sHead + first_size, block_, second_size);
        data_pos = second_size;
    }
    else
    {
        //足够一个header
        memcpy(sHead, block_ + head, C_HEAD_SIZE);
        data_pos = head + C_HEAD_SIZE;

        //已到内存边界，返回头部
        if(data_pos == blocksize_ )
        {
            data_pos = 0;
        }
    }	

    //  get meta data

    //时间戳
    memcpy(&time_stamp,sHead,C_TM_PRIVATE_SIZE);

    //header+数据 长度
    unsigned int total_len = *(unsigned int *)(&sHead[C_TM_PRIVATE_SIZE]);

    //flow
    flow = *(unsigned int *)(sHead + C_TM_PRIVATE_SIZE +sizeof(unsigned));

    //msg type
    msgtype = *(unsigned char *)(sHead + C_TM_PRIVATE_SIZE + C_PUB_HEAD_SIZE);

    blob->ext_type = msgtype;
    blob->ext_len = *(unsigned short *)(sHead + C_TM_PRIVATE_SIZE + C_PUB_HEAD_SIZE + sizeof(unsigned char ) );
    blob->len= total_len - C_HEAD_SIZE - blob->ext_len;

    assert(total_len <= used_len && blob->len <= (int) (total_len - C_HEAD_SIZE) && blob->len >= 0 );

    unsigned int data_len = total_len - C_HEAD_SIZE;

    if(unlikely(blob->len > MAX_BLOB_DATA_LEN ))
        return COMMU_ERR_OTFBUFF;

    //if data_pos + blob->len > blocksize_
    if( (data_pos + blob->len ) > blocksize_ )
    {
        unsigned int first_size = blocksize_ - data_pos;
        unsigned int second_size = blob->len - first_size;

        //copy first 
        memcpy(blob->data,block_ + data_pos,first_size);

        //copy left
        memcpy(blob->data + first_size,block_,second_size);
        memcpy( blob->extdata,block_ + second_size,blob->ext_len );

        *head_ = second_size + blob->ext_len;
    }

    //if blocksize_ >= data_pos + blob->len && < data_pos + blob->len + blob->ext_len
    else if( blocksize_ >=( data_pos + blob->len) && blocksize_ < data_pos + data_len )
    {
        unsigned int first_size = blocksize_ - data_pos -blob->len;
        unsigned int second_size = blob->ext_len - first_size;

        //copy data
        memcpy(blob->data,block_ + data_pos,blob->len);

        //copy first
        memcpy(blob->extdata,block_ + data_pos + blob->len,first_size);
        //copy second
        memcpy(((char *) blob->extdata ) + first_size,block_ ,second_size );

        *head_ = second_size;
    }
    else
    {	    
        //copy data
        memcpy(blob->data,block_ + data_pos,blob->len);
        memcpy(blob->extdata,block_ + data_pos + blob->len,blob->ext_len);

        *head_ = data_pos + data_len;
    }
    return 0;

}
/////////////////////////////////////////////////////////////////////////////////////////////
CShmProducer::CShmProducer():mq_(NULL)
{
}
CShmProducer::~CShmProducer()
{
    fini();
}
int CShmProducer::init(int shmkey, int shmsize)
{
    fini();

    mq_ = new CShmMQ;
    return mq_->init(shmkey, shmsize);
}
void CShmProducer::fini()
{
    if(mq_ != NULL)
    {
        delete mq_;
        mq_ = NULL;
    }
}

int CShmProducer::produce(const void * arg1, unsigned flow, unsigned char msgtype)
{
    int ret = 0;
    ret = mq_->enqueue(arg1, flow,msgtype);
    return ret;
}

/////////////////////////////////////////////////////////////////////////////////////////////
CShmProducerL::CShmProducerL()
{
#ifndef SLK_LINUX
    mutex_ = NULL;
    mfd_ = 0;
#else
    semid_ = -1;
#endif
}
CShmProducerL::~CShmProducerL()
{
    fini();	
}
int CShmProducerL::init(int shmkey, int shmsize)
{
    fini();

    int ret = CShmProducer::init(shmkey, shmsize);
    if(likely(!ret))
    {
#ifndef SLK_LINUX	
        //创建一个使用文件映射的共享线程锁
        char mfile[128] = {0};
        sprintf(mfile, "/tmp/mq_producer_%d.lock", shmkey);
        mfd_ = open(mfile, O_RDWR | O_CREAT, 0666); 
        if(likely(mfd_ > 0))
        {	
            ftruncate(mfd_, sizeof(pthread_mutex_t));
            mutex_ = (pthread_mutex_t*)mmap(NULL, sizeof(pthread_mutex_t), PROT_READ | PROT_WRITE, MAP_SHARED, mfd_, 0); 
            if(likely(mutex_ != MAP_FAILED))
            {		
                pthread_mutexattr_t attr;	
                pthread_mutexattr_init(&attr); 
                pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED); 
                pthread_mutex_init(mutex_, &attr); 
                pthread_mutexattr_destroy(&attr);
                return 0;
            }
            else
            {
                close(mfd_);
                return COMMU_ERR_FILEMAP;
            }
        }
        else
        {
            return COMMU_ERR_FILEOPEN;
        }
#else
        return sem_init(shmkey);
#endif		
    }
    else
    {
        return ret;
    }
}
void CShmProducerL::fini()
{
    CShmProducer::fini();
#ifndef SLK_LINUX		
    if(mutex_ != NULL && mfd_ > 0)
    {	
        pthread_mutex_destroy(mutex_); 
        munmap(mutex_, sizeof(pthread_mutex_t)); 
        close(mfd_);
    }
#else

#endif
}

int CShmProducerL::produce(const void * arg1, unsigned flow, unsigned char msgtype)
{
    int ret = 0;	
#ifndef SLK_LINUX
    if(likely(!(ret = pthread_mutex_lock(mutex_))))
    {	
        ret = CShmProducer::produce(arg1, flow,msgtype);
        pthread_mutex_unlock(mutex_);	
    }
#else
    if(likely(!(ret = sem_lock())))
    {
        ret = CShmProducer::produce(arg1, flow,msgtype);
        sem_unlock();
    }
#endif	
    return ret;
}

#ifdef SLK_LINUX
int CShmProducerL::sem_init(int key)
{
    semid_ = semget(key, 1, IPC_CREAT | 0666);
    if(likely(semid_ != -1))
    {
        union shmsemun arg;
        arg.val = 1;
        semctl(semid_, 0, SETVAL, arg);
    } 
    else 
    {
        return COMMU_ERR_SEMGET;
    }
    return 0;
}
int CShmProducerL::sem_lock()
{
    struct sembuf op;
    op.sem_num = 0;
    op.sem_flg = SEM_UNDO;
    op.sem_op = -1;

    if(unlikely(semop(semid_, &op, 1) == -1))
    {
        return COMMU_ERR_SEMLOCK;
    }
    return 0;
}
int CShmProducerL::sem_unlock()
{
    struct sembuf op;
    op.sem_num = 0;
    op.sem_flg = SEM_UNDO;
    op.sem_op = 1;
    while(semop(semid_, &op, 1) == -1) 
    {
        if(errno != EINTR)
            return COMMU_ERR_SEMUNLOCK;
    }
    return 0;
}
#endif
/////////////////////////////////////////////////////////////////////////////////////////////
CShmComsumer::CShmComsumer():mq_(NULL)
{
}
CShmComsumer::~CShmComsumer()
{
    fini();	
}
int CShmComsumer::init(int shmkey, int shmsize)
{
    fini();

    mq_ = new CShmMQ;
    return mq_->init(shmkey, shmsize);
}
void CShmComsumer::fini()
{
    if(mq_ != NULL)
    {
        delete mq_;
        mq_ = NULL;
    }
}

inline int CShmComsumer::comsume_i(void * arg1, unsigned & flow, unsigned char & msgtype)
{
    return mq_->dequeue(arg1,  flow,msgtype);
}

int CShmComsumer::comsume(void * arg1, unsigned int & flow, unsigned char & msgtype, bool block/* = false*/)
{
    int ret = 0;	
    ret = comsume_i(arg1,flow,msgtype);
    if(!ret || ret != COMMU_ERR_MQEMPTY)
    {
        return ret;
    }
    
    return COMMU_ERR_MQEMPTY;
}

/////////////////////////////////////////////////////////////////////////////////////////////
CShmComsumerL::CShmComsumerL()
{
#ifndef SLK_LINUX
    mutex_ = NULL;
    mfd_ = 0;
#else
    semid_ = -1;
#endif		
}
CShmComsumerL::~CShmComsumerL()
{
    fini();
}
int CShmComsumerL::init(int shmkey, int shmsize)
{
    fini();

    int ret = 0;
    ret = CShmComsumer::init(shmkey, shmsize);
    if(!ret)
    {
#ifndef SLK_LINUX	
        //创建一个使用文件映射的共享线程锁
        char mfile[128] = {0};
        sprintf(mfile, "/tmp/mq_comsumer_%d.lock", shmkey);
        mfd_ = open(mfile, O_RDWR | O_CREAT, 0666); 
        if(mfd_ > 0)
        {	
            ftruncate(mfd_, sizeof(pthread_mutex_t));
            mutex_ = (pthread_mutex_t*)mmap(NULL, sizeof(pthread_mutex_t), PROT_READ | PROT_WRITE, MAP_SHARED, mfd_, 0); 
            if(mutex_ != MAP_FAILED)
            {		
                pthread_mutexattr_t attr;	
                pthread_mutexattr_init(&attr); 
                pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED); 
                pthread_mutex_init(mutex_, &attr); 
                pthread_mutexattr_destroy(&attr);
                return 0;
            }
            else
            {
                close(mfd_);
                return COMMU_ERR_FILEMAP;
            }
        }
        else
        {
            return COMMU_ERR_FILEOPEN;
        }
#else
        return sem_init(shmkey);
#endif				
    }
    else
    {
        return ret;
    }
}
void CShmComsumerL::fini()
{
    CShmComsumer::fini();
#ifndef SLK_LINUX	
    if(mutex_ != NULL && mfd_ > 0)
    {	
        pthread_mutex_destroy(mutex_); 
        munmap(mutex_, sizeof(pthread_mutex_t)); 
        close(mfd_);
    }
#else

#endif			 
}

inline int CShmComsumerL::comsume_i(void * arg1, unsigned & flow, unsigned char & msgtype)
{
    int ret = 0;
#ifndef SLK_LINUX
    if(!(ret = pthread_mutex_lock(mutex_)))
    {	
        ret = mq_->dequeue(arg1, flow,msgtype);
        pthread_mutex_unlock(mutex_);
    }
#else
    if(!(ret = sem_lock()))
    {
        ret = mq_->dequeue(arg1, flow,msgtype);
        sem_unlock();
    }
#endif
    return ret;
}

#ifdef SLK_LINUX
int CShmComsumerL::sem_init(int key)
{
    semid_ = semget(key, 1, IPC_CREAT | 0666);
    if(semid_ != -1)
    {
        union shmsemun arg;
        arg.val = 1;
        semctl(semid_, 0, SETVAL, arg);
    } 
    else 
    {
        return COMMU_ERR_SEMGET;
    }
    return 0;
}
`	int CShmComsumerL::sem_lock()
{
    struct sembuf op;
    op.sem_num = 0;
    op.sem_flg = SEM_UNDO;
    op.sem_op = -1;

    if(semop(semid_, &op, 1) == -1)
    {
        return COMMU_ERR_SEMLOCK;
    }
    return 0;
}
int CShmComsumerL::sem_unlock()
{
    struct sembuf op;
    op.sem_num = 0;
    op.sem_flg = SEM_UNDO;
    op.sem_op = 1;
    while(semop(semid_, &op, 1) == -1) 
    {
        if(errno != EINTR)
            return COMMU_ERR_SEMUNLOCK;
    }
    return 0;
}
#endif

/////////////////////////////////////////////////////////////////////////////////////////////
CTShmCommu::CTShmCommu():producer_(NULL), comsumer_(NULL), locktype_(0), expiretime_(0), lastchecktime_(0)
{
    buff_blob_.len = 0;
    buff_blob_.data = NULL;
    buff_blob_.owner = NULL;
    buff_blob_.ext_type = EXT_TYPE_NONE;
    buff_blob_.ext_len = 0;
    buff_blob_.extdata = NULL;
}
CTShmCommu::~CTShmCommu()
{
    fini();
}
void CTShmCommu::fini()
{
    if(producer_ != NULL)
    {
        delete producer_;
        producer_ = NULL;
    }
    if(comsumer_ != NULL)
    {
        delete comsumer_;
        comsumer_ = NULL;
    }
    if(buff_blob_.data != NULL)
    {
        delete [] buff_blob_.data;
        buff_blob_.data = NULL;
    }

    if(buff_blob_.extdata != NULL )
    {
        delete [] (char *) buff_blob_.extdata;
        buff_blob_.extdata = NULL;
    }
}
void CTShmCommu::check_expire()
{
    int now = get_timebytask(NULL);
    if(now - lastchecktime_ > 60)
    {	
        std::map<unsigned, unsigned>::iterator it;
        std::map<unsigned, unsigned> newmap;
        unsigned deadline_time = get_timebytask(NULL) - expiretime_;
        for(it = expiremap_.begin(); it != expiremap_.end(); ++it)
        {
            //超时
            if(it->second < deadline_time)
            {
                //通知用户超时
                blob_type blob;
                blob.len = 0;
                blob.data = NULL;
                blob.ext_type = EXT_TYPE_TIMEOUT;
                blob.ext_len = 0;
                blob.extdata = NULL;
                if(func_list_[CB_TIMEOUT] != NULL)
                {
                    func_list_[CB_TIMEOUT](it->first, &buff_blob_, func_args_[CB_TIMEOUT]);	
                }
            }
            //未超时
            else
            {
                newmap.insert(std::pair<unsigned, unsigned>(it->first, it->second));
            }
        }

        if(newmap.size())
            expiremap_.swap(newmap);

        lastchecktime_ = now;
    }
}

int CTShmCommu::init(const void* config)
{
    fini();
    TShmCommuConf* conf = (TShmCommuConf*)config;
    assert(conf->expiretime_ >= 0 && conf->maxpkg_ >= 0 );

    int ret = 0;
    locktype_ = conf->locktype_;
    //创建生产者
    if(locktype_ & LOCK_TYPE_PRODUCER)
        producer_ = new CShmProducerL;
    else
        producer_ = new CShmProducer;

    //创建消费者	
    if(locktype_ & LOCK_TYPE_COMSUMER)
        comsumer_ = new CShmComsumerL;
    else
        comsumer_ = new CShmComsumer;


    //初始化其他变量	
    buff_size_ = conf->shmsize_comsumer_;
    buff_ext_size_ = MAX_BLOB_EXTDATA_LEN;//最大64K
    buff_blob_.len = buff_size_; //最大不会超过comsumer使用的共享内存的大小
    buff_blob_.data = new char[buff_size_];
    buff_blob_.owner = this;
    buff_blob_.ext_type = EXT_TYPE_NONE;
    buff_blob_.ext_len = buff_ext_size_;
    buff_blob_.extdata = new char[buff_ext_size_];

    expiretime_ = conf->expiretime_;
    
    expiremap_.clear();
    lastchecktime_ = get_timebytask(NULL);
    
#ifdef LOAD_CHECK_ENABLE
    myload_.maxload(conf->maxpkg_);  //设置最大包量
#endif

    ret = producer_->init(conf->shmkey_producer_, conf->shmsize_producer_);
    producer_->mq_->msg_timeout_ = conf->msg_timeout_;
    if(!ret)
    {
        ret = comsumer_->init(conf->shmkey_comsumer_, conf->shmsize_comsumer_);
        comsumer_->mq_->msg_timeout_ = conf->msg_timeout_;
    }
    return ret;
    
}
int CTShmCommu::poll(bool block)
{
    int ret = 0;
    unsigned flow = 0;
    unsigned char msgtype = EXT_TYPE_NONE;
    do
    {
        ret = comsumer_->comsume(&buff_blob_, flow,msgtype, block);
        if(!ret)
        {
#ifdef LOAD_CHECK_ENABLE
            if(unlikely(myload_.check_load()))
            {
                if(func_list_[CB_OVERLOAD] != NULL)
                {	
                    blob_type blob;
                    blob.len == 0;
                    blob.data = (char*)COMMU_ERR_OVERLOAD_PKG;
                    blob.ext_type = EXT_TYPE_OVERLOAD;
                    blob.ext_len = 0;
                    blob.extdata = NULL;
                    func_list_[CB_OVERLOAD](0, &blob, func_args_[CB_OVERLOAD]);
                }
                break;
                //printf("%s:%d:%s overload\n", __FILE__, __LINE__, __FUNCTION__);
            }
            myload_.grow_load(1);
#endif			

            switch(msgtype)
            {
                case EXT_TYPE_CONNECTED:
                {
                    if(func_list_[CB_CONNECTED] != NULL )
                    {	
                        func_list_[CB_CONNECTED](flow,&buff_blob_,func_args_[CB_CONNECTED]);
                    }
                }
                break;

                case EXT_TYPE_OVERLOAD:
                {
                    if(func_list_[CB_OVERLOAD] != NULL )
                    {
                        func_list_[CB_OVERLOAD](flow,&buff_blob_,func_args_[CB_OVERLOAD]);
                    }
                }
                break;

                case EXT_TYPE_TIMEOUT:
                {
                    if(func_list_[CB_TIMEOUT] != NULL )
                    {
                        func_list_[CB_TIMEOUT](flow,&buff_blob_,func_args_[CB_TIMEOUT]);
                    }
                }
                break;

                case EXT_TYPE_DISCONNECT:
                {
                    if(func_list_[CB_DISCONNECT] != NULL )
                    {
                        func_list_[CB_DISCONNECT](flow,&buff_blob_,func_args_[CB_DISCONNECT]);
                    }
                }
                break;

                default:
                {
                    func_list_[CB_RECVDATA](flow, &buff_blob_, func_args_[CB_RECVDATA]);
                }
                break;
            }

        }
	
		
#ifdef _SPP_PROXY
        else       //无数据了
#endif
            break;

    }while(!block);

    if(expiretime_)
    {
        if(!ret)
        {
            expiremap_.erase(flow);
        }
        check_expire();
    }
    return ret;
}

int CTShmCommu::sendto(unsigned flow, void* arg1, void* arg2)
{
    int ret = 0;
    blob_type* blob = (blob_type*)arg1;

    unsigned char msg_type = blob->ext_type;
    if(blob->extdata != NULL && blob->ext_len > 0 )
    {
        msg_type = blob->ext_type;
    }

    ret = producer_->produce(arg1,  flow,msg_type);//发送到消息队列中


    if(expiretime_ && !ret)	//需要监控超时	
        expiremap_.insert(std::pair<unsigned, unsigned>(flow, get_timebytask(NULL)));
    
    if(!ret)
    {
        if(func_list_[CB_SENDDATA] != NULL)
        {
            func_list_[CB_SENDDATA](flow, blob, func_args_[CB_SENDDATA]);
        }
    }
    else
    {
        if(func_list_[CB_SENDERROR] != NULL)
        {
            func_list_[CB_SENDERROR](flow, blob, func_args_[CB_SENDERROR]);
        }
    }
    return ret;
}
int CTShmCommu::ctrl(unsigned flow, ctrl_type type, void* arg1, void* arg2)
{
    switch(type)
    {
        case CT_STAT:
        {
            TMQStat mq;
            char* buf = (char*)arg1;
            int* len = (int*)arg2;
            time_t now = get_timebytask(NULL);
            struct tm tmm;
            localtime_r(&now, &tmm); 
            *len += sprintf(buf + *len, "TShmCommu[%-5d],%04d-%02d-%02d %02d:%02d:%02d\n", (int)syscall(__NR_gettid), 
                tmm.tm_year + 1900, tmm.tm_mon + 1, tmm.tm_mday, tmm.tm_hour, tmm.tm_min, tmm.tm_sec);
            *len += sprintf(buf + *len, "%-20s%-10s|%-10s|%-10s|%-10s|%-10s|%-10s\n", "Type", "ShmKey", "ShmID", 
                "ShmSize", "Total", "Used", "Free"); 

            producer_->getstat(mq);
            *len += sprintf(buf + *len, "%-20s%-10x|%-10d|%-10d|%-10d|%-10d|%-10d\n", "Producer", mq.shmkey_, 
                mq.shmid_, mq.shmsize_, mq.totallen_, mq.usedlen_, mq.freelen_);

            comsumer_->getstat(mq);
            *len += sprintf(buf + *len, "%-20s%-10x|%-10d|%-10d|%-10d|%-10d|%-10d\n", "Comsumer", mq.shmkey_, 
                mq.shmid_, mq.shmsize_, mq.totallen_, mq.usedlen_, mq.freelen_);
        }
        break;

	case CT_GET_CONN_EXT_INFO:
	case CT_DISCONNECT:
	case CT_CLOSE:
	case CT_LOAD:
	default:
	{
	}
	break;
    }
    return 0;
}

int CTShmCommu::get_msg_count()
{
    return 0;
}

