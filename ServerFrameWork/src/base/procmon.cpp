#include <stdio.h>
#include <assert.h>
#include <time.h>
#include <stdlib.h>
 #include <pthread.h>
#include <sys/syscall.h>
#include <errno.h>
#include "procmon.h"
#include "misc.h"
#include "likelydef.h"
#include <errno.h>

#ifdef _PROCMON_LOG
#include "log.h"
using namespace comm::log;

CLog monlog;

#ifdef _XPRINT_
#define xprintf(fmt, args...)    monlog.LOG_P(LOG_DEBUG, fmt, ##args)
#endif

#endif

#define GROUP_UNUSED   -1

#define STAT_LEN_WARNING_STR "\n***stat data too long***\n"

using namespace spp::procmon;
using namespace comm::base;

//static bool unused = do_recv(0); //recv all message at start, discard some expired client message 


#define CUR_CONN_NUM   "cur_connect_num"  //当前连接数

CMQCommu::CMQCommu(): mqid_(0)
{
}
CMQCommu::CMQCommu(key_t mqkey): mqid_(0) 
{
    key_t mqkey_ = mqkey;
    init((void*)&mqkey_);
}
CMQCommu::~CMQCommu()
{
}
int CMQCommu::init(void* args)
{
    key_t mqkey = *((key_t *)args);
    mqid_ = msgget(mqkey, IPC_CREAT | 0666);
    if(mqid_ == -1)
    {
        printf("msgget fail,errno=%d,errmsg=%s\n",errno,strerror(errno));
    }
    assert(mqid_ != -1);
    return 0;
}
int CMQCommu::recv(TProcMonMsg* msg, long msgtype)
{
    int ret = 0;
    do 
    {
        ret = msgrcv(mqid_, (void*)msg, sizeof(TProcMonMsg), msgtype, IPC_NOWAIT);
    }while(ret == -1 && errno == EINTR);
    return ret;
}
int CMQCommu::send(TProcMonMsg* msg)
{
    int ret = 0; 
    msg->timestamp_ = get_timebytask(NULL);
    while((ret = msgsnd(mqid_, (const void*)msg, msg->msglen_, IPC_NOWAIT) != 0) 
        && (errno == EINTR));

    return ret;
}


/////////////////////////////////////////////////////////////////////////////////////////////
#define ADJUST_PROC_DELAY 120			//进程调整的延迟时间
#define ADJUST_PROC_CYCLE 1200		//进程调整周期
#define MIN_NOTIFY_TIME_CYCLE	30		//进程告警最小间隔时间

static int last_adjust_proc_time = 0;

CTProcMonSrv::CTProcMonSrv(): commu_(NULL), notify_(NULL), notify_arg_(NULL)
{
    group_num_ = 0;
    cur_group_ = 0;
    check_group_interval_ = 3 * 60;//默认3分钟
    memset(proc_groups_, 0x0, sizeof(TProcGroupObj) * MAX_PROC_GROUP_NUM);
    for(int i = 0; i < MAX_PROC_GROUP_NUM; ++i)
        proc_groups_[i].curprocnum_ = GROUP_UNUSED;

    msg_[1].msglen_ = (long)(((TProcMonMsg*)0)->msgcontent_) + sizeof(TProcInfo);
    msg_[1].srctype_ = (MSG_VERSION << 1) | MSG_SRC_SERVER; 

    //	do_recv(0); //recv all message at start, discard some expired client message 
}
CTProcMonSrv::~CTProcMonSrv()
{
    TProcGroupObj* group;
    TProcObj* proc = NULL;
    TProcObj* prev = NULL;
    for(int i = 0; i < cur_group_; ++i)
    {
        group = &proc_groups_[i];
        if(group->curprocnum_ != GROUP_UNUSED)
        {	
            for(int j = 0; j < BUCKET_SIZE; ++j)
            {
                prev = NULL;
                list_for_each_entry(proc, &group->bucket_[j], list_) 
                {
                    if(likely(prev))
                    {
                        delete prev;
                    }
                    
                    prev = proc;
                }
                if(likely(prev))
                    delete prev;
                }
            }
    }
    if(likely(commu_))
        delete commu_;
}
int CTProcMonSrv::add_group(const TGroupInfo* groupinfo)
{
    assert(cur_group_ < MAX_PROC_GROUP_NUM && cur_group_ == groupinfo->groupid_);

    TProcGroupObj* procgroup = &proc_groups_[cur_group_++];
    procgroup->curprocnum_ = 0;
    procgroup->errprocnum_ = 0;
    procgroup->last_check_group_time_ = 0;
    memcpy(&procgroup->groupinfo_, groupinfo, sizeof(TGroupInfo));		
    for(int i = 0; i < BUCKET_SIZE; ++i)
        INIT_LIST_HEAD(&(procgroup->bucket_[i]));

    return 0;
}
int CTProcMonSrv::mod_group(int groupid, const TGroupInfo* groupinfo)
{
    assert(groupid < cur_group_ && groupid == groupinfo->groupid_);

    TProcGroupObj* procgroup = &proc_groups_[groupid];
    memcpy(&procgroup->groupinfo_, groupinfo, sizeof(TGroupInfo));		

    return 0;
}
void CTProcMonSrv::set_commu(CCommu* commu)
{
    if(commu_)
        delete commu_;
    commu_ = commu;

    do_recv(0); 
}

