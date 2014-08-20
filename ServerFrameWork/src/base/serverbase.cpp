
#include "serverbase.h"
#include <stdio.h>
#include "misc.h"

using namespace spp::base;
using namespace comm::base;

char CServerBase::version_desc[64] = {"ServerFramework_1.4.2_release"};
unsigned char CServerBase::flag_ = 0;

/////////////////////////////////////////////////////
//队列相关操作
#if 0
int CContextQueue::AddItem(const string & key,const TContext * context)
{
    Iter itr = ctx_map_.find(key);
    if(itr != ctx_map_.end() )
        return -1;//exist
    if(ctx_map_.size() >= max_len_ )
        return -2;//over flow

    ctx_map_[key] = (TContext * ) context;
    return 0;
}

TContext * CContextQueue::GetItem(const string & key, bool bRemove /* = false*/)
{
    Iter itr = ctx_map_.find(key);
    if(itr == ctx_map_.end())
        return NULL;//not found

    TContext * context = itr->second;
    if(bRemove)
    {
        ctx_map_.erase(itr);
    }
    return context;

}

//取得超时的列表
int CContextQueue::GetTimeOutList(std::vector<string> & v)
{
    int now = 	get_timebytask(NULL);//当前时间
    Iter itr = ctx_map_.begin(),last = ctx_map_.end();
    for(;itr != last;itr++)
    {
        if(unlikely(itr->second != NULL &&  (now - itr->second->timestamp_) > itr->second->timeout_) )
        {
            //timeout
            v.push_back(itr->first);
        }
    }

    return 0;
}
#endif
/////////////////////////////////////////////////////


struct CServerBase::TInternal
{
    //main参数值
    int argc_;
    char** argv_;

    //监控信息上报间隔时间
    unsigned int moni_inter_time_;

    //进程组个数
    unsigned int group_num_;

    //当前进程所属的进程组号
   unsigned  int cur_group_id_;

    //进程组最大进程数
    unsigned int max_proc_num_;
};

CServerBase::CServerBase()
{
    ix_ = new TInternal;
    ix_->argc_ = 0;
    ix_->argv_ = NULL;
    ix_->moni_inter_time_ = 15;
    ix_->group_num_ = 0;
    ix_->cur_group_id_ = 0;
    ix_->max_proc_num_ = 0;

    memset(event_func_list_, 0, sizeof(cb_event_func) * (ET_TIMEOUT + 1));
    memset(event_func_args_, 0, sizeof(void*) * (ET_TIMEOUT + 1));

    memset(back_event_func_list_,0x0,sizeof(cb_back_event_func) * (EVENT_TIMEOUT + 1 ) );
    memset(back_event_func_args_,0x0,sizeof(void *) * (EVENT_TIMEOUT + 1 ) );

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

    TimerListItr itr = timerlist_.begin(),last = timerlist_.end();
    for(;itr != last;++itr)
    {
        delete (*itr);
    }

}

void CServerBase::ShowVerInfo()
{
    printf("\nVersion Info:%s Date:%s, (C) JinWangXinTong Technology\n\n", version_desc, __DATE__);
    return;
}

void CServerBase::run(int argc, char* argv[])
{
    if(argc < 7)
    {
        ShowVerInfo();
        printf("usage: %s comm_config private_config groupnum curgroup maxprocnum server_falg\n\n", argv[0]);
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
            printf("usage: %s comm_config private_config groupnum curgroup maxprocnum server_falg\n\n", argv[0]);
        } 
        else
        {
            printf("\ninvalid argument.\n");	
            ShowVerInfo();
            printf("usage: %s comm_config private_config groupnum curgroup maxprocnum server_falg\n\n", argv[0]);
        }

        return;
    }

    if(argc > 7)
    {
        ret = false;
    }


    //拷贝命令行参数
    ix_->argc_ = argc;
    ix_->argv_ = new char*[ix_->argc_ + 1];	
    int i;
    for(i = 0; i < ix_->argc_; ++i)
        ix_->argv_[i] = strdup(argv[i]);
    ix_->argv_[ix_->argc_] = NULL;

    startup(ret);

    realrun(argc, argv);
}
void CServerBase::startup(bool bg_run)
{
    //默认需要root权限才能setrlimit
    struct rlimit rlim;
    rlim.rlim_cur = 100000; 
    rlim.rlim_max = 100000; 
    if(setrlimit(RLIMIT_NOFILE, &rlim))
        printf("warnning: setrlimit RLIMIT_NOFILE fail\n"); 

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

int CServerBase::reg_timer_proc(int interval, cb_timer_func func, void * args /*= NULL */)
{
    if(interval <= 0 || func == NULL )
        return  -1;

    TimerInfo * pInfo = new TimerInfo;
    if(pInfo == NULL)
        return -2;

    pInfo->interval_ = interval;
    pInfo->func_ = func;
    pInfo->args_ = args;

    get_timeofday(&pInfo->proctime_);

    timerlist_.push_back(pInfo);

    return 0;
}

int CServerBase::reg_event_proc(event_type et, cb_event_func func, void * args /* = NULL*/ )
{
    if(et <= ET_TIMEOUT)
    {	
        event_func_list_[et] = func;
        event_func_args_[et] = args;
        return 0;
    }
    else
    {
        return -1;
    }
}


int CServerBase::reg_back_event_proc(back_event_type et, cb_back_event_func func, void * args/* = NULL*/)
{
    if( et <= EVENT_TIMEOUT && et >= EVENT_CONNECTED )
    {
        back_event_func_list_[et] = func;
        back_event_func_args_[et] = args;
        return 0;
    }
    return -1;
}

/*****************************************************************
//保存上下文
//key:上下文的唯一标识
//context:用户自定义上下文
//返回值:0成功，<0失败
******************************************************************/
#if 0
int CServerBase::spp_set_context(const string & key,const TContext * context)
{
    return context_queue_.AddItem(key,context);
}


/*****************************************************************
//获取上下文
//key:上下文的唯一标识
//bRemove:是否从队列中移除上下文信息
//返回值：成功：key所指向的上下文指针，失败：NULL
******************************************************************/
TContext * CServerBase::spp_get_context(const string & key,bool bRemove)
{
    return context_queue_.GetItem(key,bRemove);
}
#endif

bool CServerBase::reload()
{
    if(flag_ & RUN_FLAG_RELOAD)
    {
        flag_ &= ~RUN_FLAG_RELOAD;
        return true;
    }
    else
        return false;
}

bool CServerBase::reloadmoduleconfig()
{
    if(flag_ & RUN_FLAG_RELOAD_MODULE)
    {
        flag_ &= ~RUN_FLAG_RELOAD_MODULE;
        return true;
    }
    else
        return false;
}

bool CServerBase::quit()
{
    if(flag_ & RUN_FLAG_QUIT)
    {
        flag_ &= ~RUN_FLAG_QUIT;
        return true;
    }
    else
        return false;	
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

