
#include "SVShm.hpp"

namespace comm
{
namespace commu
{

CSVShm::CSVShm()  : internal_id_ (0),  size_ (0), segment_ptr_ (0)
{	
}
key_t CSVShm::f2k(const char *pathname, int id)
{
    return ftok(pathname, id);
}

int CSVShm::force_open_and_attach (key_t key, size_t sz, int &init, void *virtual_addr, int flags)
{
    int ret;	
    init = 1;	
    
    ret = this->open(key, sz, SVSHM_CREATE_EXCL, SVSHM_DEFAULT_PERM);
    if(ret == -1)
    {
        if(errno != EEXIST)
        {
            //DEBUG_P(LOG_FATAL, "Open Shm %u fail.\n", key);
            return -1;
        }
        //已经存在
        //DEBUG_P(LOG_TRACE, "Shm %u already exist, so try to open it.\n", key);
        ret = this->open(key, sz, 0, SVSHM_DEFAULT_PERM);
        if(ret == -1)
        {
            //sz不同,删除旧的，再创建一个新的
            //DEBUG_P(LOG_FATAL, "Shm %u exist, but it's size is not matchwith (may be less than) requested size, so rm & creat.\n", key); 
            ret = this->open(key, 0, 0, SVSHM_DEFAULT_PERM);
            if(ret == -1)
            {
                //DEBUG_P(LOG_FATAL, "Open OLD Shm %u Error.\n", key);
                return -1;
            }
            else
            {
                ret = this->remove();
                if(ret == -1)
                {
                    //DEBUG_P(LOG_FATAL, "rm Shm %u fail.\n", key);
                    return -1;
                }
                ret = this->open(key, sz, SVSHM_CREATE_EXCL, SVSHM_DEFAULT_PERM);
                if(ret == -1)
                {	
                    //DEBUG_P(LOG_FATAL, "Open NEW Shm %u Error.\n", key);
                    return -1;
                }
            }
        }		
        else
        {
            init = 0;
        }
    }//end if(ret == -1

    ret = this->attach();
    if(ret)
    {
        //DEBUG_P(LOG_FATAL, "Attach Shm %d fail.\n", key);
    }
    else
    {
        //DEBUG_P(LOG_FATAL, "Attach Shm (key %d, id %u) OK, shm size %u.\n", key, get_id(), get_segment_size());
    }

    return ret;

}
int CSVShm::open_and_attach (key_t key, size_t sz, int create, int perms,
				       void *virtual_addr, int flags)
{
    if (this->open (key, sz, create, perms) == -1)
    {
        return -1;
    }
    
    else if (this->attach (virtual_addr, flags) == -1)
    {
        return -1;
    }
    else
    {
        return 0;
    }
}
int CSVShm::open(key_t key, size_t sz, int create, int perms)
{
    this->segment_ptr_ = 0;
    this->size_ = sz;
    this->internal_id_ = shmget (key, sz, create | perms);

    return this->internal_id_ == -1 ? -1 : 0;
}


int CSVShm::attach(void *virtual_addr , int flags)
{
    this->segment_ptr_ = shmat (this->internal_id_, virtual_addr, flags);
    return this->segment_ptr_ == (void *) -1 ? -1 : 0;
}

int  CSVShm::detach (void)
{
     return shmdt (this->segment_ptr_);
}
int  CSVShm::remove (void)
{
    return shmctl (this->internal_id_, IPC_RMID, 0);
}
int  CSVShm::control (int cmd, void *buf)
{
    return shmctl (this->internal_id_, cmd, (struct shmid_ds *) buf);
}

void *CSVShm::get_segment_ptr (void) const
{
    return this->segment_ptr_;
}
size_t CSVShm::get_segment_size (void) const
{
    return this->size_;
}
int CSVShm::get_id (void) const
{
    return this->internal_id_;
}

}
}


