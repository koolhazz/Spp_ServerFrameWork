/********************************
*文件名：shm_array.h
*功能：基于共享内存的array.
*作者：钟何明
*时间：2010.03.5
**********************************/
#include "SVShm.hpp"
#include "HashMap.hpp"
#include "lock.hpp"
#include "lock.h"
#include "hexdump.h"
#include "asm/atomic.h"

#ifndef _COMM_SHM_ARRAY_H_
#define _COMM_SHM_ARRAY_H_

using namespace comm::commu;
using namespace comm::lock;

namespace comm
{
namespace array
{

template<typename data_t>
class CShmArray
{
public:
    typedef data_t node_t;
    typedef struct data_item_t
    {
        unsigned char flag;
        node_t data;
    };
    typedef struct tagArrayHead
    {
        size_t m_ArraySize;//数组的长度
        data_item_t m_Nodes[1];//节点数组            
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
                m_pHead->m_ArraySize = (shm_size - sizeof(ShmHead))/sizeof(data_item_t)+1;
                //printf("#shm_size = %d, HeadLen=%d, nodesize=%d\n", shm_size, 
                     //sizeof(ShmHead),sizeof(node_t));               
                //printf("#New Shm Array. Total = %d\n", m_pHead->m_ArraySize);
            }
            else
            {
                //printf("Not Init:####Total = %d\n", m_pHead->m_ArraySize);
            }
            return 0;
    }


    /*
    *功能:取Array 的长度
    *返回值: Array长度
    */    
    size_t Length()
    {
        CLockGuard g(&m_Lock);
        return m_pHead->m_ArraySize;
    }
   

    /*
    *功能：设置idx元素的值
	*IN idx:数组的下标
    *IN v:待设置的元素
    *返回值：0成功，-1:overflow
    */
    int SetValue(size_t idx,const node_t &v)
    {
        CLockGuard g(&m_Lock);
        if(idx >= m_pHead->m_ArraySize)
        {
            return -1;
        }
   
        m_pHead->m_Nodes[idx].flag = 1;
        m_pHead->m_Nodes[idx].data= v;
        return 0;
    }

    /*
    *功能：取idx元素的值
	*IN idx:数组的下标
    *OUT v:待获取的元素
    *返回值：0成功，-1:overflow,-2:not init
    */
    int GetValue(size_t idx,node_t &v)
    {
        CLockGuard g(&m_Lock);
        if(idx >= m_pHead->m_ArraySize)
        {
            return -1;
        }

        if(m_pHead->m_Nodes[idx].flag != 1 )
            return -2;
            
        v = m_pHead->m_Nodes[idx].data;      
        return 0;
    }

    /*
    *功能:清空idx元素
    *IN idx:数组的下标
    *返回值：0成功，-1:overflow
    */
    int Delete(size_t idx)
    {
        CLockGuard g(&m_Lock);
        if(idx >= m_pHead->m_ArraySize)
        {
            return -1;
        }        
        
        m_pHead->m_Nodes[idx].flag = 0;

        return 0;
    }

private:

    CSVShm m_Shm;
    comm::lock::CSemLock m_Lock;
    ShmHead *m_pHead;

};


//========================ARRAY WITH RWLOCK=====================

#ifdef __x86_64__
#define ATOMICVAL              atomic64_t
#define ATOMICVAL_INC(val)    atomic64_inc(&(val))
#define ATOMICVAL_DEC(val)    atomic64_dec(&(val))
#define ATOMICVAL_ADD(val, lv)    atomic64_add(lv, &(val))
#define ATOMICVAL_SET(val, lv)    atomic64_set(&(val), lv)
#define ATOMICVAL_READ(val)	    atomic64_read(&(val))
#else
#define ATOMICVAL                 atomic_t
#define ATOMICVAL_INC(val)    atomic_inc(&(val))
#define ATOMICVAL_DEC(val)    atomic_dec(&(val))
#define ATOMICVAL_ADD(val, lv)    atomic_add(lv, &(val))
#define ATOMICVAL_SET(val, lv)    atomic_set(&(val), lv)
#define ATOMICVAL_READ(val)	    atomic_read(&(val))
#endif

struct RwlockHelper
{
	RwlockHelper(pthread_rwlock_t* rwl=NULL, bool write_locker=false):rwlock(rwl)
	{
		if (rwlock)
		{
			if (write_locker)
			{
				pthread_rwlock_wrlock(rwlock);
			}
			else
			{
				pthread_rwlock_rdlock(rwlock);
			}
		}
	}
	RwlockHelper(const RwlockHelper& rh)
	{
		RwlockHelper* r = (RwlockHelper*)&rh;
		rwlock = r->rwlock;
		r->rwlock = NULL;
	}
	~RwlockHelper()
	{
		if (rwlock)
		{
			pthread_rwlock_unlock(rwlock);
		}
	}
	void manual_lock(pthread_rwlock_t* rwl, bool write_locker=false)
	{
		manual_unlock();

		if (rwl)
		{
			if (write_locker)
			{
				pthread_rwlock_wrlock(rwl);
			}
			else
			{
				pthread_rwlock_rdlock(rwl);
			}
		}
		rwlock = rwl;
	}
	void manual_unlock()
	{
		if (rwlock)
		{
			pthread_rwlock_unlock(rwlock);
		}
		rwlock = NULL;
	}
	bool isvalid() const
	{
		return rwlock != NULL;
	}

private:
	RwlockHelper& operator =(const RwlockHelper&);

private:
	pthread_rwlock_t* rwlock;
};


template<typename data_t>
class CShmRWArray
{
public:
    typedef data_t node_t;
    typedef struct data_item_t
    {
        unsigned char flag;
        node_t data;
    };
    
    typedef struct tagArrayHead
    {
        ATOMICVAL  m_ArraySize;//数组的长度
        data_item_t m_Nodes[1];//节点数组 
    }ShmHead;

public:


    /*
    *功能:初始化共享内存，信号量
    *参数说明: 
    *IN sem_key:信号量的key
    *IN shm_key:共享内存的key
    *IN shm_size:共享内存的大小
    *IN rwlock_num:读写锁个数
    *返回值: 0成功，非0失败
    */    
    int Init(int sem_key, int shm_key, size_t shm_size,size_t rwlock_num = 32)
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

            m_rwlock_num = (ATOMICVAL * ) pShm;
            m_rwlocks = (pthread_rwlock_t * ) (m_rwlock_num + 1);

            if(tmp_init == 0 && ATOMICVAL_READ(*m_rwlock_num) != (int) rwlock_num )
            {
                //printf("rwlock_num change to %d,force new shm\n",rwlock_num);
                tmp_init = -1;//强制为新创建的shm
            }
  
            m_pHead = (ShmHead *)(m_rwlocks + rwlock_num);

            if( (char *) (m_pHead + 1 ) >  pShm + shm_size )
            {
                //printf("shm array out of bound.shm begin:0x%08x,size=%d,pos=0x%08x\n",pShm,shm_size,m_pHead + 1);
                return -3;
            }

            
            if(tmp_init != 0)
            {

                //共享内存为新创建
                ATOMICVAL_SET(*m_rwlock_num,rwlock_num);

                pthread_rwlockattr_t rwlock_attr;
                pthread_rwlockattr_init(&rwlock_attr);
                pthread_rwlockattr_setpshared(&rwlock_attr, PTHREAD_PROCESS_SHARED);		

                    
                for (size_t i = 0; i < rwlock_num; ++i)
                {
                    pthread_rwlock_init(m_rwlocks+i, &rwlock_attr);
                }

                pthread_rwlockattr_destroy(&rwlock_attr);
		
                size_t array_size = (shm_size - sizeof(ShmHead) - sizeof(pthread_rwlock_t) * rwlock_num - sizeof(ATOMICVAL)  )/sizeof(data_item_t)+1;
                ATOMICVAL_SET(m_pHead->m_ArraySize,array_size);
                //printf("#shm_size = %d, HeadLen=%d, nodesize=%d\n", shm_size, 
                     //sizeof(ShmHead),sizeof(node_t));               
                //printf("#New Shm Array. Total = %d\n", m_pHead->m_ArraySize);
            }
            else
            {
                //printf("Not Init:####Total = %d\n", m_pHead->m_ArraySize);
            }
            return 0;
    }


