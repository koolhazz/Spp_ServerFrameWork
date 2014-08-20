#include <string.h>
#include "defaultproxy.h"
#include "MarkupSTL.h"
#include "misc.h"
#include "benchadapter.h"

#define ATOR_RECV_NUM   "proxy_ator_recv_num"
#define CTOR_RECV_NUM   "proxy_ctor_recv_num"
#define ATOR_CONN_NUM "proxy_ator_conn_num"

#define DEFAULT_ROUTE_NO  1

using namespace spp::base;
using namespace comm::lock;
using namespace comm::base;
using namespace spp::proxy;
using namespace spp::stat;
using namespace comm::sockcommu;
using namespace comm::commu::shmcommu;
using namespace spp::procmon;

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

struct CDefaultProxy::IProxyParams
{
    int localprocess;
};

static char SBUFFER[MAX_BLOB_DATA_LEN];//16M

CDefaultProxy::CDefaultProxy(): ator_(NULL), iplimit_(IPLIMIT_DISABLE),IParam_(NULL)
{
}
CDefaultProxy::~CDefaultProxy()
{
    if(ator_ != NULL)
        delete ator_;

    for(unsigned i = 0; i < ctor_.size(); ++i)
    {
        if(ctor_[i] != NULL)
        {
            delete ctor_[i];
        }
    }
}


