
#ifndef _REPLY_HASH_MAP_
#define  _REPLY_HASH_MAP_

#include "ChunkAlloc.hpp"
#include <ext/hash_map>
#include <functional>
//using namespace __gnu_cxx;      // hash map is int __gnu_cxx namespace

namespace comm
{
namespace commu
{


typedef enum tagENodeFlag
{
    NODE_FLAG_UNCHG = 0x00,
    NODE_FLAG_DIRTY = 0x01,
}ENodeFlag;


typedef struct tagTHashPara
{
    int bucket_size_; //HASH桶数
    int node_total_; //节点数
    int chunk_total_; //CHUNK分片数
    int chunk_size_; //CHUNK分片大小
}THashPara; 

typedef struct tagTHashInfo
{
    THashPara hash_para_;
    int shm_key_; //shm key
    int sem_key_; //sem key
}THashInfo; 


#pragma pack(1)

template<typename key_t>
struct HashNode
{
    key_t   key_;                       //索引
    int chunk_len_;                 //CHUNK中的数据长度
    BC_MEM_HANDLER chunk_head_;   //CHUNK 句柄
    BC_MEM_HANDLER node_prev_;    //节点链表前指针
    BC_MEM_HANDLER node_next_;    //节点链表后指针
    BC_MEM_HANDLER add_prev_;     //附加链表前指针
    BC_MEM_HANDLER add_next_;     //附加链表后指针
    int add_info_1_;                //最后访问时间
    int add_info_2_;		//访问次数
    int flag_;//节点标记
};



typedef struct tagTHashMap
{	
    int node_total_;                //节点总数
    int bucket_size_;               //HASH桶的大小
    int used_node_num_;             //使用的节点数
    int used_bucket_num_;           //HASH桶使用数
    BC_MEM_HANDLER add_head_;     //附加链表头指针
    BC_MEM_HANDLER add_tail_;     //附加链表尾指针
    BC_MEM_HANDLER free_list_;    //空间节点链表头指针
    BC_MEM_HANDLER bucket[1];     //HASH桶
}THashMap;

#pragma pack()

template<
    typename key_t ,
    typename node_t = HashNode<key_t>,
    typename HashFunc = __gnu_cxx::hash<key_t>
    > 
class CHashMap
{
public:
    enum HASH_MAP_ERROR
    {
        HASH_MAP_ERROR_BASE = -1000,    
        HASH_MAP_ERROR_INVALID_PARAM = HASH_MAP_ERROR_BASE -1,    //非法参数
        HASH_MAP_ERROR_NODE_NOT_EXIST = HASH_MAP_ERROR_BASE -2,    //节点不存在
        HASH_MAP_ERROR_NODE_HAVE_EXIST = HASH_MAP_ERROR_BASE -3,    //节点已经存在
        HASH_MAP_ERROR_NO_FREE_NODE = HASH_MAP_ERROR_BASE -4,    //没有空闲节点
    };

public:
    CHashMap();
    ~CHashMap();    

    //初始化 HASH_MAP 内存块
    int open(char* pool, bool init, int node_total, int bucket_size, int n_chunks, int chunk_size);

    // 使用 <key> 进行查询.
    node_t * find_node(const key_t &key);    
     //插入节点, 如果旧节点存在, 则返回失败
    node_t * insert_node(const key_t &key, void* new_data, int new_len);
    //修改节点
    node_t * update_node(node_t * node, void* new_data, int new_len, 
    								char* old_data = NULL, int* old_len = NULL);
    //insert or update
    node_t * replace_node(const key_t &key, void* new_data, int new_len, char* old_data = NULL, int* old_len = NULL);
    //删除结点. 同时会将节点从附加链表中清除
    //返回值 = 0 表示成功, < 0 表示失败(如节点不存在,也返回失败)
    int delete_node(node_t * node, char* data = NULL, int* data_len = NULL);

    int merge_node_data(node_t * node, char* data, int* data_len);

    // 返回当前节点使用数
    int used_node_num() { return hash_map_->used_node_num_; }
    int free_node_num() { return hash_map_->node_total_ - hash_map_->used_node_num_; }
    int get_node_total() { return hash_map_->node_total_; }
    int get_bucket_used() { return hash_map_->used_bucket_num_; }
    int free_bucket_num() {return hash_map_->bucket_size_ - hash_map_->used_bucket_num_; }
    int get_bucket_size() {return hash_map_->bucket_size_;}

    CChunkAllocator* chunks() {return &allocator_; }

    // 计算HASH_MAP所需求的内存块尺寸
    static size_t get_pool_size(int node_total, int bucket_size)
    {
        size_t head_size = sizeof(THashMap) - sizeof(BC_MEM_HANDLER[1]);
        size_t bucket_total_size = bucket_size * sizeof(BC_MEM_HANDLER);
        size_t node_total_size = node_total * sizeof(node_t);

        size_t pool_size = head_size + bucket_total_size + node_total_size;
        return pool_size; 
    }
    // 取HASH_MAP 和CHUNK的内存块尺寸
    static size_t get_total_pool_size(int node_total, int bucket_size, int n_chunks, int chunk_size)
    {
        return get_pool_size(node_total, bucket_size) + CChunkAllocator::get_pool_size(n_chunks, chunk_size);
    }

    //transform handler to address
    node_t *handler2ptr(BC_MEM_HANDLER handler);

    //transform address to handler
    BC_MEM_HANDLER ptr2handler(node_t * ptr);

    //附加链表操作方法
    void insert_add_list_head(node_t * node);
    void insert_add_list_tail(node_t * node);
    void delete_from_add_list(node_t * node);
    node_t * get_add_list_prev(node_t * node);
    node_t * get_add_list_next(node_t * node);
    node_t * get_add_list_head();
    node_t * get_add_list_tail();
    ////////////////	

    void set_node_flag(node_t  * node, ENodeFlag f){assert(node); node->flag_ = (int)f;}
    ENodeFlag get_node_flag(node_t  *node){assert(node); return (ENodeFlag)node->flag_;}
    node_t * get_bucket_list_head(unsigned bucket_id);
    node_t * get_bucket_list_prev(node_t * node);
    node_t * get_bucket_list_next(node_t * node);

protected:

    void init_pool_data(int node_total, int bucket_size);
    int verify_pool_data(int node_total, int bucket_size);

    //根据索引计算HASH桶值
    int get_bucket_id(const key_t &key);
    int get_bucket_list_len(int bucket_id); //取HASH桶的碰撞数

    //将节点插入到空闲链表
    void free_list_insert(node_t  *node);
    //从空闲链表中取节点
    node_t  *free_list_remove();

    //节点链表操作方法
    void insert_node_list(node_t * node);
    void delete_from_node_list(node_t * node);

    //初始化节点
    void init_node(node_t * node);
    //将节点置为空闲模式
    void free_node(node_t  *node);
    //将节点置为使用模式
    void use_node(node_t  *node,const key_t &key, int chunk_len, BC_MEM_HANDLER chunk_head);

    char *pool_;        //内存块起始地址
    char *pool_tail_;   //内存块结束地址

    HashFunc hash_func_;//HASH函数
    THashMap* hash_map_;   //内存块中的HASHMAP 结构
    node_t * hash_node_; //内存块中的HASH节点数组
    CChunkAllocator allocator_; //CHUNK分配器

};

}
}


#include "HashMapImp.hpp"

#endif

