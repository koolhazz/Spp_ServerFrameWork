#include "defaultctrl.h"
#include "misc.h"
#include <string.h>
#include <stdlib.h>
#include "util.h"
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
 #include <sys/file.h>
 #include <string>



#define NOTIFY_NUM   "ctrl_notify_num"

using namespace comm::lock;
using namespace comm::base;
using namespace std;
using namespace spp::ctrl;
using namespace spp::base;
using namespace spp::procmon;
using namespace spp::stat;
using namespace comm::util;

static int CvtTime(int & hour,int & minute,int & second,const string & strTime)
{
    string::size_type first = strTime.find(':');
    string::size_type last = strTime.find(':',first + 1);
    if(first == string::npos  || last == string::npos )
    {
        hour = minute = second = 0;
        return 0;
    }

    string strHour = strTime.substr(0,first);
    string strMinute = strTime.substr(first + 1,last);
    string strSecond = strTime.substr(last + 1);

    hour = atoi(strHour.c_str());
    minute = atoi(strMinute.c_str());
    second = atoi(strSecond.c_str());

    return 0;
}

//TInternal的结构要保持与serverbase.cpp中的一致
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

CDefaultCtrl::CDefaultCtrl()
{
}
CDefaultCtrl::~CDefaultCtrl()
{
}

int CDefaultCtrl::do_fork_ex(const char * basepath, const char * proc_name, const char * etcfile,const char * flag)
{
    char cmd_buf[1024] = {0};
    memset(cmd_buf,0x0,sizeof(cmd_buf));
    snprintf(cmd_buf,sizeof(cmd_buf) -1 ,"%s/%s  %s %s",basepath,proc_name,etcfile,flag);
#ifdef OPEN_PRINT
    printf("system(%s)\n",cmd_buf);
#endif
     return system(cmd_buf);
}
void CDefaultCtrl::realrun(int argc, char* argv[])
{        
    //初始化配置	
    initconf(false); //这里会加载worker

    time_t pre_stat_reset_time;//上次重置统计值的时间
    bool  today_reset_flag;//当天重置标记
    unsigned int nowtime, montime,stattime;
    char statbuff[1<<16] = {0};
    char * pstatbuff = statbuff;
    int bufflen = 0;

    stattime = montime = get_timebytask(NULL);
    pre_stat_reset_time = stattime;
    today_reset_flag = false;
    
    FW_LOG_INFO("controller started!\n");
    
    while(true)
    {
        //监控信息处理
    	monsrv_.run(); // 这里会调用监控进程进行监控, spp_proxy spp_work 利用这个特性，在初始化阶段拉起这些进程
    
        //输出系统信息
    	nowtime = get_timebytask(NULL);
    	if(unlikely(nowtime - montime > ix_->moni_inter_time_))
        {
            bufflen = sizeof(statbuff);
            monsrv_.stat(statbuff, &bufflen);
            montime = nowtime;
            FW_LOG_BIN(LOG_NORMAL, statbuff, bufflen);
        }

        //输出统计信息
        if( unlikely(nowtime - stattime > stat_output_interval_ ) )
        {
            stattime = nowtime;            
            bufflen = sizeof(statbuff);
            stat_.moni_result(&pstatbuff, &bufflen);

            if(stat_output_file_.c_str() != "" )
            {
                WriteStatResult(stat_output_file_.c_str(),pstatbuff,bufflen);
            }
            else
            {
                FW_LOG_ERR("warning!!! stat output file is empty\n");
            }

            struct tm cur_tm;
            struct tm pre_tm;
            time_t cur_time = nowtime;
            localtime_r(&cur_time, &cur_tm);
            localtime_r(&pre_stat_reset_time, &pre_tm);

            //如果超过了指定的天数，LOG当前的统计信息，并重置统计值
            if( unlikely(stat_reset_days_ <= (unsigned int) (cur_tm.tm_yday - pre_tm.tm_yday) ) )
            {
                if(!today_reset_flag && stat_reset_hour_ == cur_tm.tm_hour && stat_reset_minute_ == cur_tm.tm_min ) 
                {
                    pre_stat_reset_time = nowtime;
                    today_reset_flag = true;
                    stat_.result(&pstatbuff,&bufflen);
                    stat_.reset();
                    FW_LOG_BIN(LOG_NORMAL, pstatbuff, bufflen);
                }
                
            }
            else
            {
                today_reset_flag = false;
            }

            
        }
        
        //检查reload信号
    	if(unlikely(CServerBase::reload()))
        {    
            FW_LOG_INFO("recv reload signal\n");
            initconf(true);
            //所有被监控的进程重读配置
            monsrv_.killall(SIGUSR2);
        }

        //检查reload module config信号
    	if( unlikely(CServerBase::reloadmoduleconfig() ) )
        {
            FW_LOG_INFO("recv reload module config signal\n");
            monsrv_.killall(SIGUSR3);            
        }
        
        //检查quit信号
    	if(unlikely(CServerBase::quit()))
        {    
            FW_LOG_INFO("recv quit signal\n");
            //停止所有被监控的进程
            monsrv_.killall(SIGUSR1);
            //monsrv_.killall(SIGKILL);
            break;
        }

    	sleep(3);
    }

    FW_LOG_INFO("controller stopped!\n");
}