void CDefaultProxy::realrun(int argc, char* argv[])
{
    //初始化配置
    initconf(false);

    FW_LOG_INFO("init conf ok!\n");

    struct timeval now;

    unsigned int nowtime, montime, stattime;
    int ctor_num = ctor_.size();   
    int i;	

    char ctor_stat[1<<16] = {0};
    int ctor_stat_len = 0;

    typedef struct tagQueryLoadInfo
    {
    	int curconn_num_;//当前连接数		
    }QueryLoadInfo;


    static QueryLoadInfo queryLoad;
    memset(&queryLoad,0,sizeof(QueryLoadInfo));
    int loadbuf_len = sizeof(QueryLoadInfo);	
    	
    stattime = get_timebytask(NULL);
    montime = stattime - ix_->moni_inter_time_ - 1; 

    FW_LOG_INFO("proxy started!\n");

    int t_tick = 0;
    bool que_idle = false;
    bool fd_idle = false;
    while(true)
    {
        //轮询acceptor
        if(ator_->poll()  != 0 )
        {
            fd_idle = true;
        }
        else
        {
            fd_idle = false;
        }

        //轮询connector
        for(i = 0; i < ctor_num; ++i)
        {
            if(ctor_[i]->poll() != 0 )
            {
                que_idle = true;
            }
            else
            {
                que_idle = false;
            }
        }

        // printf("fd_idel = %d,que_idle=%d\n",fd_idle,que_idel);

        if(ctor_num == 0 && fd_idle )
        {
            //适用于没有共享内存管道情况
            ++t_tick;
            if(t_tick >= 10 )
            {
                t_tick = 1;
                usleep(100);
            }
        }
        else if(que_idle && fd_idle )
        {
            ++t_tick;
            //轮询N次没有收到数据
            if(t_tick >= 100 )
            {
                t_tick = 1;
                usleep(10);
            }                     
        }
        else
        {
            t_tick = 1;
        }

		
        nowtime = get_timebytask(NULL);

        //统计信息输出
        if(unlikely(nowtime - montime > ix_->moni_inter_time_))
        {

            //取出当前的连接数
            ator_->ctrl(0,CT_LOAD, (void *) &queryLoad,&loadbuf_len);
            CLI_SEND_INFO(&moncli_)->curconnnum_ = queryLoad.curconn_num_;

            stat_.step0(ATOR_CONN_NUM, queryLoad.curconn_num_);

            //取上一统计周期的负载  ,目前采用固定启动进程的方式，不需要计算负载
            //TStatObjWrapper statobj;
            //stat_.query(ATOR_RECV_NUM, &statobj); 
            CLI_SEND_INFO(&moncli_)->watermark_ = 0;//STATVAL_READ(statobj.value_[0]) / ix_->moni_inter_time_;

            stattime = nowtime;

            //取动态内存使用量
            CLI_SEND_INFO(&moncli_)->memused_ = CMisc::getmemused();

            //输出ctor的统计信息
            ctor_stat_len = 0;
            ctor_stat_len += sprintf(ctor_stat, "Contector Stat\n");
            for(int i = 0; i < ctor_num; ++i)
            {
                ctor_[i]->ctrl(0, CT_STAT, ctor_stat, &ctor_stat_len);
            }

            CLI_SEND_INFO(&moncli_)->timestamp_ = get_timebytask(NULL);
            moncli_.run();
            montime = nowtime;
            FW_LOG_DBG("moncli run!\n");

        }

        //回调注册的定时器
        get_timeofday(&now, NULL);

        do_timer_callback( now );

		
        //检查reload信号
        if(unlikely(CServerBase::reload()))
        {	
            FW_LOG_INFO("recv reload signal\n");
            initconf(true);
        }

        //检查业务组件的reloadconfig信号
        if(unlikely(CServerBase::reloadmoduleconfig()))
        {
            FW_LOG_INFO("recv reload module config signal!\n");
            if(sppdll.spp_handle_reloadconfig != NULL )
            {
                CMarkupSTL conf;
                conf.Load(ix_->argv_[2]);
                assert(conf.FindElem("proxy"));
                conf.IntoElem();
                assert(conf.FindElem("module"));
                string module_etc = conf.GetAttrib("etc");
				
                blob_type blob;
                blob.data = SBUFFER;
                blob.extdata = NULL;
                blob.owner = (void *) ator_;
                sppdll.spp_handle_reloadconfig((void *) module_etc.c_str(),&blob,this);

                FW_LOG_INFO("recv reload module config signal\n");
            }
        }
		
		
        //检查quit信号
        if(unlikely(CServerBase::quit()))
        {	
            FW_LOG_INFO("recv quit signal\n");
            break;	
        }
    }
	
    if(sppdll.spp_handle_fini != NULL)
        sppdll.spp_handle_fini(NULL, this);

    FW_LOG_INFO("proxy stopped!\n");
}
int CDefaultProxy::initconf(bool reload)
{
    CMarkupSTL commconf,conf;
    commconf.Load(ix_->argv_[1]);
    conf.Load(ix_->argv_[2]);

    assert(commconf.FindElem("common"));
    assert(conf.FindElem("proxy"));

	//./spp_proxy ../etc/spp_common.xml ./../etc/spp_proxy.xml 2 0 1 SVR_TaoGu_Proxy_Proxy_Flag
	//./spp_worker ../etc/spp_common.xml ./../etc/spp_worker.xml 2 1 2 SVR_TaoGu_Proxy_Work_Flag
    ix_->group_num_ = atoi(ix_->argv_[3]);
    ix_->cur_group_id_ = atoi(ix_->argv_[4]);
    ix_->max_proc_num_ = atoi(ix_->argv_[5]);

    assert(ix_->cur_group_id_ == 0 );
	
    conf.IntoElem();
    commconf.IntoElem();

    ////////////////////////////////////////////////////////
    //初始化框架日志实例
    {
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
        int semkey = log_key_base;

        assert((log_level >= LOG_TRACE) && (log_level <= LOG_NONE) &&
        		(log_type >= LOG_TYPE_CYCLE) && (log_type <= LOG_TYPE_CYCLE_HOURLY) &&
        		(max_file_size > 0) && (max_file_size <= 1024000000) && (max_file_num > 0));

        m_log_internal.LOG_OPEN(log_level, log_type, log_path.c_str(), name_prefix.c_str(), max_file_size, max_file_num,semkey);

        FW_LOG_INFO("Proxy start!!!\n");
    }
    //////////////////////////////////////////////////////////////////////

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
    int semkey = log_key_base;

    assert((log_level >= LOG_TRACE) && (log_level <= LOG_NONE) &&
    		(log_type >= LOG_TYPE_CYCLE) && (log_type <= LOG_TYPE_CYCLE_HOURLY) &&
    		(max_file_size > 0) && (max_file_size <= 1024000000) && (max_file_num > 0));

    log_.LOG_OPEN(log_level, log_type, log_path.c_str(), name_prefix.c_str(), max_file_size, max_file_num,semkey);
    //////////////////////////////////////////////////////////////////////


    //acceptor配置	
    if(!reload)
    {
        conf.ResetMainPos();
        assert(conf.FindElem("acceptor"));

        string type = conf.GetAttrib("type");
        assert(type == "socket");
        TSockCommuConf socks;
        memset(&socks, 0x0, sizeof(TSockCommuConf));
        socks.maxconn_ = atoi(conf.GetAttrib("maxconn").c_str());
        socks.maxpkg_ = atoi(conf.GetAttrib("maxpkg").c_str());
        socks.expiretime_ = atoi(conf.GetAttrib("timeout").c_str());	
        socks.check_expire_interval_ = atoi(conf.GetAttrib("check_expire_interval").c_str());//检查连接超时时间间隔,0表示不检查

        assert(socks.maxconn_ > 0 && socks.maxpkg_ >= 0 && socks.expiretime_ >= 0 && socks.check_expire_interval_ >= 0);

        int i = 0;
		while(conf.FindChildElem("entry"))//对应spp_proxy.xml <entry type="tcp" if="eth1" port="8090" />
        {
            type = conf.GetChildAttrib("type");
            if(type == "tcp")
            {
                socks.sockbind_[i].type_ = SOCK_TYPE_TCP;
                socks.sockbind_[i].ipport_.ip_ = CMisc::getip(conf.GetChildAttrib("if").c_str());
                socks.sockbind_[i].ipport_.port_ = atoi(conf.GetChildAttrib("port").c_str());			
            }
            else if(type == "udp")
            {
                socks.sockbind_[i].type_ = SOCK_TYPE_UDP;
                socks.sockbind_[i].ipport_.ip_ = CMisc::getip(conf.GetChildAttrib("if").c_str());
                socks.sockbind_[i].ipport_.port_ = atoi(conf.GetChildAttrib("port").c_str());			
            }
            else if(type == "unix")
            {
                socks.sockbind_[i].type_ = SOCK_TYPE_UNIX;
                strncpy(socks.sockbind_[i].path_, conf.GetChildAttrib("path").c_str(), 256);
            }
            else
            {
                assert(false);
            }
            
            i++;
        }

        //至少有一个entry
        assert(i > 0);

        ator_ = new CTSockCommu; //初始化接入器
        assert(ator_->init(&socks) == 0);
        //ator_->reg_cb(CB_RECVDATA, ator_recvdata, this); //延迟注册	
        ator_->reg_cb(CB_OVERLOAD, ator_overload, this);	
        ator_->reg_cb(CB_DISCONNECT, ator_disconn, this);	
        ator_->reg_cb(CB_TIMEOUT, ator_timeout, this);
        ator_->reg_cb(CB_CONNECTED, ator_connected, this);

    }

    if(!reload)
    {
        //connector配置
        conf.ResetMainPos();
        if(conf.FindElem("connector"))
        {
            string type = conf.GetAttrib("type");

            if(type == "shm")
            {	
                commconf.ResetMainPos();
                assert(commconf.FindElem("ShmQueue"));
                int send_size = atoi(commconf.GetAttrib("send_size").c_str());
                int recv_size = atoi(commconf.GetAttrib("recv_size").c_str());
                int shm_queue_key_base = strtol(commconf.GetAttrib("key_base").c_str(),0,0 );          
                

                vector<TShmCommuConf> shms;
                for(unsigned int idx = 0;idx< ix_->group_num_ -1;++idx) //根据group_num_ 确定生产消费队列的个数
                {
                    TShmCommuConf shm;             
                    shm.shmkey_producer_ = shm_queue_key_base + 2*idx;//内存管道的key与组相关
                    shm.shmsize_producer_ = send_size * (1<<20); // 16m  spp_proxy.xml->ShmQueue                
                    shm.shmkey_comsumer_ = shm_queue_key_base + 2*idx + 1;
                    shm.shmsize_comsumer_ = recv_size * (1<<20);
                    shm.msg_timeout_ = 0;
                    shm.locktype_ = 0; //proxy只有一个进程，读写都不需要加锁
                    shm.expiretime_ = 0;	//不需要超时检查，由acceptor负责超时检查
                    shm.maxpkg_ = 0;
                    shms.push_back(shm);
                }

                CTCommu* commu = NULL;
                for(unsigned i = 0; i < shms.size(); ++i)
                {
                    commu = new CTShmCommu;
                    assert(commu->init(&shms[i]) == 0);
                    commu->reg_cb(CB_RECVDATA, ctor_recvdata, this);
                    ctor_.push_back(commu);
                }
                //至少要有一个ctor
                //assert(ctor_.size() > 0);
                if(ctor_.size() == 0 )
                {
                    FW_LOG_INFO("warning!!!!ctor size=0\n");
                }
            }
            else if(type == "socket")
            {
            //...
            }

        }


    }


    //初始化统计
    commconf.ResetMainPos();
    assert(commconf.FindElem("stat"));
    string mapfile = commconf.GetAttrib("mapfile");
    if(mapfile == "")
        stat_.init_statpool(NULL);    
    else
        stat_.init_statpool(mapfile.c_str());

    stat_.init_statobj(ATOR_RECV_NUM, STAT_TYPE_SUM);
    stat_.init_statobj(CTOR_RECV_NUM, STAT_TYPE_SUM);
    stat_.init_statobj(ATOR_CONN_NUM,STAT_TYPE_UPDATE);

    //初始化监控
    commconf.ResetMainPos();
    assert(commconf.FindElem("moni"));
    ix_->moni_inter_time_ = atoi(commconf.GetAttrib("intervial").c_str());
    assert(ix_->moni_inter_time_ > 0);

    int moni_key_base = strtol(commconf.GetAttrib("key_base").c_str(),0,0);
    key_t mqkey  = moni_key_base;    

    assert(mqkey != -1);
	
    CCommu* commu = new CMQCommu(mqkey);
    moncli_.set_commu(commu);
    memset(CLI_SEND_INFO(&moncli_), 0x0, sizeof(TProcInfo));
    CLI_SEND_INFO(&moncli_)->groupid_ = ix_->cur_group_id_;
    CLI_SEND_INFO(&moncli_)->procid_ = getpid();

    //加载用户服务模块
    conf.ResetMainPos();
    assert(conf.FindElem("module"));
    string module_file = conf.GetAttrib("bin");
    string module_etc = conf.GetAttrib("etc");
    FW_LOG_INFO("load module %s, %s\n", module_file.c_str(), module_etc.c_str());
    if(0 == load_bench_adapter(module_file.c_str()))//加载用户定义库的内容
    {
        assert(sppdll.spp_handle_init((void*)module_etc.c_str(), this) == 0);
        ator_->reg_cb(CB_RECVDATA, ator_recvdata, this);	//数据接收回调注册
    }

    //注册网络事件回调
    ator_->reg_cb(CB_CONNECTED,ator_connected,this);
    ator_->reg_cb(CB_OVERLOAD,ator_overload,this);
    ator_->reg_cb(CB_DISCONNECT,ator_disconn,this);
    ator_->reg_cb(CB_TIMEOUT,ator_timeout,this);
	

    FW_LOG_INFO("load module ok!\n");


    string local_handle_name=conf.GetAttrib("local_handle");
    if(local_handle_name!="")
    {
        assert(sppdll.handle!=NULL);
        local_handle=(spp_handle_local_process_t)(dlsym(sppdll.handle,local_handle_name.c_str())); 
        assert(local_handle!=NULL);
    }
    else
    {
        local_handle=NULL;
    }

    
    //iptable配置
    iplimit_ = IPLIMIT_DISABLE;
    iptable_.clear();
    conf.ResetMainPos();
    if(conf.FindElem("iptable"))
    {
        string iptable = "";
        if((iptable = conf.GetAttrib("whitelist")) != "")
        {
            iplimit_ = IPLIMIT_WHITE_LIST;
            string black_table=conf.GetAttrib("blacklist");
            if(black_table!="")
            {
                printf("WARNNING:white iptable already loaded,black iptable is invalid\n");
                FW_LOG_INFO("white iptable already loaded,black iptable is invalid  \n");		
            }
        }
        else if((iptable = conf.GetAttrib("blacklist")) != "")
        {
            iplimit_ = IPLIMIT_BLACK_LIST;	
        }
        //read iplist
        if(iplimit_ != IPLIMIT_DISABLE)
        {
            FILE* fp = fopen(iptable.c_str(), "r");
            if(fp)
            {
                char line[32] = {0};
                struct in_addr ipaddr;
                while(fgets(line, 31, fp) != NULL)
                {
                    if(inet_aton(line, &ipaddr))
                        iptable_.insert(ipaddr.s_addr);
                }

                fclose(fp);
            }

            if(iptable_.size())
            {
                FW_LOG_INFO("load iptable %d\n", iptable_.size());
            }
            else
            {
                iplimit_ = IPLIMIT_DISABLE;
            }
        }
    }
    return 0;
}


