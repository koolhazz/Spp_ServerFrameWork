#include <stdio.h>
#include <string.h>
#include "serverbase.h"

namespace spp
{
namespace base
{

char CServerBase::version_desc[64] = {"UpdateClient_1.4.2_release"};
unsigned char CServerBase::flag_ = 0;

#ifndef SIGUSR3
#define SIGUSR3 (SIGRTMIN + 1)
#endif

CServerBase::CServerBase()
{
    ix_ = new TInternal;
    ix_->argc_ = 0;
    ix_->argv_ = NULL;
}

CServerBase::~CServerBase()
{
    if(ix_ != NULL)
    {	
        if(ix_->argv_ != NULL)
        {	
            for(unsigned i = 0; i < (unsigned)ix_->argc_; ++i)
                if(ix_->argv_[i] != NULL)
                    delete ix_->argv_[i];
            delete [] ix_->argv_;
        }
        delete ix_;
    }

}

void CServerBase::ShowVerInfo()
{
    printf("\nVer:%s Date:%s, (C) JinWangXinTong Technology\n\n", version_desc, __DATE__);
    return;
}


void CServerBase::run(int argc, char* argv[])
{
    if(argc < 2)
    {
        ShowVerInfo();
        printf("usage: %s config_file flag\n\n", argv[0]);
        return;
    }

    char* p = strstr(argv[1], "-");
    bool   ret = true;
    if(p != NULL)
    {
        ++p;
        if(*p == '-')
            ++p;

        if(*p == 'v' && *(p+1) == '\0')
        {
            ShowVerInfo();
        }
        else if(*p == 'h' && *(p+1) == '\0')
        {
            ShowVerInfo();
            printf("usage: %s config_file flag\n\n", argv[0]);
        } 
        else
        {
            printf("\ninvalid argument.\n");	
            ShowVerInfo();
            printf("usage: %s config_file flag\n\n", argv[0]);
        }

        return;
    }

    if(argc > 3)
    {
        ret = false;
    }

    //拷贝参数
    ix_->argc_ = argc;
    ix_->argv_ = new char*[ix_->argc_ + 1];	

    int i;
    for(i = 0; i < ix_->argc_; ++i)
    {
        ix_->argv_[i] = strdup(argv[i]);
    }
    
    ix_->argv_[ix_->argc_] = NULL;

    startup(ret);

    realrun(argc, argv);
}
void CServerBase::startup(bool bg_run)
{
    //默认需要root权限才能setrlimit
    /*
    struct rlimit rlim;
    rlim.rlim_cur = 100000; 
    rlim.rlim_max = 100000; 
    if(setrlimit(RLIMIT_NOFILE, &rlim))
    printf("warnning: setrlimit RLIMIT_NOFILE fail\n"); 
    */

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

    CServerBase::flag_ = 0;
    signal(SIGUSR1, CServerBase::sigusr1_handle);
    signal(SIGUSR2, CServerBase::sigusr2_handle);
    signal(SIGUSR3, CServerBase::sigusr3_handle);
	
}


bool CServerBase::reload()
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

bool CServerBase::quit()
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

bool CServerBase::reloadmoduleconfig()
{
    if(flag_ & RUN_FLAG_RELOAD_MODULE)
    {
        flag_ &= ~RUN_FLAG_RELOAD_MODULE;
        return true;
    }
    else
    {
        return false;
    }    
}

void CServerBase::sigusr1_handle(int signo)
{
    flag_ |= RUN_FLAG_QUIT;
    signal(SIGUSR1, CServerBase::sigusr1_handle);
}
void CServerBase::sigusr2_handle(int signo)
{
    flag_ |= RUN_FLAG_RELOAD;
    signal(SIGUSR2, CServerBase::sigusr2_handle);
}

void CServerBase::sigusr3_handle(int signo)
{
    flag_ |= RUN_FLAG_RELOAD_MODULE;
    signal(SIGUSR3,CServerBase::sigusr3_handle);
}



}

}


