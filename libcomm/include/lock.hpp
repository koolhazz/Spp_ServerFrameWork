#ifndef _AUTOLOCK_H
#define _AUTOLOCK_H

#include <sys/sem.h>
#include <errno.h>
#include <sys/types.h>


class CAutoLock
{
    int _semid;

public:
    CAutoLock(int semid) 
    {
        _semid = semid;

        lock();
    };

    ~CAutoLock() 
    {
        unlock();
    };

private:
	void lock() 
        {
		for(;;) 
                  {
			struct sembuf sops;
			sops.sem_num = 0;
			sops.sem_op = -1;
			sops.sem_flg = SEM_UNDO;

			int ret = semop(_semid, &sops, 1);
			if(ret<0) 
                            {
				if(errno == EINTR) 
                                    {
#ifdef OPEN_PRINT
					printf("%s lock EINTR : %d,%d,%d,%s\n",__FUNCTION__,ret,_semid,errno,strerror(errno));
#endif
					continue;
				}
				else 
                                    {
#ifdef OPEN_PRINT
					printf("%s lock fail : %d,%d,%d,%s\n",__FUNCTION__,ret,_semid,errno,strerror(errno));
#endif
					return;
				}
			} 
                           else 
			{
				break;
			}
		}
	};
	
	void unlock() 
        {
		for(;;) 
                  {
			struct sembuf sops;
			sops.sem_num = 0;
			sops.sem_op = 1;
			sops.sem_flg = SEM_UNDO;

			int ret = semop(_semid, &sops, 1);
			if(ret<0) 
                            {
				if(errno == EINTR)
                                    {
#ifdef OPEN_PRINT
					printf("%s unlock EINTR : %d,%d,%d,%s\n",__FUNCTION__,ret,_semid,errno,strerror(errno));
#endif
					continue;
				}
				else 
                                    {
#ifdef OPEN_PRINT
					printf("%s unlock fail : %d,%d,%d,%s\n",__FUNCTION__,ret,_semid,errno,strerror(errno));
#endif
					return;
				}
			} 
                           else 
			{
				break;
			}
		}
	};
};

#endif