int CDefaultProxy::spp_get_group_num()
{
    return ix_->group_num_;
}

int CDefaultProxy::spp_get_group_id()
{
    return ix_->cur_group_id_;
}

void CDefaultProxy::do_timer_callback(const struct timeval & nowtime )
{
    int timediff = 0;

    TimerListItr itr = timerlist_.begin(),last = timerlist_.end();	
    for(;itr != last;itr++)
    {
        timediff = (nowtime.tv_sec - (*itr)->proctime_.tv_sec) * 1000 + (nowtime.tv_usec - (*itr)->proctime_.tv_usec ) / 1000;
        if( (*itr)->interval_ <= timediff )	
        {
            blob_type sendblob;
            sendblob.len = 0;
            sendblob.data = SBUFFER;
            sendblob.owner =  (CTCommu*) ator_;
            sendblob.extdata = NULL;

            (*(*itr)->func_)(&sendblob,this,(*itr)->args_);
            (*itr)->proctime_ = nowtime;
        }
    }
	

}


//一些回调函数
//从共享内存获取数据后发送给请求方
int CDefaultProxy::ctor_recvdata(unsigned flow, void* arg1, void* arg2)
{
    blob_type* blob = (blob_type*)arg1;
    CDefaultProxy* proxy = (CDefaultProxy*)arg2;
    proxy->m_log_internal.LOG_P_LEVEL(LOG_DEBUG, "CDefaultProxy ctor recvdata, %u,%d\n", flow, blob->len);
    proxy->stat_.step0(CTOR_RECV_NUM, 1);

    //此处应该判断回包的完整性
    int ret = proxy->ator_->sendto(flow, arg1, arg2);//数据发送给请求方
    if(unlikely(ret < 0))
    	proxy->m_log_internal.LOG_P_LEVEL(LOG_ERROR, "CDefaultProxy ator sendto error, %u,%d,%d\n", flow, blob->len,ret);

    return 0;
}