    /*
    *功能:取Array 的长度
    *返回值: Array长度
    */    
    size_t Length()
    {        
        return ATOMICVAL_READ(m_pHead->m_ArraySize);
    }
   

    /*
    *功能：设置idx元素的值
	*IN idx:数组的下标
    *IN v:待设置的元素
    *返回值：0成功，-1:overflow
    */
    int SetValue(size_t idx,const node_t &v)
    {
        if(idx >= Length() )
        {
            return -1;
        }
        
        int rwl_index  = idx % ATOMICVAL_READ(*m_rwlock_num);
        RwlockHelper rwlock(m_rwlocks + rwl_index,true);        
        m_pHead->m_Nodes[idx].flag = 1;
        m_pHead->m_Nodes[idx].data= v;
        return 0;
    }

    /*
    *功能：取idx元素的值
	*IN idx:数组的下标
    *OUT v:待获取的元素
    *返回值：0成功，-1:overflow,-2:not init
    */
    int GetValue(size_t idx,node_t &v)
    {
        if(idx >= Length() )
        {
            return -1;
        } 
        
        int rwl_index  = idx % ATOMICVAL_READ(*m_rwlock_num);
        RwlockHelper rwlock(m_rwlocks + rwl_index,true);
        if(m_pHead->m_Nodes[idx].flag != 1 )
            return -2;
            
        v = m_pHead->m_Nodes[idx].data;
        return 0;
    }

    /*
    *功能:清空idx元素
    *IN idx:数组的下标
    *返回值：0成功，-1:overflow
    */
    int Delete(size_t idx)
    {
        if(idx >= Length() )
        {
            return -1;
        } 
        
        int rwl_index  = idx % ATOMICVAL_READ(*m_rwlock_num);
        RwlockHelper rwlock(m_rwlocks + rwl_index,true);
        m_pHead->m_Nodes[idx].flag = 0;
        return 0;
    }

private:

    CSVShm m_Shm;
    comm::lock::CSemLock m_Lock;
    ShmHead *m_pHead;
    
    ATOMICVAL * m_rwlock_num;
    pthread_rwlock_t * m_rwlocks;//读写锁的地址

};

}
}


#if 0
//example

#include "shm_array.h"
#include <iostream.h>


typedef struct tagItem
{
	unsigned int uin;
	unsigned char sex;
    char szName[256];	
}TDataItem;

using namespace comm::array;

//typedef CShmArray<TDataItem> ShmArray;
typedef CShmRWArray<TDataItem> ShmArray;
int main()
{
	ShmArray UserList;
	const unsigned int SHM_SIZE = 1024000;
	int r = UserList.Init(0x01234567,0x01234567,SHM_SIZE);
	printf("Init shm array,r=%d\n",r);

	size_t num = UserList.Length();
	for(size_t i = 0;i<num;++i)
	{
		TDataItem Item;
		Item.uin=i;
		Item.sex = i % 3;
		sprintf(Item.szName,"Name-%d\n",i);
		UserList.SetValue(i,Item);
	}
	while(1)
	{
		size_t idx = rand() * 10000 % num;
		TDataItem Item;
		UserList.GetValue(idx,Item);
		printf("id=%d,sex=%d,Name=%s\n",Item.uin,Item.sex,Item.szName);

		//usleep(1000);
		sleep(1);

	}


	return 0;
}
#endif

#endif