void CTProcMonSrv::init_shm(int shmkey, int shmsize, int semkey)
{
    if( 0 == loadwriter_.init(shmkey,shmsize,semkey) )
    {
        loadwriter_.Init_LoadItem(CUR_CONN_NUM,sizeof(int));//初始化连接数负载项
    }
}

void CTProcMonSrv::SetCommFilePath(const char * comm_cfg_file)
{
    assert(comm_cfg_file);
    common_config_file_ = comm_cfg_file;

#ifdef _PROCMON_LOG

    CMarkupSTL conf;
    conf.Load(common_config_file_.c_str());
    assert(conf.FindElem("common"));
    conf.IntoElem();
    conf.ResetMainPos();
    assert(conf.FindElem("log"));
    int log_key_base = strtol(conf.GetAttrib("key_base").c_str(),0,0 );
    int logsemkey = (log_key_base & 0xffff0000) | (0x0000ff00) ;
    monlog.LOG_OPEN(LOG_TRACE, LOG_TYPE_CYCLE, "../log", "procmon", 1024000, 1,logsemkey);
#endif
    
}

void CTProcMonSrv::run()
{
    do_recv(MSG_ID_SERVER);
    do_check();
}
void CTProcMonSrv::stat(char* buf, int* len)
{
    assert(buf != NULL && len != NULL);
    int buf_len = *len;
    int ret_len;
/*	
#define PROCMON_STATUS_OK          	    0x0       //正常状态       OK
#define PROCMON_STATUS_OVERLOAD     1            //负载太大       O
#define PROCMON_STATUS_LOWSRATE     (1<<1)   //成功率过低  S
#define PROCMON_STATUS_LATENCY       (1<<2)    //延迟太大       L
#define PROCMON_STATUS_OTFMEM        (1<<3)    //使用内存太多   M
*/
    static char* proc_str[PROCMON_STATUS_OTFMEM<<1] = {"Proc Ok", "Proc O", "Proc S", "Proc OS", "Proc L", "Proc OL", "Proc SL", "Proc OSL", "Proc M", "Proc OM", "Proc SM", "Proc LM", "Proc OSM", "Proc OLM", "Proc SLM", "Proc OSLM"};
    time_t now = get_timebytask(NULL);
    struct tm tmm; 
    localtime_r(&now, &tmm);  
    TProcObj* proc = NULL;
    TProcGroupObj* group = NULL;
    TGroupInfo* groupinfo = NULL;
    TProcInfo* procinfo = NULL;
    int warning_str_len=strlen(STAT_LEN_WARNING_STR)+1;
    *len =0;
    ret_len = snprintf(buf + *len,buf_len-*len-warning_str_len,"\nTProcMonStat[%-5d], %04d-%02d-%02d %02d:%02d:%02d\n",          
            (int)syscall(__NR_gettid), tmm.tm_year + 1900, tmm.tm_mon + 1, tmm.tm_mday, tmm.tm_hour, tmm.tm_min, tmm.tm_sec);

    if(ret_len>=buf_len-*len-warning_str_len)
        goto ATAT_OVER_LENTH;
    
    *len+=ret_len;

    for(int i = 0; i < cur_group_; ++i)
    {
        group = &proc_groups_[i];
        if(group->curprocnum_ != GROUP_UNUSED)
        {	
            groupinfo = &group->groupinfo_;
            ret_len= snprintf(buf + *len,buf_len-*len-warning_str_len, "%-10s%-5u%-20s%-30s%-5u%-5u%-5u%-5u%-5u%-5u%-5u:%-5u:%-5u\n",
            		"Group", groupinfo->groupid_, groupinfo->exefile_, groupinfo->etcfile_, groupinfo->maxprocnum_, 
                    groupinfo->minprocnum_, groupinfo->heartbeat_, groupinfo->maxwatermark_, groupinfo->minsrate_, 
                    groupinfo->maxdelay_, groupinfo->maxmemused_, group->curprocnum_, group->errprocnum_);

            if(ret_len>=buf_len-*len-warning_str_len)
            goto ATAT_OVER_LENTH;

            *len+=ret_len;

            ret_len = snprintf(buf + *len,buf_len - *len - warning_str_len,"procstat | pid     | watermark | srate| delay | memused |conn_num |timestamp\n");

            if(ret_len>=buf_len-*len-warning_str_len)
                goto ATAT_OVER_LENTH;

            *len+=ret_len;

            for(int j = 0; j < BUCKET_SIZE; ++j)
            {
                list_for_each_entry(proc, &group->bucket_[j], list_) 
                {
                    procinfo = &proc->procinfo_;
                    ret_len=snprintf(buf + *len,buf_len-*len-warning_str_len, "%-10s%-15u%-8u%-8u%-8u%-8u%-8u%-20s",
                    proc_str[proc->status_], procinfo->procid_, procinfo->watermark_, procinfo->srate_, 
                    procinfo->delay_, procinfo->memused_, procinfo->curconnnum_,ctime((const time_t*)&procinfo->timestamp_)); 
                    
                    if(ret_len>=buf_len-*len-warning_str_len)
                        goto ATAT_OVER_LENTH;
                    *len+=ret_len; 
                }
            }

            ret_len=snprintf(buf + *len,buf_len-*len-warning_str_len,
                    "--------------------------------------------------------------------------------------\n");

            if(ret_len>=buf_len-*len-warning_str_len)
                goto ATAT_OVER_LENTH;
            
            *len+=ret_len;
        }
    }
    return;
    
ATAT_OVER_LENTH:
    *len=buf_len-warning_str_len-1;
    *len+=snprintf(buf+*len,warning_str_len,"%s",STAT_LEN_WARNING_STR);
    return;
}

