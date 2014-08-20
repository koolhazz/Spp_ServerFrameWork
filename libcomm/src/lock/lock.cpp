#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdarg.h>
#include <time.h>
#include <linux/unistd.h>
#include <errno.h>
#include "lock.h"
#include "likelydef.h"

namespace comm
{
namespace lock
{

int CSemLock::sem_init(int key)
{
    semid_ = semget(key, 1, IPC_CREAT | 0666);
    if(semid_ != -1)
    {
        union logsemun arg;
        arg.val = 1;
        semctl(semid_, 0, SETVAL, arg);
    } 
    else 
    {
        return -1;
    }
    return 0;
}
int CSemLock::sem_lock()
{
    struct sembuf op;
    op.sem_num = 0;
    op.sem_flg = SEM_UNDO;
    op.sem_op = -1;

    if(unlikely(semop(semid_, &op, 1) == -1))
    {
        return -1;
    }
    return 0;
}
int CSemLock::sem_unlock()
{
    struct sembuf op;
    op.sem_num = 0;
    op.sem_flg = SEM_UNDO;
    op.sem_op = 1;
    while(semop(semid_, &op, 1) == -1) 
    {
        if(errno != EINTR)
            return -1;
    }
    return 0;
}



}

}


