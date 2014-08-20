
/********************************
*文件名：shm_stack.h
*功能：基于共享内存的stack.
*作者：张荐林
*时间：2009.06.10
**********************************/
#include "SVShm.hpp"
#include "HashMap.hpp"
#include "lock.hpp"
#include "lock.h"
#include "hexdump.h"

#ifndef _COMM_SHM_STACK_H_
#define _COMM_SHM_STACK_H_

using namespace comm::commu;
using namespace comm::lock;

namespace comm
{
namespace stack
{


template<typename data_t>
class CShmStack
{
public:

    typedef data_t node_t;

    typedef struct tagShmHead
    {
            size_t m_TotalItemNum;
            size_t m_CurItemNum;
            node_t m_Nodes[1];
    }ShmHead;


public:
 
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
            char *pShm = (char *)(m_Shm.get_segment_ptr());
  
            m_pHead = (ShmHead *)(pShm);
            if(tmp_init != 0)
            {
		//共享内存为新创建

                m_pHead->m_TotalItemNum = (shm_size - sizeof(ShmHead))/sizeof(node_t)+1;
                //printf("#shm_size = %d, HeadLen=%d, nodesize=%d\n", shm_size, 
                     //sizeof(ShmHead),sizeof(node_t));
                m_pHead->m_CurItemNum = 0;
                //printf("#New Shm. Total = %d, cur=%d\n", m_pHead->m_TotalItemNum, m_pHead->m_CurItemNum);
            }
            else
            {
                //printf("Not Init:####Total = %d, cur=%d\n", m_pHead->m_TotalItemNum, m_pHead->m_CurItemNum);
            }
            return 0;
    }

    /*
    *功能:取stack中元素的个数
    *返回值: stack中元素的个数
    */    
    size_t Size()
    {
        CLockGuard g(&m_Lock);
        return m_pHead->m_CurItemNum;
    }
    
    /*
    *功能:stack是否为空
    *返回值: true空，false不空
    */    
    bool Empty()
    {
        return (Size() == 0);
    }

    /*
    *功能：入stack.
    *IN v:待入stack的元素
    *返回值：0成功，-1:stack满
    */
    int Push(const node_t &v)
    {
        CLockGuard g(&m_Lock);
        if(m_pHead->m_CurItemNum >= m_pHead->m_TotalItemNum)
        {
            return -1;
        }        
        m_pHead->m_Nodes[m_pHead->m_CurItemNum] = v;
        ++m_pHead->m_CurItemNum;
        return 0;
    }

    /*
    *功能：出stack.
    *OUT v:出stack的元素
    *返回值：0成功，-1:stack空
    */
    int Pop(node_t &v)
    {
        CLockGuard g(&m_Lock);
        if(m_pHead->m_CurItemNum == 0)
        {
            return -1;
        }        
        v = m_pHead->m_Nodes[m_pHead->m_CurItemNum-1];
        --m_pHead->m_CurItemNum;
        return 0;
    }

private:
    CSVShm m_Shm;
    comm::lock::CSemLock m_Lock;
    
    ShmHead *m_pHead;
    node_t *m_pNodes;

};

}
}

#endif