void CTProcMonSrv::query(TProcQueryObj*& result, int& num) 
{
    if(cur_group_ <= 0)
        return;

    //调用者需要释放result分配的内存空间
    num = cur_group_;
    result = new TProcQueryObj[num];
    memset(result, 0x0, sizeof(TProcQueryObj) * num);
    TProcGroupObj* group = NULL;
    TProcObj* proc = NULL;
    TProcQueryObj* qobj = NULL;
    for(int i = 0; i < num; ++i)
    {
        qobj = &result[i];
        group = &proc_groups_[i];
        if(group->curprocnum_ != GROUP_UNUSED)
        {		
            qobj->group = group;
            qobj->proc = new TProcObj*[group->curprocnum_ + 1];
            int count = 0;
            for(int j = 0; j < BUCKET_SIZE; ++j)
            {
                list_for_each_entry(proc, &group->bucket_[j], list_) 
                {
                    if(count < group->curprocnum_)
                        qobj->proc[count++] = proc;
                    else
                        break;
                }
            }
            
            if(count <= group->curprocnum_)		//最后一个NULL作为结束标志
                qobj->proc[count] = NULL;	
        }
        else
        {
            qobj->group = NULL;
            qobj->proc = NULL;
        }
    }
}
bool CTProcMonSrv::do_recv(long msgtype)
{
    int ret = 0;
    TProcInfo* procinfo = NULL;
    TProcObj* proc = NULL;
    bool msgrecved = false;
    do
    {
        ret = commu_->recv(&msg_[0], msgtype);
        if(unlikely(ret > 0))
        {
            if((msg_[0].srctype_ & 0x1) == MSG_SRC_CLIENT)    //from client
            {	
                procinfo = (TProcInfo*)(msg_[0].msgcontent_);
#ifdef _PROCMON_LOG				
                xprintf("recv from cli: msgtype=%u,msglen=%u,msgver=%u,msgsrc=%u,ts=%u,groupid=%u,procid=%u,ts2=%u,curconnnum=%d\n", 
                        msgtype, msg_[0].msglen_, msg_[0].srctype_ >> 1, msg_[0].srctype_ & 0x1, msg_[0].timestamp_, 
                            procinfo->groupid_, procinfo->procid_, procinfo->timestamp_,procinfo->curconnnum_);
#endif

                //将连接数写入共享内存
                loadwriter_.write(CUR_CONN_NUM,(void *) &procinfo->curconnnum_,sizeof(unsigned));

                proc = find_proc(procinfo->groupid_, procinfo->procid_);	
                if(likely(proc != NULL))
                {
                    memcpy(&proc->procinfo_, procinfo, sizeof(TProcInfo));
                }
                else
                {
                    add_proc(procinfo->groupid_, procinfo);
                }

                msgrecved = true;
            }			
            else                            				//from server
            {
                //discard it				
                #ifdef _PROCMON_LOG 				
                xprintf("discard msg: msgtype=%u,msglen=%u,msgver=%u,msgsrc=%u,ts=%u\n", 
                    msgtype, msg_[0].msglen_, msg_[0].srctype_ >> 1, msg_[0].srctype_ & 0x1, msg_[0].timestamp_);
                #endif
            }
        }
    }while(ret > 0);
    
    return msgrecved;
}