int CDefaultCtrl::InitProcMon(CMarkupSTL & conf, bool reload /*= false*/)
{
#ifdef OPEN_PRINT
    printf("set common config file,path_name=%s\n",ix_->argv_[1]);
#endif
    monsrv_.SetCommFilePath(ix_->argv_[1]);//设置公共配置文件路径
        
    std::vector<TGroupInfo> GroupInfoList;
    int group_num = atoi( conf.GetAttrib("groupnum").c_str() ); //spp_ctrl.xml
    int check_group_interval = atoi(conf.GetAttrib("check_group_interval").c_str() );
    if(check_group_interval != 0 )
        monsrv_.SetCheckGroupInterval(check_group_interval);
        
    assert(group_num > 0 && group_num < MAX_PROC_GROUP_NUM );
    ix_->group_num_ = group_num;
    ix_->cur_group_id_ = group_num;

    TGroupInfo groupinfo;
    int start_grp = 0,end_grp = 0,cur_grp = 0;

    while(conf.FindChildElem("group")) // 加载进程组信息
    {
    	memset(&groupinfo, 0x0, sizeof(TGroupInfo));
    	start_grp = atoi(conf.GetChildAttrib("start_group").c_str());
    	end_grp = atoi(conf.GetChildAttrib("end_group").c_str());
    	groupinfo.adjust_proc_time= 0;
    	strncpy(groupinfo.basepath_, conf.GetChildAttrib("basepath").c_str(), MAX_FILEPATH_LEN);
    	strncpy(groupinfo.exefile_, conf.GetChildAttrib("exe").c_str(), MAX_FILEPATH_LEN);
    	strncpy(groupinfo.etcfile_, conf.GetChildAttrib("etc").c_str(), MAX_FILEPATH_LEN);
    	strncpy(groupinfo.server_flag_,conf.GetChildAttrib("flag").c_str(),MAX_SERVER_FLAG_LEN);
    	groupinfo.exitsignal_ = atoi(conf.GetChildAttrib("exitsignal").c_str());

    	if(*conf.GetChildAttrib("maxprocnum").c_str())
        {
            groupinfo.maxprocnum_ = atoi(conf.GetChildAttrib("maxprocnum").c_str());
        }
    	else
        {
             groupinfo.maxprocnum_ = 100;
        }

    	if(*conf.GetChildAttrib("minprocnum").c_str())
        {
            groupinfo.minprocnum_ = atoi(conf.GetChildAttrib("minprocnum").c_str());
        }
        else
        {
            groupinfo.minprocnum_ = 1;
        }

        if(*conf.GetChildAttrib("heartbeat").c_str())
        {
            groupinfo.heartbeat_ = atoi(conf.GetChildAttrib("heartbeat").c_str());
        }
         else
         {
            groupinfo.heartbeat_ = 60;
         }
        
    	groupinfo.maxwatermark_ = atoi(conf.GetChildAttrib("watermark").c_str());
    	groupinfo.minsrate_ = atoi(conf.GetChildAttrib("minsrate").c_str());
    	groupinfo.maxdelay_ = atoi(conf.GetChildAttrib("maxdelay").c_str());
    	groupinfo.maxmemused_ = atoi(conf.GetChildAttrib("maxmemused").c_str());

    	assert(groupinfo.minprocnum_ >= 0 && groupinfo.minprocnum_ <= groupinfo.maxprocnum_ &&
            	groupinfo.exitsignal_ > 0 && groupinfo.groupid_ >= 0);

            
        assert(start_grp == cur_grp);

        if(end_grp < start_grp )
            end_grp = start_grp;

        if(end_grp >= group_num )
            end_grp = group_num -1;

        assert(start_grp <= end_grp );


        for(int idx = start_grp; idx <= end_grp;++idx)
        {
            groupinfo.groupid_ = idx;
            GroupInfoList.push_back(groupinfo);
        }

        cur_grp = end_grp + 1;
        if(cur_grp == group_num )
            break;
                
    }


        monsrv_.SetGroupNum(group_num);

        for(int idx = 0;idx < group_num;++idx ) //添加进程组的信息到procmon
        {
            memset(&groupinfo, 0x0, sizeof(TGroupInfo));
            groupinfo = GroupInfoList[idx];
            if(!reload)
            {
                monsrv_.add_group(&groupinfo);
            }
            else
            {
                monsrv_.mod_group(groupinfo.groupid_, &groupinfo);
            }
        }

        monsrv_.set_notify(notify, this);    
        return 0;

}