int CDefaultProxy::ator_recvdata(unsigned flow, void* arg1, void* arg2)
{
    blob_type* blob = (blob_type*)arg1;
    CDefaultProxy* proxy = (CDefaultProxy*)arg2;
    proxy->m_log_internal.LOG_P_LEVEL(LOG_DEBUG, "CDefaultProxy ator recvdata v2, flow=%u,len=%d\n", flow, blob->len);
    int total_len=blob->len;
    int processed_len=0;
    int proto_len = -1; 
    int ret=0;
    
    while(blob->len>0&&(proto_len= sppdll.spp_handle_input(flow, arg1, arg2))>0)
    {
        if(proto_len > blob->len)
        {
            proxy->m_log_internal.LOG_P_LEVEL(LOG_ERROR, "CDefaultProxy spp_handle_input error, flow=%u,len=%d,proto_len=%d\n", flow, blob->len, proto_len);
            processed_len = total_len;//丢弃这些错误的数据
            break;
        }

        DELAY_INIT;
        	
        ret = 0;

        proxy->stat_.step0(ATOR_RECV_NUM, 1);
        processed_len+=proto_len;

        blob_type sendblob = *blob;
        sendblob.len = proto_len;

        proxy->m_log_internal.LOG_P_LEVEL(LOG_DEBUG,"CDefaultProxy ator_recvdata_v2,blob->len=%d,blob->ext_len=%d,blob->ext_type=%d\n",sendblob.len,sendblob.ext_len,sendblob.ext_type);

        if(proxy->local_handle)
        {
            //直接在proxy中处理
            sendblob.len = proto_len;
            sendblob.data = SBUFFER;
            memcpy(sendblob.data , blob->data, proto_len);
            sendblob.owner =  (CTCommu*) proxy->ator_;
            
            proxy->m_log_internal.LOG_P_LEVEL(LOG_DEBUG,"CDefaultProxy local process,data len =%d\n",sendblob.len);

            //本地处理，不需要发送给后端处理
            //加入超时监控
            //DELAY_INIT;
            ret = proxy->local_handle(flow,(void *) &sendblob,arg2);

            proxy->m_log_internal.LOG_P_LEVEL(LOG_DEBUG,"CDefaultProxy local handle,ret =%d\n",ret);
            if(likely(!ret))
            {
            	//成功
            }
            else
            {
                //返回负值，表示处理失败，正值表示成功且主动关闭连接				
                CTCommu* commu = (CTCommu*)blob->owner;
                blob_type rspblob;
                rspblob.len = 0;
                rspblob.data = NULL;
                commu->sendto(flow, &rspblob, arg2);
                proxy->m_log_internal.LOG_P_LEVEL(LOG_DEBUG, "CDefaultProxy close conn, flow=%u\n", flow);
                break;//连接已关闭，在缓冲中的数据已被丢掉，必须跳出循环
            }

        }
        else
        {
            unsigned int route_no;
            if(!sppdll.spp_handle_route)
            {
                route_no=1;//默认路由到Group1
            }
            else  
            {
                route_no = sppdll.spp_handle_route(flow, &sendblob, arg2);
                if(likely(route_no>proxy->ctor_.size() ||route_no <=0))
                {
                    proxy->m_log_internal.LOG_P_LEVEL(LOG_ERROR, "CDefaultProxy route error, flow=%u,len=%d,proto_len=%d,route_no=%d\n", flow, sendblob.len, proto_len, route_no);
                    break;
                }
            }

            proxy->m_log_internal.LOG_P_LEVEL(LOG_DEBUG, "CDefaultProxy ator recvdone, flow=%u,len=%d,proto_len=%d,route_no=%d\n", flow, sendblob.len, proto_len, route_no);

            proxy->m_log_internal.LOG_P_LEVEL(LOG_DEBUG,"CDefaultProxy ctor sendto,flow=%u,route_no=%d,blob_len=%d,ext_len=%d\n",flow,route_no,sendblob.len,sendblob.ext_len);
            //通过后端通讯组件将包投递给后端进程处理
            ret = proxy->ctor_[route_no - 1]->sendto(flow, &sendblob, arg2);
            if(unlikely(ret))
            {
                proxy->m_log_internal.LOG_P_LEVEL(LOG_ERROR, "CDefaultProxy ctor sendto error, route_no=%d,ret=%d\n", route_no-1,ret);
                if(ret == COMMU_ERR_MQFULL )
                {
                    //共享内存管道满了
                    ret = sppdll.spp_handle_queue_full(flow,route_no - 1,&sendblob,arg2);

                    if(ret == 0 )
                    {
                        break;//数据继续保留在内存中
                    }
                    else if(ret < 0 )
                    {
                        //返回负值，表示需要主动关闭连接				
                        CTCommu* commu = (CTCommu*)blob->owner;
                        blob_type rspblob;
                        rspblob.len = 0;
                        rspblob.data = NULL;
                        commu->sendto(flow, &rspblob, arg2);
                        proxy->m_log_internal.LOG_P_LEVEL(LOG_DEBUG, "%s:call spp_handle_queue_full() ,ret=%d,close connection, flow=%u\n",__FUNCTION__,ret, flow);
                        break;//连接已关闭，在缓冲中的数据已被丢掉，必须跳出循环
                    }
                        
                }
                
            }

        }

        proxy->m_log_internal.LOG_P_LEVEL(LOG_DEBUG,"blob len=%d,prot_len=%d\n",blob->len,proto_len);
        blob->data+=proto_len;
        blob->len-=proto_len;

    }

    proxy->m_log_internal.LOG_P_LEVEL(LOG_DEBUG,"processed_len=%d,prot_len=%d\n",processed_len,proto_len);
    if(proto_len < 0)
        return proto_len;
    
    return processed_len;
}