bool CTProcMonSrv::check_groupbusy(int groupid)
{
    TProcGroupObj* groupobj = &proc_groups_[groupid];
    TGroupInfo* group = &(groupobj->groupinfo_);	
    TProcObj* proc = NULL;
    int  j;
    for(j = 0; j < BUCKET_SIZE; ++j)
    {
        list_for_each_entry(proc, &groupobj->bucket_[j], list_) 
        {
            if(unlikely(group->maxwatermark_ != 0 && proc->procinfo_.watermark_ > group->maxwatermark_))
            {
                //printf("group maxwatermark[%d] proc watermark[%d]", group->maxwatermark_, proc->procinfo_.watermark_);
                //printf("check group result: %d\n",(int) unlikely(group->maxwatermark_ != 0 && proc->procinfo_.watermark_ > group->maxwatermark_));
                return true;
            }
        }
    }	

    // printf("group maxwatermark[%d] proc watermark[%d]", group->maxwatermark_, proc->procinfo_.watermark_);
    // printf("check group result: %d\n",(int) unlikely(group->maxwatermark_ != 0 && proc->procinfo_.watermark_ > group->maxwatermark_));

    return false;
}
bool CTProcMonSrv::do_check()
{
    TProcObj* proc = NULL;
    TProcGroupObj* group = NULL;
    int i, j;
    for(i = 0; i < cur_group_; ++i)
    {
        group = &proc_groups_[i];
        if(group->curprocnum_ != GROUP_UNUSED)
        {
            unsigned int nowTime = get_timebytask(NULL);
            if(group->last_check_group_time_ == 0 ||  (nowTime >= (group->last_check_group_time_ + check_group_interval_ ) ) )
            {
                check_group(&group->groupinfo_, group->curprocnum_);
                group->last_check_group_time_ = nowTime;
            }           
            
            for(j = 0; j < BUCKET_SIZE; ++j)
            {
                list_for_each_entry(proc, &group->bucket_[j], list_) 
                {
                    if(unlikely(check_proc(&group->groupinfo_, &proc->procinfo_)))
                    {
                        //some proc has been deleted, go back to list head & travel again
                        --j;
                        break; 	
                    }
                }
            }
        }
    }
    return true;
}
void CTProcMonSrv::killall(int signo)
{
    TProcObj* proc = NULL;
    TProcGroupObj* group = NULL;
    int i, j;
    for(i = 0; i < cur_group_; ++i)
    {
        group = &proc_groups_[i];
        if(group->curprocnum_ != GROUP_UNUSED)
        {	
            for(j = 0; j < BUCKET_SIZE; ++j)
            {
                list_for_each_entry(proc, &group->bucket_[j], list_) 
                {
                    do_kill(proc->procinfo_.procid_, signo);
                }
            }
        }
    }
}
int CTProcMonSrv::add_proc(int groupid, const TProcInfo* procinfo)
{

    if(groupid >= cur_group_)
    {
        return -1;
    }

    TProcObj* proc = new TProcObj;	   
    memcpy(&proc->procinfo_, procinfo, sizeof(TProcInfo));
    proc->status_ = PROCMON_STATUS_OK;
    proc->notifytime_ = get_timebytask(NULL);
    INIT_LIST_HEAD(&proc->list_);

    TProcGroupObj* group = &proc_groups_[groupid];
    list_add(&proc->list_, &group->bucket_[procinfo->procid_ % BUCKET_SIZE]);	
    group->curprocnum_++;	

    return 0;
}
TProcObj* CTProcMonSrv::find_proc(int groupid, int procid)
{
    if(groupid >= cur_group_)
        return NULL;
    TProcGroupObj* group = &proc_groups_[groupid];
    int bucket = procid % BUCKET_SIZE;
    TProcObj* proc = NULL;
    list_for_each_entry(proc, &group->bucket_[bucket], list_) 
    {
        if(proc->procinfo_.procid_ == procid)
        return proc;
    }
    return NULL;
}
void CTProcMonSrv::del_proc(int groupid, int procid)
{
    if(groupid >= cur_group_)
        return;

    TProcGroupObj* group = &proc_groups_[groupid];
    int bucket = procid % BUCKET_SIZE;
    TProcObj* proc = NULL;
    list_for_each_entry(proc, &group->bucket_[bucket], list_) 
    {
        if(proc->procinfo_.procid_ == procid)
        {
            list_del(&proc->list_);
            delete proc;
            break;
        }	
    }
    group->curprocnum_--;
}
void CTProcMonSrv::check_group(TGroupInfo* group, int curprocnum)
{  
    
    int event = 0;
    int procdiff = 0;
    if(unlikely((procdiff = (int)group->minprocnum_ - curprocnum) > 0))
    {
        event |= PROCMON_EVENT_PROCDOWN;
    }
    else if(unlikely((procdiff = curprocnum - (int)group->maxprocnum_) > 0))
    {
#ifdef OPEN_PRINT
        printf("debug1: %d %d %d\n", procdiff, curprocnum, group->maxprocnum_);
#endif
        event |= PROCMON_EVENT_PROCUP;
    }
    else
    {
        //if(unlikely(((int)group->minprocnum_ < curprocnum) && (last_adjust_proc_time + ADJUST_PROC_CYCLE < time(NULL))))
        if(unlikely(((int)group->minprocnum_ < curprocnum) && (group->adjust_proc_time + ADJUST_PROC_DELAY < get_timebytask(NULL))))
        {
            if(!check_groupbusy(group->groupid_))
            {
            #ifdef OPEN_PRINT
                printf("debug2: %d %d \n", group->minprocnum_, curprocnum);
            #endif
                event |= PROCMON_EVENT_PROCUP;
                procdiff = 1;
            }
        }
    }
	
    if(unlikely(event != 0))
    {
        if(likely(notify_ != NULL))
        {
            notify_(group, NULL, event, notify_arg_);
        }    

        do_event(event, (void*)&procdiff, (void*)group);	
    }
}
bool CTProcMonSrv::check_proc(TGroupInfo* group, TProcInfo* proc)
{
    int event = 0;
    if(unlikely(group->maxwatermark_ != 0 && proc->watermark_ > group->maxwatermark_))
        event |= PROCMON_EVENT_OVERLOAD;

    if(unlikely(group->minsrate_ != 0 && proc->srate_ < group->minsrate_))
        event |= PROCMON_EVENT_LOWSRATE;

    if(unlikely(group->maxdelay_ != 0 && proc->delay_ > group->maxdelay_))
        event |= PROCMON_EVENT_LATENCY;

    if(unlikely(group->maxmemused_ != 0 && proc->memused_ > group->maxmemused_))
        event |= PROCMON_EVENT_OTFMEM;

    if(unlikely(proc->timestamp_ < (long)(get_timebytask(NULL) - group->heartbeat_) ))
    {
        //通过进程ID查找该进程是否存在，若不存在则认为进程真正死掉了
        //否则可能会误杀正在处理业务的进程
        char buf[1024] = {0};
        sprintf(buf,"/proc/%d",proc->procid_);
        if( unlikely( -1 == access(buf,R_OK)  ) )
        {        
            if(errno == 2)
            {
                event |= PROCMON_EVENT_PROCDEAD;
                #ifdef _PROCMON_LOG 
                    xprintf("CTProcMonSrv::check_proc[line=%d]:proc dead,pid=%d\n",__LINE__,proc->procid_);
                #endif
            }
        }
    }

    ((TProcObj*)proc)->status_ = PROCMON_STATUS_OK;
    if(unlikely(event != 0))
    {
        int t = get_timebytask(NULL);
        if(notify_ != NULL && ((TProcObj*)proc)->notifytime_ + MIN_NOTIFY_TIME_CYCLE < t)
        {
            notify_(group, proc, event, notify_arg_);
            ((TProcObj*)proc)->notifytime_ = t;
        }
        return do_event(event, (void*)&proc->procid_, (void*)proc);
    }
    else
    {
        return false;
    }
}
bool CTProcMonSrv::do_event(int event, void* arg1, void* arg2) // start spp_proxy spp_worker
{
    //////////////////////group event//////////////////////////
    if(event & PROCMON_EVENT_PROCDOWN)
    {
        int diff = *((int*)arg1);
        TGroupInfo* group = (TGroupInfo*)arg2;
        do_fork(group->basepath_,group->exefile_,group->etcfile_,group->groupid_,group->maxprocnum_,group->server_flag_,diff);
#ifdef _PROCMON_LOG 		
        xprintf("srv group event: event=PROCDOWN,diff=%d\n", diff);
#endif	
        last_adjust_proc_time = get_timebytask(NULL);
        group->adjust_proc_time = get_timebytask(NULL);
        return false;
    }
	
    if(event & PROCMON_EVENT_PROCUP)
    {
#ifdef _PROCMON_LOG 		
        xprintf("srv group event: event=PROCUP,diff=%u\n", *((unsigned*)arg1));
#endif
        TProcGroupObj* group = (TProcGroupObj*)arg2;
        int diff = *((int*)arg1);
        int groupid = group->groupinfo_.groupid_;
        //int signalno = group->groupinfo_.exitsignal_;
        TProcObj* proc = NULL;
        int procid = 0;
        int count = 0;
        for(int i = 0; i < BUCKET_SIZE; ++i)
        {
            list_for_each_entry(proc, &group->bucket_[i], list_) 
            {
                procid = proc->procinfo_.procid_;
                do_kill(procid, SIGUSR1);
                do_recv(procid);	
                del_proc(groupid, procid);
                count++;
                i--;
                break;
            }
            if(count >= diff)
                break;
        }

        last_adjust_proc_time = get_timebytask(NULL);
        //group->_proc_time = time(NULL);
        group->groupinfo_.adjust_proc_time = get_timebytask(NULL);

        return false;
    }
	
    /////////////////////////proc event/////////////////////////
    TProcInfo* proc = (TProcInfo*)arg2;
    ((TProcObj*)proc)->status_ = PROCMON_STATUS_OK;
    if(event & PROCMON_EVENT_PROCDEAD)
    {
#ifdef _PROCMON_LOG 		
        xprintf("srv proc event: event=PROCDEAD,procid=%u,ts=%u\n", proc->procid_, proc->timestamp_);
#endif
        TGroupInfo* group = &proc_groups_[proc->groupid_].groupinfo_;
        int signalno = group->exitsignal_;
        do_kill(proc->procid_, signalno); //send user defined signal firstly
        do_recv(proc->procid_);				      //del msg in mq
        del_proc(proc->groupid_, proc->procid_);  //del from proc list				
        do_fork(group->basepath_,group->exefile_,group->etcfile_,group->groupid_,group->maxprocnum_ ,group->server_flag_,1);
        ((TProcGroupObj*)group)->errprocnum_++;	
        return true;  //change the proc list
    }
    if(event & PROCMON_EVENT_OVERLOAD)
    {
#ifdef _PROCMON_LOG 		
        xprintf("srv proc event: event=OVERLOAD,procid=%u,ts=%u\n", proc->procid_, proc->timestamp_);
#endif
        ((TProcObj*)proc)->status_ |= PROCMON_STATUS_OVERLOAD;
        do_order(proc->groupid_, proc->procid_, PROCMON_EVENT_OVERLOAD, PROCMON_CMD_LOAD, proc_groups_[proc->groupid_].groupinfo_.maxwatermark_);

        //检查是否需要增加进程
        TProcGroupObj* group = (TProcGroupObj*)&proc_groups_[proc->groupid_].groupinfo_;
        //if((group->curprocnum_ < (int)group->groupinfo_.maxprocnum_) && (last_adjust_proc_time + ADJUST_PROC_DELAY < time(NULL)))
        //if((group->curprocnum_ < (int)group->groupinfo_.maxprocnum_) && (group->groupinfo_.adjust_proc_time + ADJUST_PROC_DELAY < time(NULL)))
        if(group->curprocnum_ < (int)group->groupinfo_.maxprocnum_ )
        {
            do_fork(group->groupinfo_.basepath_,group->groupinfo_.exefile_,group->groupinfo_.etcfile_,group->groupinfo_.groupid_,group->groupinfo_.maxprocnum_,group->groupinfo_.server_flag_,1);
            last_adjust_proc_time = get_timebytask(NULL);
            group->groupinfo_.adjust_proc_time = get_timebytask(NULL);
        }
    }


    if(event & PROCMON_EVENT_LOWSRATE)
    {
#ifdef _PROCMON_LOG 		
        xprintf("srv proc event: event=LOWSRATE,procid=%u,ts=%u\n", proc->procid_, proc->timestamp_);
#endif		
        ((TProcObj*)proc)->status_ |= PROCMON_STATUS_LOWSRATE;
        do_order(proc->groupid_, proc->procid_, PROCMON_EVENT_LOWSRATE, PROCMON_CMD_LOAD, proc_groups_[proc->groupid_].groupinfo_.maxwatermark_);
    }
    if(event & PROCMON_EVENT_LATENCY)
    {
#ifdef _PROCMON_LOG 		
        xprintf("srv proc event: event=LATENCY,procid=%u,ts=%u\n", proc->procid_, proc->timestamp_);
#endif		
        ((TProcObj*)proc)->status_ |= PROCMON_STATUS_LATENCY;
        do_order(proc->groupid_, proc->procid_, PROCMON_EVENT_LATENCY, PROCMON_CMD_LOAD, proc_groups_[proc->groupid_].groupinfo_.maxwatermark_);
    }
    if(event & PROCMON_EVENT_OTFMEM)
    {
#ifdef _PROCMON_LOG 		
        xprintf("srv proc event: event=OTFMEM,procid=%u,ts=%u\n", proc->procid_, proc->timestamp_);
#endif		
        ((TProcObj*)proc)->status_ |= PROCMON_STATUS_OTFMEM;
        do_order(proc->groupid_, proc->procid_, PROCMON_EVENT_OTFMEM, PROCMON_CMD_LOAD, proc_groups_[proc->groupid_].groupinfo_.maxwatermark_);
    }

    return false;
}	

