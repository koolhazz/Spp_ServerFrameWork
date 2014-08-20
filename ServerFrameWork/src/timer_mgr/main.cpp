#include <signal.h>
#include <sys/resource.h>
#include <unistd.h>
#include <stdio.h>
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

#include "misc.h"


#ifndef SIGUSR3
#define SIGUSR3 (SIGRTMIN + 1)
#endif

#define RUN_FLAG_QUIT		0x01
#define RUN_FLAG_RELOAD		0x02
#define RUN_FLAG_RELOAD_MODULE 0x04


using namespace comm::base;


static void do_kill(int signo, int procid)
{
    char cmd_buf[64] = {0};

    if(procid <= 0)
        return;
    snprintf(cmd_buf, sizeof(cmd_buf) - 1, "kill -%d %u > /dev/null 2>&1", signo, procid);	
    system(cmd_buf);
}

static void* map_file (const char* filename, int size, bool& f_new)
{
    f_new = true;
   int t_fd = ::open(filename, O_RDONLY);
   if(t_fd > 0)
    {
        ::close(t_fd);
        f_new = false;
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
static int WriteToFile(const char * pszName, const char * buffer, unsigned int buf_len)
{
    int file_fd = open(pszName, O_CREAT | O_RDWR | O_APPEND, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    if(file_fd < 0)
    {
        printf("Open file %s fail !\n", pszName);
        return -1;
    }

    if(write(file_fd, buffer, buf_len) !=(int) buf_len )
    {
        printf("write file %s fail !\n", pszName);
        close(file_fd);
        return -1;
    }

     close(file_fd);
     file_fd = 0;
    return 0;
}
#endif

static int DbgLog(const char *fmt, ...)
{
    return 0;
#if 0
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
#endif
}




class CTimerTask
{
public:
    int run(int argc,char ** argv);
    void startup(bool bg_run);


protected:
    int realrun(int argc,char ** argv);
    static void sigusr1_handle(int signo);
    static void sigusr2_handle(int signo);
    static void sigusr3_handle(int signo);
    static bool reload();
    static bool quit();

    static unsigned char  flag_;
    
};

unsigned char CTimerTask::flag_ = 0;

int CTimerTask::run(int argc, char * * argv)
{
    startup(true);
    return realrun(argc,argv);
}

void CTimerTask::startup(bool bg_run)
{
    signal(SIGHUP, SIG_IGN);
    signal(SIGPIPE, SIG_IGN);
    signal(SIGTTOU, SIG_IGN);
    signal(SIGTTIN, SIG_IGN);
    signal(SIGCHLD, SIG_IGN);
    if(bg_run)
    {		
        signal(SIGINT,  SIG_IGN);
        signal(SIGTERM, SIG_IGN);
        daemon(1, 1);		
    }

    CTimerTask::flag_ = 0;
    signal(SIGUSR1, CTimerTask::sigusr1_handle);
    signal(SIGUSR2, CTimerTask::sigusr2_handle);
    signal(SIGUSR3, CTimerTask::sigusr3_handle);

}


int CTimerTask::realrun(int argc, char * * argv)
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

    ptm_mgr->pid = getpid();
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

        //ºÏ≤Èquit–≈∫≈
        if( CTimerTask::quit())
        {    
            DbgLog("recv quit signal,timer task will be quit!!!\n");
            break;    
        }

        usleep(5000);
    }

    //log end
    DbgLog("%s:end ...\n",__FUNCTION__);

    return 0;
			 
}



void CTimerTask::sigusr1_handle(int signo)
{
    DbgLog("%s|enter\n",__FUNCTION__);
    flag_ |= RUN_FLAG_QUIT;
    signal(SIGUSR1, CTimerTask::sigusr1_handle);	
    DbgLog("%s|leave\n",__FUNCTION__);
}
void CTimerTask::sigusr2_handle(int signo)
{
    DbgLog("%s|enter\n",__FUNCTION__);
    flag_ |= RUN_FLAG_RELOAD;
    signal(SIGUSR2, CTimerTask::sigusr2_handle);
    DbgLog("%s|leave\n",__FUNCTION__);
}

void CTimerTask::sigusr3_handle(int signo)
{
    DbgLog("%s|enter\n",__FUNCTION__);
    flag_ |= RUN_FLAG_RELOAD_MODULE;
    signal(SIGUSR3,CTimerTask::sigusr3_handle);
    DbgLog("%s|leave\n",__FUNCTION__);
}



bool CTimerTask::quit()
{
    if(flag_ & RUN_FLAG_QUIT)
    {
        flag_ &= ~RUN_FLAG_QUIT;
        return true;
    }
    else
    {
        return false;
    }
}

bool CTimerTask::reload()
{
    if(flag_ & RUN_FLAG_RELOAD)
    {
        flag_ &= ~RUN_FLAG_RELOAD;
        return true;
    }	
    else
    {
        return false;	
    }
}

int main(int argc, char* argv[])
{
    CTimerTask task;
    task.run(argc,argv);
    return 0;
}