int CDefaultProxy::ator_overload(unsigned flow, void* arg1, void* arg2)
{
    blob_type* blob = (blob_type*)arg1;
    CDefaultProxy* proxy = (CDefaultProxy*)arg2;
    proxy->m_log_internal.LOG_P_LEVEL(LOG_ERROR, "CDefaultProxy proxy overload,len=%d\n", (long)blob->len);

    if(proxy->event_func_list_[ET_OVERLOAD] != NULL )
        proxy->event_func_list_[ET_OVERLOAD](flow,arg1,arg2,proxy->event_func_args_[ET_OVERLOAD]);


    if( sppdll.spp_handle_event_route != NULL )
    {
        blob->ext_type = EXT_TYPE_OVERLOAD;
        blob->ext_len = 0;
        blob->extdata = NULL;
        int route_no = -1;
        route_no = sppdll.spp_handle_event_route(flow,ET_OVERLOAD,arg1,arg2);
        if( route_no > 0 && route_no <=(int) proxy->ctor_.size() )
        {
            proxy->ctor_[route_no-1]->sendto(flow, arg1,arg2);
        }
        else if(route_no == 0 )
        {
            for(unsigned int i = 0; i< proxy->ctor_.size(); ++i)
            {
                proxy->m_log_internal.LOG_P_LEVEL(LOG_DEBUG,"CDefaultProxy ator_overload,route_no=%u\n",i);
                proxy->ctor_[i]->sendto(flow, arg1,arg2);
            }
        }
        else
        {
            proxy->m_log_internal.LOG_P_LEVEL(LOG_ERROR,"CDefaultProxy ator_overload,route no error!\n");
        }
    }

    return 0;
}