#if 0
void CTProcMonSrv::do_fork(const char* basepath, const char* exefile, const char* etcfile, int num)
{
    char cmd_buf[256] = {0};
    for(int i = 0; i < num; ++i)
    {
        snprintf(cmd_buf, sizeof(cmd_buf) - 1, "%s/%s %s/%s", basepath, exefile, basepath, etcfile);
        system(cmd_buf);
    }
}

void CTProcMonSrv::do_fork(const char * basepath, const char * exefile, const char * etcfile, const char * serv_flag, int num)
{
    char cmd_buf[256] = {0};
    for(int i = 0;i< num;++i)
    {
        snprintf(cmd_buf,sizeof(cmd_buf) -1,"%s/%s %s/%s %s", basepath, exefile, basepath, etcfile,serv_flag);
        system(cmd_buf); 
    }
}

#endif

void CTProcMonSrv::do_fork(const char * basepath, const char * exefile, const char * etcfile, 
        int groupid, int max_procnum, const char * serv_flag, int num)
{
    char cmd_buf[1024] = {0};
    for(int i = 0;i< num;++i)
    {
        memset(cmd_buf,0x0,sizeof(cmd_buf));
        snprintf(cmd_buf,sizeof(cmd_buf) -1,"%s/%s %s %s/%s %d %d %d %s", basepath, exefile, common_config_file_.c_str(),
            basepath, etcfile,group_num_,groupid,max_procnum,serv_flag);
        system(cmd_buf); 
    }
}

