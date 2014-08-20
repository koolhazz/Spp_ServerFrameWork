/********************************
*文件名：shm_queue.h
*功能：基于共享内存的queue.
*作者：钟何明
*时间：2010.03.10
**********************************/
#include "SVShm.hpp"
#include "HashMap.hpp"
#include "lock.hpp"
#include "lock.h"
#include "hexdump.h"
#include "asm/atomic.h"


#ifndef _COMM_SHM_QUEUE_H_
#define _COMM_SHM_QUEUE_H_

using namespace comm::commu;
using namespace comm::lock;

namespace comm 
{
namespace queue 
{

template<typename data_t>
class CShmQueue
{
public:
    typedef data_t node_t;
    typedef struct tagShmHead
    {
    	 unsigned int m_nFront;//队首
    	 unsigned int m_nRear;//队尾
        unsigned int m_nSize;//数组的长度
        node_t m_Nodes[1];//节点数组            
    }ShmHead;

    /*
    *功能:初始化共享内存，信号量
    *参数说明: 
    *IN sem_key:信号量的key
    *IN shm_key:共享内存的key
    *IN shm_size:共享内存的大小
    *返回值: 0成功，非0失败
    */    
    int Init(int sem_key, int shm_key, size_t shm_size)
    {
            int r = m_Lock.sem_init(sem_key);
            if(r != 0)
            {
                return -1;
            }

            int tmp_init = 0;
            comm::lock::CLockGuard g(&m_Lock);
            r = m_Shm.force_open_and_attach(shm_key, shm_size, tmp_init);
            if(r != 0)
            {
                return -2;
            }
            //tmp_init 不为0代表是新创建的shm.
            char * pShm = (char *)(m_Shm.get_segment_ptr());  
            m_pHead = (ShmHead *)(pShm);

            if(tmp_init != 0)
            {
		  //共享内存为新创建
		  m_pHead->m_nFront = 0;
		  m_pHead->m_nRear = 0;
                m_pHead->m_nSize = (shm_size - sizeof(ShmHead))/sizeof(node_t)+1;                
                //printf("Init Shm:####Total = %u, front=%u,rear=%u\n", m_pHead->m_nSize, m_pHead->m_nFront,m_pHead->m_nRear);
            }
            else
            {
                //printf("Not Init:####Total = %u, front=%u,rear=%u\n", m_pHead->m_nSize, m_pHead->m_nFront,m_pHead->m_nRear);
            }

            assert(m_pHead->m_nSize != 0 );
            
            return 0;
    
    }

    /*
    *功能:取QUEUE中元素的个数
    *返回值: QUEUE中元素的个数
    */    
    unsigned int  Size()
    {
        CLockGuard g(&m_Lock);
        return (m_pHead->m_nRear  + m_pHead->m_nSize - m_pHead->m_nFront) % m_pHead->m_nSize;
    }

    /*
    *功能:取QUEUE的容量
    *返回值:QUEUE的容量
    */
    unsigned int Length()
    {
        CLockGuard g(&m_Lock);
        return m_pHead->m_nSize;
    }

    /*
    *功能:QUEUE是否为满
    *返回值: true空，false不空
    */    
    bool Full()
    {
        CLockGuard g(&m_Lock);
        return IsFull();
    }

    
    /*
    *功能:QUEUE是否为空
    *返回值: true空，false不空
    */    
    bool Empty()
    {
        CLockGuard g(&m_Lock);
        return IsEmpty();
    }

    

    /*
    *功能：入QUEUE.
    *IN v:待入QUEUE的元素
    *返回值：0成功，-1:QUEUE满
    */
    int Push(const node_t &v)
    {
        CLockGuard g(&m_Lock);
        if( IsFull() )
        {
            return -1;
        }
        
        m_pHead->m_Nodes[m_pHead->m_nRear] = v;
        m_pHead->m_nRear = (m_pHead->m_nRear + 1) % m_pHead->m_nSize;
        return 0;
    }

    /*
    *功能：出QUEUE.
    *OUT v:出QUEUE的元素
    *返回值：0成功，-1:QUEUE空
    */
    int Pop(node_t &v)
    {
        CLockGuard g(&m_Lock);
        if(IsEmpty() )
        {
            return -1;
        }
        
        v = m_pHead->m_Nodes[m_pHead->m_nFront];
        m_pHead->m_nFront = (m_pHead->m_nFront + 1) % m_pHead->m_nSize;
        return 0;
    }

    /*
    *功能:清空QUEUE元素
    *返回值:无
    */
    void  Clean()
    {
        CLockGuard g(&m_Lock);
        m_pHead->m_nFront = 0;
	  m_pHead->m_nRear = 0;	  
    }


protected:

    bool IsFull()
    {
        if(m_pHead->m_nFront == (m_pHead->m_nRear + 1) % m_pHead->m_nSize )
        {
            return true;
        }
        else
        {
            return false;
        }
    }

    bool IsEmpty()
    {
        return (m_pHead->m_nFront == m_pHead->m_nRear);
    }


private:

    CSVShm m_Shm;
    comm::lock::CSemLock m_Lock;
    ShmHead *m_pHead;

};


}
}

#if 0
#include <iostream>
#include "shm_queue.h"

typedef struct tagMsgInfo
{
	unsigned int msg_type;
	char buf[256];
	//...
}MsgInfo;

using namespace std;
using namespace comm::queue;

typedef CShmQueue<MsgInfo> ShmQueue;
int main(int argc,char ** argv)
{

	if(argc != 2)
	{
	    printf("Usage:%s type\n",argv[0]);
	    printf("type:1,push queue.\t2,pop queue.\n");
            return 0;
	}
    
	int mode = atoi(argv[1]);

	printf("mode=%d\n",mode);
    
	ShmQueue MsgQueue;
	const unsigned int SHM_SIZE = 1024000;
	int r = MsgQueue.Init(0x01234567,0x01234567,SHM_SIZE);
	printf("Init shm queue,r=%d\n",r);

        if(mode == 1)
        {
            int i = 0;
            MsgInfo Item;
        	while(1)
        	{        		
        		Item.msg_type=++i % 5;
        		sprintf(Item.buf,"Item-%d\n",i);
        		r = MsgQueue.Push(Item);
        		printf("push queue,ret = %d,cur Shm Queue size=%d\n",r,MsgQueue.Size());
        		sleep(1);
        	}

        	
	}
	else if ( mode == 2)
	{
	    MsgInfo Item;
	    while(1)
            {
                
                r = MsgQueue.Pop(Item);
                printf("Pop queue,r=%d,msg_type=%u,buf=%s\n",r,Item.msg_type,Item.buf);
                sleep(1);
            }
	}
	else
	{
	    printf("Usage:%s type\n",argv[0]);
	    printf("type:1,push queue.\t2,pop queue.\n");
	}
	return 0;
}

#endif


#endif