int CDefaultProxy::ator_connected(unsigned flow, void* arg1, void* arg2)
{
    blob_type* blob = (blob_type*)arg1;
    CDefaultProxy* proxy = (CDefaultProxy*)arg2;
    TConnExtInfo* extinfo = (TConnExtInfo*)blob->extdata;

    if(proxy->iplimit_ == IPLIMIT_WHITE_LIST)
    {
        if(proxy->iptable_.find(extinfo->remoteip_) == proxy->iptable_.end())
        {
            proxy->m_log_internal.LOG_P_LEVEL(LOG_ERROR, "ip white limited, %s\n", inet_ntoa(*((struct in_addr*)&extinfo->remoteip_)));
            return -1;	
        }
    }
    else if(proxy->iplimit_ == IPLIMIT_BLACK_LIST)
    {
        if(proxy->iptable_.find(extinfo->remoteip_) != proxy->iptable_.end())
        {
            proxy->m_log_internal.LOG_P_LEVEL(LOG_ERROR, "ip black limited, %s\n", inet_ntoa(*((struct in_addr*)&extinfo->remoteip_)));
            return -2;
        }
    }

    proxy->m_log_internal.LOG_P_LEVEL(LOG_DEBUG,"flow=%u,remote ip=%s,remote port=%d\n",flow,inet_ntoa(*((struct in_addr*)&extinfo->remoteip_)),extinfo->remoteport_);
    if(proxy->event_func_list_[ET_CONNECTED] != NULL ) 
        proxy->event_func_list_[ET_CONNECTED](flow,arg1,arg2,proxy->event_func_args_[ET_CONNECTED]);

    if( sppdll.spp_handle_event_route != NULL )
    {
        blob->ext_type = EXT_TYPE_CONNECTED;
        blob->ext_len = 0;
        blob->extdata = NULL;
        int route_no = -1;
        route_no = sppdll.spp_handle_event_route(flow,ET_CONNECTED,arg1,arg2);
        if( route_no > 0 && route_no <=(int) proxy->ctor_.size() )
        {
            proxy->ctor_[route_no-1]->sendto(flow, arg1,arg2);//放入消息对列中
        }
        else if(route_no == 0 )
        {
            for(unsigned int i = 0; i< proxy->ctor_.size(); ++i)
            {
                proxy->m_log_internal.LOG_P_LEVEL(LOG_DEBUG,"CDefaultProxy ator_connected,route_no=%u\n",i);
                proxy->ctor_[i]->sendto(flow, arg1,arg2);
            }
        }
        else
        {
            proxy->m_log_internal.LOG_P_LEVEL(LOG_ERROR,"CDefaultProxy ator_connected,route_no error!\n");
        }
    }

    return 0;
}

