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
#include "loadtable.h"
#include "misc.h"

using namespace comm::base;

const unsigned char HEAD_LEN = sizeof(unsigned *);

namespace comm
{
namespace load
{

//内部使用的数据结构
typedef struct tagBlockHeader
{
    tagBlockHeader()
    {
    	memset(this,0x0,sizeof(tagBlockHeader));
    }
    ~tagBlockHeader(){}

    char key_[MAX_KEY_LEN];//保存KEY的BUF
    unsigned int timestamp_;//更新的时间戳
    unsigned char flag_;//块标记
    unsigned short len_;//数据块长度，不包括Header的长度						
}BlockHeader;


CLoadTable::CLoadTable():shmkey_(0),shmsize_(0),shmid_(0),shmmem_(NULL),tail_(NULL),block_(NULL),blocksize_(0),lock_(NULL)
{
}

CLoadTable::~CLoadTable()
{
    fini();
}


int CLoadTable::init(int shmkey, int shmsize,int semkey /*=-1*/)
{
    fini();

    lock_ = new CNullLock;

    shmkey_ = shmkey;
    shmsize_ = shmsize;
    int ret = 0;
    if((ret = getmemory(shmkey_, shmsize_)) > 0)
    {
        memset(shmmem_, 0x0, HEAD_LEN);//前HEAD_LEN个字节用于保存tail_
    }
    else if(ret < 0)
    {
        return ret;
    }

    //the tail of data section
    tail_ =  (unsigned*)shmmem_;
    //data section base address
    block_ = (char*)(tail_ + 1);
    //data section length
    blocksize_ = shmsize_ - HEAD_LEN; 

    xprintf("CLoadTable::init ok,shmmem_=0x%X,block_=0x%X,blocksize=%u, the tail_ = %u\n",
        (int)shmmem_,(int)block_,blocksize_,*tail_);

    return 0;
}

int CLoadTable::getmemory(int shmkey, int shmsize)
{
    if((shmid_ = shmget(shmkey, shmsize, 0)) != -1)  
    {
        shmmem_ = shmat(shmid_, NULL, 0);
        if(shmmem_ != MAP_FAILED)
        {       
            return 0;
        }
        else
        {	
            return -1;
        }
    }
    else
    {		
        shmid_ = shmget(shmkey, shmsize, IPC_CREAT | 0666);
        if(shmid_ != -1)  
        {	
            shmmem_ = shmat(shmid_, NULL, 0);
            if(shmmem_ != MAP_FAILED)
            {       
                return 1;
            }
            else
            {	
                return -2;
            }	
        }
        else
        {
            return -3;
        }
    }


}

void CLoadTable::fini()
{
    if(shmmem_)
        shmdt((const void*)shmmem_);

    if(lock_ != NULL)
        delete lock_;
    lock_ = NULL;
}

int CLoadTable::Init_LoadItem(const char * key, unsigned short len)
{
    if(key == NULL )
    {
        xprintf("key=%s invalid !\n",key);
        return 0;//Invalid key
    }
    //first,find in the map
    Iter itr = items_.find(key);
    if(itr != items_.end() )
    {
        //find
        xprintf("find in map,key=%s\n",key);
        return itr->second.len_;
    }

    CLockGuard guard(lock_);//Lock the share memory
    //second,exist in share memory?
    unsigned short buflen = 0;
    unsigned offset = findblock(key, buflen);
    if( offset > 0 && buflen > 0 )
    {
        //find in memory,insert to map
        tagItemInfo info;
        info.len_ = buflen;
        info.pos_ = offset;
        items_[key] = info;

        xprintf("find in memory,key=%s,pos = %d,len=%d\n",key,offset,buflen);
        return buflen;
    }

    int headlen = sizeof(BlockHeader);

    //third,allock memory for store data
    unsigned tail = *tail_;
    if( (tail + headlen + len )> blocksize_)
    {
        xprintf("over flow!!!key=%s\n",key);
        return 0;//Overflow
    }

    char * pBlock = block_ + tail;
    BlockHeader header;
    header.len_ = len;
    strncpy(header.key_,key,MAX_KEY_LEN);
    memcpy(pBlock,&header,headlen);
    memset(pBlock + headlen,0x0,len);

    *tail_ = tail + headlen + len;

    xprintf("now,the tail are:%u\n",*tail_);

    //insert into map
    {
        tagItemInfo info;
        info.len_ = len;
        info.pos_ = tail + headlen;
        items_[key] = info;
    }

    return len;


}


unsigned CLoadTable::findblock(const char * key,unsigned short &len)
{
    len = 0;
    if(key == NULL)
    {
        xprintf("invalid key!!!key=%s\n",key);
        return 0;//invalid key
    }

    unsigned tail = *tail_;

    if( 0 == tail || tail >= blocksize_ )
    {
        xprintf("find key=%s,empty data or overflow,tail=%u!\n",key,tail);
        return 0;//empty or overflow
    }

    BlockHeader header ;
    char * pBlock = block_;
    int headlen = sizeof(BlockHeader);
    int nPos = 0;

    while(pBlock < block_ + tail )
    {
        nPos = pBlock - block_;		
        memcpy((void *) &header,pBlock,sizeof(BlockHeader));		
        if(strcmp((const char *) header.key_,key) == 0 
        && (pBlock + headlen + header.len_) <= (block_ + tail) )
        {
            //find 
            len = header.len_;
            return (nPos+ headlen);
        }
        else
        {
            pBlock += headlen + header.len_;
        }
    }

    return 0;

}

int CLoadTable::read(void * buf, unsigned short buf_len, const char * key,int timeout /*=60 */)
{
    if(key == NULL)
        return 0;

    tagItemInfo info = {0};
    //find  in map
    Iter itr = items_.find(key);
    if(itr != items_.end())
    {
        info = itr->second;
    }
    else
    {
        return -1;//not find
    }

    unsigned tail = *tail_;
    if( (info.pos_ + info.len_) > tail )
        return 0;

    CLockGuard guard(lock_);

    BlockHeader header;
    int head_len = sizeof(BlockHeader);
    memcpy(&header,block_ + info.pos_ - head_len,head_len);


    //判断信息是否过期
    unsigned int nowtime = time(NULL);
    if( (nowtime -  header.timestamp_ ) > (unsigned int ) timeout ) 
        return 0;

    int readlen = info.len_ > buf_len ? buf_len:info.len_;
    xprintf("read key=%s in pos=%u,len=%d\n",key,info.pos_,readlen);
    memcpy(buf,block_ + info.pos_,info.len_);
    return readlen;

}

int CLoadTable::write(const char * key, void * buf, unsigned short len)
{
    if(key == NULL)
        return 0;

    tagItemInfo info = {0};
    //find  in map
    Iter itr = items_.find(key);
    if(itr != items_.end())
    {
        info = itr->second;
    }
    else
    {
        return -1;//not find
    }

    unsigned tail = *tail_;
    if( (info.pos_ + info.len_) >  tail )
        return 0;

    CLockGuard guard(lock_);
    BlockHeader header;
    int head_len = sizeof(BlockHeader);
    memcpy(&header,block_ + info.pos_ - head_len,head_len);
    header.timestamp_ = time(NULL);
    memcpy(block_ + info.pos_ - head_len,&header,head_len);
    int writelen = info.len_ > len ? len:info.len_;
    xprintf("write key=%s in pos=%u,len=%d\n",key,info.pos_,writelen);
    memcpy(block_ + info.pos_,buf,writelen);
    return writelen;

}

}
}
