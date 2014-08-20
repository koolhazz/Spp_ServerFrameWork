#include "misc.h"
#include <fcntl.h>
#include <stdarg.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <net/if.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <malloc.h>
#include <stdio.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <asm/atomic.h>
#include <time.h>



//using namespace comm::base;

namespace comm
{
namespace base
{

static void* map_file (const char* filename, int size, bool& f_new)
{
    f_new = true;
   int t_fd = ::open(filename, O_RDONLY);
   if(t_fd > 0)
    {
        ::close(t_fd);
        f_new = false;
        //printf("%s is old\n", filename);
    }

    int fd = ::open(filename, O_RDWR|O_CREAT, 0666);
    void *map = NULL;

    if(fd >= 0)
    {
        if(size > 0)
            ftruncate(fd, size);
        else
            size = lseek(fd, 0L, SEEK_END);

        if(size > 0)
            map = mmap(0, size, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
        ::close(fd);
    } 
    else if(size > 0) 
    {
        map = mmap(0, size, PROT_READ|PROT_WRITE, MAP_ANONYMOUS, -1, 0);
    }

    if(map == MAP_FAILED)
    {
        map = NULL;
    }

    return map;
}

struct time_mgr
{
    struct timeval  tval;
    int  tm;
    atomic_t tm_fag;
    atomic_t tval_fag;
    int pid;
};

#if 0

static void do_kill(int signo, int procid)
{
    char cmd_buf[64] = {0};

    if(procid <= 0)
        return;
    snprintf(cmd_buf, sizeof(cmd_buf) - 1, "kill -%d %u > /dev/null 2>&1", signo, procid);	
    system(cmd_buf);
}


static int WriteToFile(const char * pszName, const char * buffer, unsigned int buf_len)
{
    int file_fd = open(pszName, O_CREAT | O_RDWR | O_APPEND, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    if(file_fd < 0)
    {
        printf("Open file %s fail !\n", pszName);
        return -1;
    }

    if(write(file_fd, buffer, buf_len) != buf_len )
    {
        printf("write file %s fail !\n", pszName);
        close(file_fd);
        return -1;
    }

     close(file_fd);
     file_fd = 0;
    return 0;
}

static int DbgLog(const char *fmt, ...)
{
    const char * pszLogFile = "../log/spp_timer.log";
    const unsigned int MAX_BUFF_LEN = 1024 * 4;
    char  BUFFER[MAX_BUFF_LEN] = {0};
    unsigned int buff_len = 0;

    time_t now = time(NULL);
    struct tm tmm;
    localtime_r(&now, &tmm); 

    buff_len += snprintf(BUFFER + buff_len, MAX_BUFF_LEN - buff_len - 1, "[%04d-%02d-%02d %02d:%02d:%02d]", 
        tmm.tm_year + 1900, tmm.tm_mon + 1, tmm.tm_mday, tmm.tm_hour, tmm.tm_min, tmm.tm_sec); 

    pid_t tid = getpid();
    buff_len += snprintf(BUFFER + buff_len,MAX_BUFF_LEN - buff_len - 1,"[pid=%-5d]", tid);

    va_list ap;
    va_start(ap, fmt);
    buff_len += vsnprintf(BUFFER + buff_len, MAX_BUFF_LEN - buff_len - 1,fmt, ap);
    va_end(ap); 

    WriteToFile(pszLogFile,BUFFER,buff_len);

    return 0;
       
}


void get_time_task()
{
    bool f_new;
    struct time_mgr* ptm_mgr = (struct time_mgr*)  map_file("./time_mgrtask",sizeof(struct time_mgr), f_new);
    if(!ptm_mgr)
    {
        printf("map ./time_mgrtask fail\n ");
        printf("create time mgr task fail\n");
        DbgLog("map ./time_mgrtask fail|pid=%d\n",getpid());
        exit(0);
    }
    	
    gettimeofday(&ptm_mgr->tval, NULL);
    ptm_mgr->tm = time(NULL);

    if(!f_new)
    {
        DbgLog("old_pid = ptm_mgr->pid=%d\n",ptm_mgr->pid);
        if(ptm_mgr->pid != 0)
        {
            DbgLog("kill old spp_timer,pid=%d\n",ptm_mgr->pid);
            do_kill(9, ptm_mgr->pid); 
        }
        ptm_mgr->pid = 0;
    }

    int pid = fork();

    if(pid < 0)
    {
        printf("create time mgr task fail: %m\n");
        DbgLog("create time mgr task fail: %m\n");
        exit(0);
    }

    if(pid > 0)
    {
        DbgLog("create time mgr task ok,pid=%d\n",pid);
        ptm_mgr->pid = pid;
        return;
    }

    //ptm_mgr->pid = pid;
    int count = 0;
    int log_flag = 0;

    //log start
    DbgLog("%s:begin time mgr task,pid=%d\n",__FUNCTION__,getpid());

    while(true)
    {
        atomic_add(1,&ptm_mgr->tval_fag);
        gettimeofday(&ptm_mgr->tval, NULL);

        count++;
        if(count > 20)
        {
            count = 0;   
            atomic_add(1,&ptm_mgr->tm_fag);
            ptm_mgr->tm = time(NULL);            
        }

        ++log_flag;
        if(log_flag > 10000 )
        {
            log_flag = 0;
            DbgLog("%s:time mgr task running...,pid=%d\n",__FUNCTION__,getpid());
        }

        usleep(5000);
    }

    //log end
    DbgLog("%s:end ...\n",__FUNCTION__);
    
			 
}
#endif

int get_timebytask(const char* tmfile)
{
    if(!tmfile)
    {
        tmfile = "./time_mgrtask";
    }
  
    bool f_new;
    static  struct time_mgr* ptm_mgr = (struct time_mgr*)  map_file(tmfile,sizeof(struct time_mgr), f_new);

    if(!ptm_mgr)
        return time(NULL);

    int c_tm = 0;
    int flag;

    do
    {
        flag = atomic_read(&ptm_mgr->tm_fag) ;
        c_tm = ptm_mgr->tm;
    }while(atomic_read(&ptm_mgr->tm_fag) != flag);  

    return c_tm;

}

void get_timeofday(struct timeval* pval,const char* tmfile)
{
    if(!tmfile)
    {
        tmfile = "./time_mgrtask";
    }

    bool f_new;
    static  struct time_mgr* ptm_mgr = (struct time_mgr*)  map_file(tmfile,sizeof(struct time_mgr), f_new);   

    if(!ptm_mgr)
    {
        gettimeofday(pval,NULL);
        return;
    }

    int flag = 0;

    do
    {
        flag = atomic_read(&ptm_mgr->tval_fag) ;

        pval->tv_sec = ptm_mgr->tval.tv_sec;
        pval->tv_usec = ptm_mgr->tval.tv_usec;

    }while(atomic_read(&ptm_mgr->tval_fag) != flag);

    return;	
  
}




unsigned CMisc::getip(const char* ifname)
{
    if(!ifname)
        return 0;

    register int fd, intrface;
    struct ifreq buf[10];
    struct ifconf ifc;
    unsigned ip = 0;

    if((fd = socket(AF_INET, SOCK_DGRAM, 0)) >= 0)
    {
        ifc.ifc_len = sizeof(buf);
        ifc.ifc_buf = (caddr_t)buf;
        if(!ioctl(fd, SIOCGIFCONF, (char*)&ifc))
        {
            intrface = ifc.ifc_len / sizeof(struct ifreq);
            while(intrface-- > 0)  
            {
                if(strcmp(buf[intrface].ifr_name, ifname) == 0)
                {
                    if(!(ioctl(fd, SIOCGIFADDR, (char *)&buf[intrface])))
                        ip = (unsigned)((struct sockaddr_in *)(&buf[intrface].ifr_addr))->sin_addr.s_addr;
                    break;
                }
            }
        }
        close(fd);
    }
    return ip;
}
unsigned CMisc::getdelay(const struct timeval& begin)
{
    struct timeval now;
    //gettimeofday(&now, NULL);
    get_timeofday(&now);
    unsigned delay = 0;
    if((delay = now.tv_sec - begin.tv_sec) > 0)
    {
        delay *= 1000;	
    }

    if(now.tv_usec > begin.tv_usec)
    {
        delay += (now.tv_usec - begin.tv_usec) / 1000;		
    }
    else
    {
        delay -= (begin.tv_usec - now.tv_usec) / 1000;
    }

    return delay;
}
unsigned CMisc::getmemused()
{
    struct mallinfo info = mallinfo();
    return (info.arena + info.hblkhd) / 1024;	
}


}

}