int CDefaultProxy::ator_timeout(unsigned flow, void* arg1, void* arg2)
{
    CDefaultProxy* proxy = (CDefaultProxy*)arg2;
    proxy->m_log_internal.LOG_P_LEVEL(LOG_DEBUG, "CDefaultProxy ator timeout, %u\n", flow);

    if(proxy->event_func_list_[ET_TIMEOUT] != NULL ) 
        proxy->event_func_list_[ET_TIMEOUT](flow,arg1,arg2,proxy->event_func_args_[ET_TIMEOUT]);

    if( sppdll.spp_handle_event_route != NULL )
    {
        blob_type * blob = (blob_type *) arg1;
        blob->ext_type = EXT_TYPE_TIMEOUT;
        blob->ext_len = 0;
        blob->extdata = NULL;
	        
        int route_no = -1;
        route_no = sppdll.spp_handle_event_route(flow,ET_TIMEOUT,arg1,arg2);
        if( route_no > 0 && route_no <=(int) proxy->ctor_.size() )
        {
            proxy->ctor_[route_no-1]->sendto(flow, arg1,arg2);
        }
        else if(route_no == 0 )
        {
            for(unsigned int i = 0; i< proxy->ctor_.size(); ++i)
            {
                proxy->m_log_internal.LOG_P_LEVEL(LOG_DEBUG,"CDefaultProxy ator_timeout,route_no=%u\n",i);
                proxy->ctor_[i]->sendto(flow, arg1,arg2);
            }
        }
        else
        {
            proxy->m_log_internal.LOG_P_LEVEL(LOG_ERROR,"CDefaultProxy ator_timeout,route_no error!\n");
        }
    }

    return 0;
}


