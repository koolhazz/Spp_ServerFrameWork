#ifndef _SV_SHM_HPP_
#define _SV_SHM_HPP_

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <errno.h>

namespace comm
{
namespace commu
{

class CSVShm
{
public:
    enum
    {
        SVSHM_CREATE_EXCL = IPC_CREAT | IPC_EXCL,		
        SVSHM_CREATE = IPC_CREAT,
        SVSHM_OPEN = 0,
        SVSHM_DEFAULT_PERM = 0600
    };

    CSVShm(void);
    key_t f2k(const char *pathname, int id);
    int  force_open_and_attach (key_t key, size_t sz, int &init, void *virtual_addr = 0, int flags = 0);

    int  open_and_attach (key_t key, size_t size, int create = SVSHM_OPEN, int perms = SVSHM_DEFAULT_PERM,
            void *virtual_addr = 0,  int flags = 0);
    int open(key_t key, size_t sz, int create = SVSHM_OPEN, int perms = SVSHM_DEFAULT_PERM);
    int attach(void *virtual_addr = 0, int flags =0);
    int  detach (void);
    int  remove (void);
    int  control (int cmd, void *buf);

    void *get_segment_ptr (void) const;
    size_t get_segment_size (void) const;
    int get_id (void) const;

private:
    int internal_id_;
    size_t size_;
    void *segment_ptr_;	
};

}
}


#endif	