int CDefaultCtrl::WritePidFile(const char * pszName)
{
    const char *fname = "spp_ctrl";
    char pid_file[1024];
    char pid_buf[16];

    sprintf(pid_file, "%s.pid", fname);

    int pid_file_fd = open(pid_file, O_WRONLY | O_CREAT , S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    if(pid_file_fd < 0)
    {
        printf("Open pid file %s fail !\n", pid_file);
        return -1;
    }

    if( flock(pid_file_fd, LOCK_EX | LOCK_NB) < 0 )
    {
        return 0;
    }

    if(ftruncate(pid_file_fd, 0) < 0)
    {
        printf("truncate pid file %s fail !\n", pid_file);
        return -1;
    }

    sprintf(pid_buf, "%u\n", (int)getpid());
    if(write(pid_file_fd, pid_buf, strlen(pid_buf)) != (int)strlen(pid_buf))
    {
        printf("write pid file %s fail !\n", pid_file);
        return -1;
    }

    int val = fcntl(pid_file_fd, F_GETFD, 0);
    if( val < 0)
    {
        printf("fcntl F_GETFD pid file %s fail !\n", pid_file);
        return -1;
    }

    val |= FD_CLOEXEC;

    if(fcntl(pid_file_fd, F_SETFD, val) < 0)
    {
        printf("fcntl F_SETFD pid file %s fail !\n", pid_file);
        return -1;
    } 

    return 1;
}

int CDefaultCtrl::WriteStatResult(const char * pszName, const char * buffer, unsigned int buf_len)
{
    int file_fd = open(pszName, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    if(file_fd < 0)
    {
        printf("Open file %s fail !\n", pszName);
        return -1;
    }

    if(ftruncate(file_fd, 0) < 0)
    {
        printf("truncate file %s fail !\n", pszName);
        close(file_fd);
        return -1;
    }

    if(write(file_fd, buffer, buf_len) != (int) buf_len )
    {
        printf("write file %s fail !\n", pszName);
        close(file_fd);
        return -1;
    }

     close(file_fd);
     file_fd = 0;
    return 0;
    
}

int CDefaultCtrl::initconf(bool reload)
{
    CMarkupSTL commconf,conf;
    commconf.Load(ix_->argv_[1]);//common config file path name spp_common.xml
    conf.Load(ix_->argv_[2]); //spp_ctrl.xml
    assert(commconf.FindElem("common"));
    assert(conf.FindElem("controller"));

    if(!reload)
    {
        string strPidFile = conf.GetAttrib("pidfile");
        WritePidFile(strPidFile.c_str());
    }

    commconf.IntoElem();
    conf.IntoElem();

    //启动timer
    commconf.ResetMainPos();
    if(commconf.FindElem("timer"))
    {
        string strBasePath = commconf.GetAttrib("basepath");
        string strExeName = commconf.GetAttrib("exe");
        string strConf = commconf.GetAttrib("etc");
        string strFlag = commconf.GetAttrib("flag");
        assert(strBasePath != "");
        if(strExeName != "" )
        {
            do_fork_ex(strBasePath.c_str(),strExeName.c_str(),strConf.c_str(),strFlag.c_str()) ;
         }
    }
    
    ////////////////////////////////////////////////////////
    {
        //初始化框架日志
        commconf.ResetMainPos();
        assert(commconf.FindElem("fwlog"));
        int log_level = atoi(commconf.GetAttrib("level").c_str());
        int log_type = atoi(commconf.GetAttrib("type").c_str());
        string log_path = commconf.GetAttrib("path");

        //log前缀通过程序名+组号生成,spp_ctrl的前缀直接用程序名
        char * pLogPrefix =  strrchr(ix_->argv_[0],'/');
        assert(pLogPrefix);
        string name_prefix =pLogPrefix;

        int max_file_size = atoi(commconf.GetAttrib("maxfilesize").c_str());
        int max_file_num = atoi(commconf.GetAttrib("maxfilenum").c_str());
        
        int log_key_base = strtol(commconf.GetAttrib("key_base").c_str(),0,0 );
        int logsemkey = (log_key_base & 0xffff0000) | (0x0000ff00) ;

        //创建LOG文件夹
        MkDir(log_path.c_str());

        assert((log_level >= LOG_TRACE) && (log_level <= LOG_NONE) &&
                (log_type >= LOG_TYPE_CYCLE) && (log_type <= LOG_TYPE_CYCLE_HOURLY) &&
                (max_file_size > 0) && (max_file_size <= 1024000000) && (max_file_num > 0) );
                
    #ifdef OPEN_PRINT	        
        printf("init fwlog,log_level=%d,log_type=%d,log_path=%s,name_prefix=%s,max_file_size=%d,max_file_num=%d,log_sem_key=%u\n",
        log_level,log_type,log_path.c_str(),name_prefix.c_str(),max_file_size,max_file_num,logsemkey);
    #endif

        m_log_internal.LOG_OPEN(log_level, log_type, log_path.c_str(), name_prefix.c_str(), max_file_size, max_file_num,logsemkey);
        FW_LOG_INFO("starting Ctrl ...\n");
    }
    ////////////////////////////////////////////////////////

    ////////////////////////////////////////////////////////
    //初始化业务日志实例
    commconf.ResetMainPos();
    assert(commconf.FindElem("log"));
    int log_level = atoi(commconf.GetAttrib("level").c_str());
    int log_type = atoi(commconf.GetAttrib("type").c_str());
    string log_path = commconf.GetAttrib("path");

    //log前缀通过程序名+组号生成,spp_ctrl的前缀直接用程序名
    char * pLogPrefix =  strrchr(ix_->argv_[0],'/');
    assert(pLogPrefix);
    string name_prefix =pLogPrefix;

    int max_file_size = atoi(commconf.GetAttrib("maxfilesize").c_str());
    int max_file_num = atoi(commconf.GetAttrib("maxfilenum").c_str());
    
    int log_key_base = strtol(commconf.GetAttrib("key_base").c_str(),0,0 );
    int logsemkey = (log_key_base & 0xffff0000) | (0x0000fe00) ;

    //创建LOG文件夹
    MkDir(log_path.c_str());

    assert((log_level >= LOG_TRACE) && (log_level <= LOG_NONE) &&
            (log_type >= LOG_TYPE_CYCLE) && (log_type <= LOG_TYPE_CYCLE_HOURLY) &&
            (max_file_size > 0) && (max_file_size <= 1024000000) && (max_file_num > 0) );
            
#ifdef OPEN_PRINT	        
    printf("init log,log_level=%d,log_type=%d,log_path=%s,name_prefix=%s,max_file_size=%d,max_file_num=%d,log_sem_key=%u\n",
    log_level,log_type,log_path.c_str(),name_prefix.c_str(),max_file_size,max_file_num,logsemkey);
#endif

    log_.LOG_OPEN(log_level, log_type, log_path.c_str(), name_prefix.c_str(), max_file_size, max_file_num,logsemkey);

    ////////////////////////////////////////////////////////

    //初始化ProcMon
    conf.ResetMainPos();
    assert(conf.FindElem("procmon"));
    InitProcMon(conf,reload); //spp_ctrl.xml
    
    //初始化统计
    commconf.ResetMainPos();
    assert(commconf.FindElem("stat"));
    string mapfile = commconf.GetAttrib("mapfile");
    stat_output_file_ = commconf.GetAttrib("output");
    stat_output_interval_ = atoi(commconf.GetAttrib("interval").c_str() );
    stat_reset_days_ = atoi(commconf.GetAttrib("stat_reset_days").c_str());
    string stat_reset_time_ = commconf.GetAttrib("stat_reset_time");
    CvtTime(stat_reset_hour_,stat_reset_minute_,stat_reset_second_,stat_reset_time_);
    
    FW_LOG_DBG("map_file=%s,output_file=%s,output_interval=%d,reset_days=%d,reset_time=%d:%d:%d\n",
        mapfile.c_str(),stat_output_file_.c_str(),stat_output_interval_,stat_reset_days_,
        stat_reset_hour_,stat_reset_minute_,stat_reset_second_);   

    if(mapfile == "")
    {
        stat_.init_statpool(NULL);        
    }
    else
    {
    	stat_.init_statpool(mapfile.c_str());
    }
    
    stat_.init_statobj(NOTIFY_NUM, STAT_TYPE_SUM);

    //初始化监控
    commconf.ResetMainPos();
    assert(commconf.FindElem("moni"));
    ix_->moni_inter_time_ = atoi(commconf.GetAttrib("intervial").c_str());
    assert(ix_->moni_inter_time_ > 0);

    int moni_key_base = strtol(commconf.GetAttrib("key_base").c_str(),0,0);    
    
    key_t mqkey  = moni_key_base;
    int shmkey = moni_key_base;
    int semkey = moni_key_base;

    //共享内存的KEY,SIZE,SEMKEY初始化
    int shmsize = atoi(commconf.GetAttrib("shmsize").c_str() );

    assert(mqkey != -1 && shmkey != -1 && semkey != -1);

    CCommu* commu = new CMQCommu(mqkey);
    monsrv_.set_commu(commu);
    monsrv_.init_shm(shmkey, shmsize, semkey);

    //启动上报进程
    commconf.ResetMainPos();
    if(commconf.FindElem("report"))
    {
        string strBasePath = commconf.GetAttrib("basepath");
        string strExeName = commconf.GetAttrib("exe");
        string strConf = commconf.GetAttrib("etc");
        string strFlag = commconf.GetAttrib("flag");
        assert(strBasePath != "");
        if(strExeName != "" )
        {
            do_fork_ex(strBasePath.c_str(),strExeName.c_str(),strConf.c_str(),strFlag.c_str()) ;
         }
    }

        //启动更新配置进程
    commconf.ResetMainPos();
    if(commconf.FindElem("updateclient") )
    {
        string strBasePath = commconf.GetAttrib("basepath");
        string strExeName = commconf.GetAttrib("exe");
        string strConf = commconf.GetAttrib("etc");
        string strFlag = commconf.GetAttrib("flag");
        assert(strBasePath != "");
        if(strExeName != "" )
        {
            do_fork_ex(strBasePath.c_str(),strExeName.c_str(),strConf.c_str(),strFlag.c_str()) ;
         }
    }

    return 0;
}
void CDefaultCtrl::notify(const TGroupInfo* groupinfo, const TProcInfo* procinfo, int event, void* arg)
{
    //此处要做更多的通知信息，通过groupinfo，procinfo可以取得组信息和该进程的信息，event是发生的事件
    CDefaultCtrl* ctrl = (CDefaultCtrl*)arg;
    if(groupinfo != NULL && procinfo != NULL)    //进程事件
    {
    	ctrl->m_log_internal.LOG_P_LEVEL(LOG_NORMAL,"process warnning, groupid = %d, procid = %d, event = %d\n", 
                    	groupinfo->groupid_, procinfo->procid_, event);
    }
    else //进程组事件
    {
    	ctrl->m_log_internal.LOG_P_LEVEL(LOG_NORMAL,"group warnning, groupid = %d, event = %d\n", 
                    	groupinfo->groupid_, event);
    }

    ctrl->stat_.step0(NOTIFY_NUM, 1);
}