int CDefaultProxy::ator_disconn(unsigned flow, void * arg1, void * arg2)
{
    CDefaultProxy * proxy = (CDefaultProxy *) arg2;
    proxy->m_log_internal.LOG_P_LEVEL(LOG_DEBUG,"CDefaultProxy ator disconnect,flow=%u\n",flow);

    if(proxy->event_func_list_[ET_DISCONNECT] != NULL ) 
    {
        proxy->event_func_list_[ET_DISCONNECT](flow,arg1,arg2,proxy->event_func_args_[ET_DISCONNECT]);
    }
    else
    {
        proxy->m_log_internal.LOG_P_LEVEL(LOG_NORMAL,"%s,proxy->event_func_list_[ET_DISCONNECT] is null\n",__FUNCTION__);
    }


    //通知WORKER网络连接断开
    if( sppdll.spp_handle_event_route != NULL )
    {
        blob_type * blob = (blob_type *) arg1;
        blob->ext_type = EXT_TYPE_DISCONNECT;
        blob->ext_len = 0;
        blob->extdata = NULL;
	        
        int route_no = -1;
        route_no = sppdll.spp_handle_event_route(flow,ET_DISCONNECT,arg1,arg2);
        if( route_no > 0 && route_no <=(int) proxy->ctor_.size() )
        {
            proxy->ctor_[route_no-1]->sendto(flow, arg1,arg2);
        }
        else if(route_no == 0 )
        {
            for(unsigned int i = 0; i< proxy->ctor_.size(); ++i)
            {
                proxy->m_log_internal.LOG_P_LEVEL(LOG_DEBUG,"CDefaultProxy ator_disconn,route_no=%u\n",i);
                proxy->ctor_[i]->sendto(flow, arg1,arg2);
            }
        }
        else
        {
            proxy->m_log_internal.LOG_P_LEVEL(LOG_ERROR,"CDefaultProxy ator_disconn,route_no error!\n");
        }
    }
    else
    {
        proxy->m_log_internal.LOG_P_LEVEL(LOG_NORMAL,"%s,sppdll.spp_handle_event_route is null\n",__FUNCTION__);
    }
    

    return 0;	
}