void CTProcMonSrv::do_kill(int procid, int signo)
{
    char cmd_buf[64] = {0};
    snprintf(cmd_buf, sizeof(cmd_buf) - 1, "kill -%d %u > /dev/null 2>&1", signo, procid);	
    system(cmd_buf);
}
void CTProcMonSrv::do_order(int groupid, int procid, int eventno, int cmd, int arg1, int arg2)
{
    msg_[1].msgtype_ = procid;
    TProcEvent* event = (TProcEvent*)msg_[1].msgcontent_;	
    event->groupid_ = groupid;
    event->procid_ = procid;
    event->cmd_ = cmd;
    event->arg1_ = arg1;
    event->arg2_ = arg2;
    commu_->send(&msg_[1]);
}
void CTProcMonSrv::set_notify(monsrv_cb notify, void* arg)
{
    notify_ = notify;
    notify_arg_ = arg;
}
/////////////////////////////////////////////////////////////////////////////////////
CTProcMonCli::CTProcMonCli():commu_(NULL), notify_(NULL), notify_arg_(NULL)
{
    msg_[0].msgtype_ = MSG_ID_SERVER;
    msg_[0].msglen_ = (long)(((TProcMonMsg*)0)->msgcontent_) + sizeof(TProcInfo);
    msg_[0].srctype_ = (MSG_VERSION << 1) | MSG_SRC_CLIENT;
}
CTProcMonCli::~CTProcMonCli()
{
    if(commu_)
        delete commu_;
}
void CTProcMonCli::set_commu(CCommu* commu)
{
    if(commu_)
        delete commu_;
    commu_ = commu;
}
void CTProcMonCli::set_notify(moncli_cb notify, void* arg)
{
    notify_ = notify;
    notify_arg_ = arg;
}
void CTProcMonCli::run()
{
    int ret = 0;

    //send to server
    ret = commu_->send(&msg_[0]);
#ifdef _PROCMON_LOG 			
    xprintf("send to srv: msgtype=%u,msglen=%u,msgver=%u,msgsrc=%u,ts=%u,groupid=%u,procid=%u,connect_num=%u\n", 
    			msg_[0].msgtype_, msg_[0].msglen_, msg_[0].srctype_ >> 1, msg_[0].srctype_ & 0x1, msg_[0].timestamp_,CLI_SEND_INFO(this)->curconnnum_);
#endif

    //recv from server
    long msgtype = CLI_SEND_INFO(this)->procid_;
    while((ret = commu_->recv(&msg_[1], msgtype)) > 0)
    {
        if((msg_[1].srctype_ & 0x1) == MSG_SRC_SERVER)
        {		
            TProcEvent* event = (TProcEvent*)msg_[1].msgcontent_;
#ifdef _PROCMON_LOG 			
            xprintf("recv from srv: msgtype=%u,msglen=%u,msgver=%u,msgsrc=%u,ts=%u,groupid=%u,procid=%u\n", 
                msgtype, msg_[1].msglen_, msg_[1].srctype_ >> 1, msg_[1].srctype_ & 0x1, msg_[1].timestamp_,
                event->groupid_, event->procid_); 
#endif		
            assert(event->groupid_ == CLI_SEND_INFO(this)->groupid_);
            if(notify_) //使用者通过cmd，可以知道要执行什么命令，通过宏CLI_RECV_INFO可以得到命令的内容
                notify_(event->cmd_, notify_arg_);
        }
    }
}


