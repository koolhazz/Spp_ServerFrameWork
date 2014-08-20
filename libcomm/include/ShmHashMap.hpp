#ifndef _SHM_HASH_MAP_H_
#define _SHM_HASH_MAP_H_

#include "SVShm.hpp"
#include "HashMap.hpp"
#include "lock.hpp"
#include "lock.h"

namespace comm
{
namespace commu
{


template<
    typename key_t ,
    typename node_t = HashNode<key_t>,
    typename HashFunc = __gnu_cxx::hash<key_t>,
    typename hash_map = CHashMap<key_t,node_t,HashFunc> 
    > 
class CShmHashMap
{
public:
    typedef CShmHashMap<key_t, node_t, HashFunc, hash_map> _Self;
    class vistor
    {
        public:
            vistor(_Self &obj):m_Context(obj), hash_map_(obj.hash_map_)
            {
                m_Lock.sem_attach(obj.sem_id_);
                while(1)
                {
                    m_LockRet = m_Lock.Lock();
                    if( 0 == m_LockRet)
                    {
                        break;
                    }
                    if(errno == EINTR)
                    {
                        continue;
                    }
                    break;
                }                
            }
            
            ~vistor()
            {
                if(0 == m_LockRet)
                {
                    while(1)
                    {
                        m_LockRet = m_Lock.UnLock();
                        if( 0 == m_LockRet)
                        {
                            break;
                        }
                        if(errno == EINTR)
                        {
                            continue;
                        }
                        break;
                    }
                }
            }

            //已用的节点数
            int used_size()const
            {
                return hash_map_.used_node_num();
            }

            //总的节点数
            int total_size()const
            {
                return hash_map_.get_node_total();
            }


        	node_t * begin()
        	{
        		return hash_map_.get_add_list_head() ;
        	}

        	node_t * end()
        	{
        		return hash_map_.get_add_list_tail();
        	}
        	
        	node_t * next(node_t * node)
        	{
        		return hash_map_.get_add_list_next(node);
        	}

        	node_t * prev(node_t * node)
        	{
        		return hash_map_.get_add_list_prev(node);
        	}
        	
        	int GetData(node_t * node,char * data,int * data_len)
        	{
        		return hash_map_.merge_node_data(node,data,data_len);
        	}

        	int Erase(node_t * node)
        	{
        		if(node == NULL )
        			return -1;
        		return (hash_map_.delete_node(node,0,0));
        	}

        	node_t *  UpdateData(node_t * node,char * data,int data_len)
        	{
        		if(node == NULL )
        			return NULL;
        		return (hash_map_.update_node(node, data, data_len,0,0)); 
        	}
            
        private:
               vistor(const vistor &v);
               vistor& operator= (const vistor &v);
        private:
               _Self &m_Context;    
              	hash_map &hash_map_;//HASH 表

               comm::lock::CSemLock m_Lock; 
               int m_LockRet;
    };

    friend class vistor;
public:
 
	CShmHashMap():sem_id_(-1){}
	~CShmHashMap(){}

	//初始化方法
	//hash_info:配置参数
	//返回值
	//0:成功,-1失败
	int Init(const THashInfo &hash_info)
	{
		size_t pool_size;
		int tmp_init, ret,init;

		pool_size = hash_map_.get_total_pool_size(hash_info.hash_para_.node_total_, hash_info.hash_para_.bucket_size_,
			hash_info.hash_para_.chunk_total_, hash_info.hash_para_.chunk_size_);

		//先开sem,锁住后在计算shm的
		sem_id_ = semget(hash_info.sem_key_, 1, IPC_CREAT | IPC_EXCL | 0666);	
		if (sem_id_ < 0) 
	        { 
			if (errno != EEXIST) 
	                  {
				return -1;
			}
			sem_id_ = semget(hash_info.sem_key_, 1, 0666);
	        
			if( sem_id_ < 0 ) 
	                  {
				return -1;
			}
		} 
	        else 
	        {
			// init sem
			unsigned short* init_array = new unsigned short[1];
			init_array[0] = 1;
			int ret = semctl(sem_id_, 0, SETALL, init_array);
			delete [] init_array;
			if(ret < 0) 
	                  {
				return -1;
			}
		}

		CAutoLock l(sem_id_);

		ret = shm_.force_open_and_attach(hash_info.shm_key_, pool_size, tmp_init);
		if(ret < 0)
		{
			return -1;
		}

		if(tmp_init)
		{
			//共享内存为新创建，强制修改为初始化模式
			init = tmp_init;
		}

		ret = hash_map_.open((char *)shm_.get_segment_ptr(),init, hash_info.hash_para_.node_total_,
			hash_info.hash_para_.bucket_size_, hash_info.hash_para_.chunk_total_, hash_info.hash_para_.chunk_size_);
		if(ret < 0)
		{
			return -1;
		}
		else
		{
		
		}
		return 0;
	}

	//查找方法
	//返回值
	//true:成功,false:失败
	bool Find(const key_t & key)
	{
		CAutoLock l(sem_id_);
		return (hash_map_.find_node(key) != NULL);
	}

	//查找方法
	//[out] buf:接收数据缓冲
	//[in,out] 接收缓冲区长度
	//返回值
	//0:成功，-1失败
	//说明:若buf长度不够，则通过buf_len返回需要的buf大小
	int Find(const key_t & key,char * buf,int * buf_len)
	{
		CAutoLock l(sem_id_);
		node_t * node = hash_map_.find_node(key);
		if(node != NULL )
		{
			return hash_map_.merge_node_data(node,buf,buf_len);		
		}
		
		return -1;
	}

	//插入方法
	//若key存在则返回失败
	int Insert(const key_t & key,void * data,int data_len)
	{
		CAutoLock l(sem_id_);
		return ( hash_map_.insert_node(key,data,data_len)  == NULL ? -1:0 );
	}

	//insert or replace
	int SetValue(const key_t & key,void * data,int data_len)
	{
		CAutoLock l(sem_id_);
		return (hash_map_.replace_node(key,data,data_len,0,0) == NULL ? -1:0 );
	}

	//删除方法
	int Delete(const key_t & key)
	{
		CAutoLock l(sem_id_);
		node_t * node = hash_map_.find_node(key);
		if( node != NULL )
			return (hash_map_.delete_node(node,0,0));
		return -1;
	}


	//获取HASH表大小
	int GetSize() const{return hash_map_.get_node_total();}

	//获取HASH表中元素个数
	int GetCount() const {return hash_map_.used_node_num();}

private:
	CSVShm shm_;//共享内存
	hash_map hash_map_;//HASH 表
	int sem_id_;//锁的ID
	
};


}
}

#endif
