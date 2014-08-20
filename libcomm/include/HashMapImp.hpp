
#ifndef _HASH_MAP_IMP_H_
#define _HASH_MAP_IMP_H_



namespace comm
{
namespace commu
{


template<
    typename key_t ,
    typename node_t,
    typename HashFunc
    > 
CHashMap<key_t,node_t,HashFunc>::CHashMap()
{
    pool_ = NULL;
    pool_tail_ = NULL;
    hash_map_ = NULL;
    hash_node_ = NULL;
}

// Clear things up.
template<
    typename key_t ,
    typename node_t,
    typename HashFunc
    > 
CHashMap<key_t,node_t,HashFunc>::~CHashMap()
{
    //NOTHING TO DO
}

template<
    typename key_t ,
    typename node_t,
    typename HashFunc
    > 
int CHashMap<key_t,node_t,HashFunc>::open(char* pool, bool init, int node_total, int bucket_size, int n_chunks, int chunk_size)
{
    int ret = 0;

    size_t hash_map_pool_size = get_pool_size(node_total, bucket_size);
    size_t head_size = sizeof(THashMap) - sizeof(BC_MEM_HANDLER[1]);
    size_t bucket_total_size = bucket_size * sizeof(BC_MEM_HANDLER);

    pool_ = pool;
    pool_tail_ = pool_ + hash_map_pool_size;
    hash_map_ = (THashMap*)pool_;
    hash_node_ = (node_t*)(pool_ + head_size + bucket_total_size);

    if (init)
    {
        init_pool_data(node_total, bucket_size);
    }
    else
    {
        if ((ret = verify_pool_data(node_total, bucket_size)) != 0) 
        {
            return ret;
        }
    }

    if ((ret = allocator_.open(pool_tail_, init, n_chunks, chunk_size)) != 0) 
    {
        return ret;
    }

    return 0;    
}

template<
    typename key_t ,
    typename node_t,
    typename HashFunc
    > 
void CHashMap<key_t,node_t,HashFunc>::init_pool_data(int node_total, int bucket_size)
{
    hash_map_->node_total_ = node_total;
    hash_map_->bucket_size_ = bucket_size;
    hash_map_->used_node_num_ = 0;
    hash_map_->used_bucket_num_ = 0;

    hash_map_->add_head_ = INVALID_BC_MEM_HANDLER;
    hash_map_->add_tail_ = INVALID_BC_MEM_HANDLER;
    hash_map_->free_list_ = INVALID_BC_MEM_HANDLER;

    int i;
    for(i = 0; i < bucket_size; i++)
    {
        hash_map_->bucket[i] = INVALID_BC_MEM_HANDLER;
    }

    //将所有节点插入到空闲链表中
    node_t * hash_node;
    size_t offset;
    for(i = 0; i < node_total; i++)
    {
        offset = i * (sizeof(node_t));
        hash_node = (node_t *)((char*)hash_node_ + offset);
        init_node(hash_node);
        free_list_insert(hash_node);
    }

    return;
}

template<
    typename key_t ,
    typename node_t,
    typename HashFunc
    > 
int CHashMap<key_t,node_t,HashFunc>::verify_pool_data(int node_total, int bucket_size)
{
    if (node_total != hash_map_->node_total_)
    {
        ERROR_RETURN(HASH_MAP_ERROR_BASE, "pool data verify fail : cfg node_total");
    }
    if (bucket_size != hash_map_->bucket_size_)
    {
        ERROR_RETURN(HASH_MAP_ERROR_BASE, "pool data verify fail : bucket_size");
    }

    int used_bucket_count = 0;
    for (int i = 0; i < hash_map_->bucket_size_; i++)
    {
        if (hash_map_->bucket[i] != INVALID_BC_MEM_HANDLER)
        {
            used_bucket_count ++;
        }
    }
    if (used_bucket_count != hash_map_->used_bucket_num_)
    {
        ERROR_RETURN(HASH_MAP_ERROR_BASE, "pool data verify fail : used_bucket_count");
    }

    int free_node_count = 0;
    node_t * free_node = handler2ptr(hash_map_->free_list_);
    while(free_node)
    {
        free_node_count++;
        free_node = handler2ptr(free_node->node_next_);
    }

    if ((hash_map_->used_node_num_ + free_node_count) != hash_map_->node_total_) 
    {
        ERROR_RETURN(HASH_MAP_ERROR_BASE, "pool data verify fail : real node_total_");
    }

    return 0;
}

template<
    typename key_t ,
    typename node_t,
    typename HashFunc
    > 
node_t * CHashMap<key_t,node_t,HashFunc>::find_node(const key_t &key)
{
    int bucket_id = get_bucket_id(key);
    BC_MEM_HANDLER node_hdr = hash_map_->bucket[bucket_id];
    while(node_hdr != INVALID_BC_MEM_HANDLER)
    {
        node_t * node = handler2ptr(node_hdr);
        if (node->key_ == key) 
        {
            //将该节点插入到附加链表头部
            node->add_info_1_ = (int)time(NULL);
            ++node->add_info_2_;
            insert_add_list_head(node);
            return node;
        }
        node_hdr = node->node_next_;
    }
    ERROR_RETURN_NULL(HASH_MAP_ERROR_NODE_NOT_EXIST, "node not exist");
}

template<
    typename key_t ,
    typename node_t,
    typename HashFunc
    > 
node_t * CHashMap<key_t,node_t,HashFunc>::insert_node(const key_t &key, void* new_data, int new_len)
{
    node_t * node = free_list_remove();
    if (node == NULL) 
    {
        return NULL;                
    }

    int new_chunk_num = allocator_.get_chunk_num(new_len);
    BC_MEM_HANDLER head_hdr = allocator_.malloc(new_chunk_num);
    if(head_hdr == INVALID_BC_MEM_HANDLER)
    {
        free_list_insert(node);
        return NULL;
    }

    allocator_.split(head_hdr, new_data, new_len);
    use_node(node, key, new_len, head_hdr);

    //将该节点插入到附加链表头部
    node->add_info_1_ = (int)time(NULL);
    ++node->add_info_2_;
    insert_add_list_head(node);
    return node;
}

template<
    typename key_t ,
    typename node_t,
    typename HashFunc
    > 
node_t  * CHashMap<key_t,node_t,HashFunc>::update_node(node_t * node, void* new_data, int new_len, 
                            char* old_data, int* old_len)
{
    if(old_data != NULL && old_len != NULL)
    {
        //返回旧数据
        if(allocator_.merge(node->chunk_head_, node->chunk_len_,  old_data, old_len) != 0)
        {
            return NULL;
        }
    }
    else if(old_len != NULL)
    {
        *old_len = node->chunk_len_;
    }

    int old_chunk_num = allocator_.get_chunk_num(node->chunk_len_);
    int new_chunk_num = allocator_.get_chunk_num(new_len);

    if (old_chunk_num != new_chunk_num)
    {
        //需要重新分配CHUNK. 先FREE再MALLOC.        
        if (new_chunk_num > old_chunk_num)
        {
            if (allocator_.get_free_chunk_num() < (new_chunk_num - old_chunk_num))
            {
                //剩余CHUNK数不足
                ERROR_RETURN_NULL(CChunkAllocator::CHUNK_ALLOCATOR_ERROR_FREE_CHUNK_LACK, "free chunk lack");
            }
        }

        allocator_.free(node->chunk_head_);

        BC_MEM_HANDLER head_hdr = allocator_.malloc(new_chunk_num);   //CHUNK数足够, 不会失败
        allocator_.split(head_hdr, new_data, new_len);

        node->chunk_len_ = new_len;
        node->chunk_head_ = head_hdr;
    }
    else
    {
        allocator_.split(node->chunk_head_, new_data, new_len);
        node->chunk_len_ = new_len;
    }

    return node;
}
    
template<
    typename key_t ,
    typename node_t,
    typename HashFunc
    > 
node_t  * CHashMap<key_t,node_t,HashFunc>::replace_node(const key_t &key, void* new_data, int new_len, char* old_data, int* old_len)
{
    node_t * node = find_node(key);
    if(node != NULL)
    {
        return update_node(node, new_data, new_len, old_data, old_len);
    }
    return insert_node(key, new_data, new_len);
}

template<
    typename key_t ,
    typename node_t,
    typename HashFunc
    > 
int CHashMap<key_t,node_t,HashFunc>::delete_node(node_t * node, char* data, int* data_len)
{
    //旧节点存在
    if(data != NULL && data_len != NULL)
    {
        //返回旧数据
        if(allocator_.merge(node->chunk_head_, node->chunk_len_, data, data_len) != 0)
        {
            return -1;
        }
    }
    else if(data_len != NULL)
    {
        *data_len = node->chunk_len_;
    }

    delete_from_add_list(node);

    free_node(node);
    free_list_insert(node);
    return 0;
}

template<
    typename key_t ,
    typename node_t,
    typename HashFunc
    > 
void CHashMap<key_t,node_t,HashFunc>::insert_node_list(node_t * node)
{
    //插入到节点链表头
    int bucket_id = get_bucket_id(node->key_);
    BC_MEM_HANDLER node_hdr = ptr2handler(node);

    node->node_next_ = hash_map_->bucket[bucket_id];
    node->node_prev_ = INVALID_BC_MEM_HANDLER;
    hash_map_->bucket[bucket_id] = node_hdr;
    node_t * next_node = handler2ptr(node->node_next_);
    if(next_node != NULL)
    {
        next_node->node_prev_ = node_hdr;
    }
    
    //stat
    hash_map_->used_node_num_ ++;    
}

template<
    typename key_t ,
    typename node_t,
    typename HashFunc
    > 
void CHashMap<key_t,node_t,HashFunc>::delete_from_node_list(node_t * node)
{
    BC_MEM_HANDLER next_node_hdr = node->node_next_;
    BC_MEM_HANDLER prev_node_hdr = node->node_prev_;
    
    if(prev_node_hdr != INVALID_BC_MEM_HANDLER)
    {
        node_t * prev_node = handler2ptr(prev_node_hdr);
        prev_node->node_next_ = node->node_next_;
    }
    if(next_node_hdr != INVALID_BC_MEM_HANDLER)
    {
        node_t * next_node = handler2ptr(next_node_hdr);
        next_node->node_prev_ = node->node_prev_;
    }
    
    BC_MEM_HANDLER node_hdr = ptr2handler(node);
    
    int bucket_id = get_bucket_id(node->key_);
    if (node_hdr == hash_map_->bucket[bucket_id]) 
    {
        //当前节点为链表头节点
        hash_map_->bucket[bucket_id] = next_node_hdr;
        
    }
    
    //将前后链表指针清零
    node->node_next_ = INVALID_BC_MEM_HANDLER;
    node->node_prev_ = INVALID_BC_MEM_HANDLER;    

    //stat
    hash_map_->used_node_num_ --;    
}

template<
    typename key_t ,
    typename node_t,
    typename HashFunc
    > 
void CHashMap<key_t,node_t,HashFunc>::free_node(node_t  *node)
{
    //从链表中删除
    delete_from_node_list(node);
    
    //释放 chunk
    allocator_.free(node->chunk_head_);
    
    //stat
    int bucket_list_len = get_bucket_list_len(get_bucket_id(node->key_));
    if (bucket_list_len == 0)
    {
        //the bucket change to unused
        hash_map_->used_bucket_num_ --;
    }    

    //reset member
    init_node(node);
}

template<
    typename key_t ,
    typename node_t,
    typename HashFunc
    > 
void CHashMap<key_t,node_t,HashFunc>::use_node(node_t  *node, const key_t &key, int chunk_len, 
                              BC_MEM_HANDLER chunk_head)
{
    //set member
    node->key_ = key;
    node->chunk_len_ = chunk_len;
    node->chunk_head_ = chunk_head;
    node->add_info_1_ = 0;
    node->add_info_2_ = 0;

    int bucket_list_len = get_bucket_list_len(get_bucket_id(node->key_));
    if (bucket_list_len == 0)
    {
        //the bucket change from unused
        hash_map_->used_bucket_num_ ++;
    }

    insert_node_list(node);
    return;
}

template<
    typename key_t ,
    typename node_t,
    typename HashFunc
    > 
int CHashMap<key_t,node_t,HashFunc>::get_bucket_list_len(int bucket_id)
{
    int num = 0;

    BC_MEM_HANDLER node_hdr;
    node_hdr = hash_map_->bucket[bucket_id];

    while (node_hdr != INVALID_BC_MEM_HANDLER)
    {
        num ++;		
        node_t * node = handler2ptr(node_hdr);
        node_hdr = node->node_next_;
    }

    return num;
}

template<
    typename key_t ,
    typename node_t,
    typename HashFunc
    > 
void CHashMap<key_t,node_t,HashFunc>::insert_add_list_head(node_t * node)
{
    delete_from_add_list(node);
    BC_MEM_HANDLER node_hdr = ptr2handler(node);
    
    //insert node into head of add list
    node->add_next_ = hash_map_->add_head_;
    hash_map_->add_head_ = node_hdr;

    if (hash_map_->add_tail_ == INVALID_BC_MEM_HANDLER)
    {
        hash_map_->add_tail_ = node_hdr;        
    }

    node->add_prev_ = INVALID_BC_MEM_HANDLER;
    node_t * next_node = handler2ptr(node->add_next_);
    if(next_node != NULL)
    {
        next_node->add_prev_ = node_hdr;
    }
   
}


template<
    typename key_t ,
    typename node_t,
    typename HashFunc
    > 
void CHashMap<key_t,node_t,HashFunc>::insert_add_list_tail(node_t * node)
{
    delete_from_add_list(node);
    //reform add list, insert to head
    BC_MEM_HANDLER node_hdr = ptr2handler(node);
    
    node->add_prev_ = hash_map_->add_tail_;
    hash_map_->add_tail_ = node_hdr;

    if (hash_map_->add_head_ == INVALID_BC_MEM_HANDLER)
    {
        hash_map_->add_head_ = node_hdr;        
    }    

    node->add_next_ = INVALID_BC_MEM_HANDLER;
    node_t * prev_node = handler2ptr(node->add_prev_);
    if(prev_node != NULL)
    {
        prev_node->add_next_ = node_hdr;
    }       
}

template<
    typename key_t ,
    typename node_t,
    typename HashFunc
    > 
void CHashMap<key_t,node_t,HashFunc>::delete_from_add_list(node_t * node)
{
    //link the prev add node and the next add node
    BC_MEM_HANDLER node_hdr = ptr2handler(node);
    BC_MEM_HANDLER next_add_hdr = node->add_next_;
    BC_MEM_HANDLER prev_add_hdr = node->add_prev_;
    
    if ((next_add_hdr == INVALID_BC_MEM_HANDLER) &&
            (prev_add_hdr == INVALID_BC_MEM_HANDLER) &&
            (hash_map_->add_head_ != node_hdr) &&
            (hash_map_->add_tail_ != node_hdr)) 
    {
        //不在链表中
        return ;
    }

    if(prev_add_hdr != INVALID_BC_MEM_HANDLER)
    {
        node_t * prev_add = handler2ptr(prev_add_hdr);
        prev_add->add_next_ = node->add_next_;
    }
    if(next_add_hdr != INVALID_BC_MEM_HANDLER)
    {
        node_t * next_add = handler2ptr(next_add_hdr);
        next_add->add_prev_ = node->add_prev_;
    }
    
    
    if (hash_map_->add_head_ == node_hdr)
    {
        hash_map_->add_head_ =  next_add_hdr;        
    }
    if (hash_map_->add_tail_ == node_hdr) 
    {
        hash_map_->add_tail_ =  prev_add_hdr;
    }
    
    //将前后链表指针清零
    node->add_prev_ = INVALID_BC_MEM_HANDLER;
    node->add_next_ = INVALID_BC_MEM_HANDLER;
    
}

template<
    typename key_t ,
    typename node_t,
    typename HashFunc 
    > 
node_t * CHashMap<key_t,node_t,HashFunc>::get_add_list_head()
{
    return handler2ptr(hash_map_->add_head_);
}

template<
    typename key_t ,
    typename node_t,
    typename HashFunc 
    > 
node_t  * CHashMap<key_t,node_t,HashFunc>::get_add_list_tail()
{
    return handler2ptr(hash_map_->add_tail_);
}

template<
    typename key_t ,
    typename node_t,
    typename HashFunc 
    > 
node_t  * CHashMap<key_t,node_t,HashFunc>::get_add_list_prev(node_t * node)
{
    return handler2ptr(node->add_prev_);
}

template<
    typename key_t ,
    typename node_t,
    typename HashFunc 
    > 
node_t * CHashMap<key_t,node_t,HashFunc>::get_add_list_next(node_t * node)
{
    return handler2ptr(node->add_next_);
}

template<
    typename key_t ,
    typename node_t,
    typename HashFunc 
    > 
inline int CHashMap<key_t,node_t,HashFunc>::get_bucket_id(const key_t &key)
{
	return (unsigned int) hash_func_(key) % hash_map_->bucket_size_;
}

template<
    typename key_t ,
    typename node_t,
    typename HashFunc 
    > 
inline node_t  * CHashMap<key_t,node_t,HashFunc>::handler2ptr(BC_MEM_HANDLER handler)
{
    if (handler == INVALID_BC_MEM_HANDLER)
    {
        return NULL;
    }
    return (node_t *)(pool_ + handler);
}

template<
    typename key_t ,
    typename node_t,
    typename HashFunc 
    > 
inline BC_MEM_HANDLER CHashMap<key_t,node_t,HashFunc>::ptr2handler(node_t * ptr)
{
    char *tmp_ptr = (char *)ptr;
    if((tmp_ptr < pool_) || (tmp_ptr >= pool_tail_))
    {
        return INVALID_BC_MEM_HANDLER;
    }
    return (BC_MEM_HANDLER)(tmp_ptr - pool_);    
}

template<
    typename key_t ,
    typename node_t,
    typename HashFunc 
    > 
inline void CHashMap<key_t,node_t,HashFunc>::free_list_insert(node_t  *node)
{
    //insert to free list's head
    node->node_next_ = hash_map_->free_list_;
    BC_MEM_HANDLER node_hdr = ptr2handler(node);
    hash_map_->free_list_ = node_hdr;
    return;
}

template<
    typename key_t ,
    typename node_t,
    typename HashFunc 
    > 
inline node_t * CHashMap<key_t,node_t,HashFunc>::free_list_remove()
{
    //get head node from free list
    if(hash_map_->free_list_ == INVALID_BC_MEM_HANDLER)
    {
        ERROR_RETURN_NULL(HASH_MAP_ERROR_NO_FREE_NODE, "no free node");
    }
	
    node_t * head_node = handler2ptr(hash_map_->free_list_);
    hash_map_->free_list_ = head_node->node_next_;
    head_node->node_next_ = INVALID_BC_MEM_HANDLER;	
    return head_node;
}

template<
    typename key_t ,
    typename node_t,
    typename HashFunc
    > 
inline void CHashMap<key_t,node_t,HashFunc>::init_node(node_t * node)
{
    node->chunk_len_ = 0;
    node->add_info_1_ = 0;
    node->add_info_2_ = 0;
    node->flag_ = NODE_FLAG_UNCHG;

    node->chunk_head_ = INVALID_BC_MEM_HANDLER;
    node->node_next_= INVALID_BC_MEM_HANDLER;
    node->node_prev_= INVALID_BC_MEM_HANDLER;
    node->add_next_= INVALID_BC_MEM_HANDLER;
    node->add_prev_= INVALID_BC_MEM_HANDLER;

    return;
}

template<
    typename key_t ,
    typename node_t,
    typename HashFunc
    > 
inline node_t *  CHashMap<key_t,node_t,HashFunc>::get_bucket_list_head(unsigned bucket_id)
{
    assert(bucket_id < (unsigned)hash_map_->bucket_size_);

    BC_MEM_HANDLER node_hdr = hash_map_->bucket[bucket_id];
    return node_hdr != INVALID_BC_MEM_HANDLER ? handler2ptr(node_hdr) : NULL; 
}

template<
    typename key_t ,
    typename node_t,
    typename HashFunc
    > 
inline node_t *  CHashMap<key_t,node_t,HashFunc>::get_bucket_list_prev(node_t * node)
{
    assert(node);
    return node->node_prev_!= INVALID_BC_MEM_HANDLER ? handler2ptr( node->node_prev_) : NULL;
}

template<
    typename key_t ,
    typename node_t,
    typename HashFunc
    > 
inline node_t *  CHashMap<key_t,node_t,HashFunc>::get_bucket_list_next(node_t * node)
{
    assert(node);
    return node->node_next_!= INVALID_BC_MEM_HANDLER ? handler2ptr( node->node_next_) : NULL;
}

template<
    typename key_t ,
    typename node_t,
    typename HashFunc
    > 
inline int CHashMap<key_t,node_t,HashFunc>::merge_node_data(node_t * node, char* data, int* data_len)
{
    assert(node);
   if(data != NULL && data_len != NULL)
   	return allocator_.merge(node->chunk_head_, node->chunk_len_, data, data_len);

   if(data_len  != NULL )
   	*data_len = node->chunk_len_;
   return -1;
}


}
}


#endif

