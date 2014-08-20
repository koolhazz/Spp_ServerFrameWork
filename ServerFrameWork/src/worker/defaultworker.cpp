#include "defaultworker.h"
#include "MarkupSTL.h"
#include "misc.h"
#include "benchadapter.h"
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include "socket.h"
#include <stdio.h>
#include <stdlib.h>
#include "connset.h"
#include "sockcommu.h"
#include "hexdump.h"

#define ATOR_RECV_NUM   "work_ator_recv_num"
#define ATOR_SEND_NUM   "work_ator_send_num"
#define AVG_PROC_TIME	    "work_avg_proc_time"
#define MAX_PROC_TIME    "work_max_proc_time"
#define FAIL_PROC_NUM     "work_fail_proc_num"

#define CTOR_RECV_NUM   "work_ctor_recv_num"
#define CTOR_SEND_NUM   "work_ctor_send_num"

using namespace comm::util;
using namespace spp::base;
using namespace comm::lock;
using namespace comm::base;
using namespace std;
using namespace spp::worker;
using namespace comm::commu;
using namespace comm::sockcommu;
using namespace comm::commu::shmcommu;
using namespace spp::procmon;


static char SBUFFER[MAX_BLOB_DATA_LEN];
///////////////////////////////////////////////
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

static int AllTrim(string & str)
{
    str.erase( std::remove(str.begin(), str.end(), ' '), str.end());
    return 0;
}

CDefaultWorker::CDefaultWorker(): ator_(NULL),ctor_(NULL),route_config_(NULL)
{
}
CDefaultWorker::~CDefaultWorker()
{
    if(ator_ != NULL)
        delete ator_;
    
    if(ctor_ != NULL)
        delete ctor_;
    
    if(route_config_ != NULL )
        delete route_config_;
}

int CDefaultWorker::spp_sendto(unsigned int& to_flow, unsigned int serv_type, unsigned int route_key,void * arg1, void * arg2)
{
    if(route_config_ == NULL || ctor_ == NULL )
    {
        FW_LOG_ERR("route table  error!!\n");
        return -1;
    }

    TRouteTable tbl;
    int ret = route_config_->GetRouteTable(tbl,serv_type);//加载../dat/route_table/*.xml
    if(ret != 0 )
    {
        FW_LOG_ERR("get route table fail!!!!serv_type=%u\n",serv_type);
        return -2;
    }
        
    int node_key = 0;
    if(tbl.route_ == "%" )
    {
        node_key = route_key % tbl.route_val_;
    }
    else if(tbl.route_ == "/" )
    {
        node_key = route_key / tbl.route_val_;
    }

    unsigned int node_id = tbl.GetNodeID(node_key);

    FW_LOG_DBG("serv_type=%u,node_key=%d,node_id=%d\n",serv_type,node_key,node_id);

    if(node_id == 0 )
    {
        FW_LOG_ERR("call GetNodeID() return fail\n");
        return -3;
    }

    to_flow = commu_mng_.GetFlow(serv_type,node_id);
    if(to_flow > 0 )
    {
        ret = ctor_->sendto(to_flow, arg1, arg2);
        FW_LOG_DBG("send to:serv_type=%d,node_id=%d,flow=%d,ret=%d\n",serv_type,node_id,to_flow,ret);
        
        if( ret  == -E_NOT_FINDFD || ret == -E_NEED_CLOSE )
        {
            //连接未建立或已经断开,重新建立连接
            CTSockCommu * commu = (CTSockCommu * ) ctor_;
            //reconnect
            TNodeInfo NodeItem;
            if( 0 == tbl.GetNodeInfo(NodeItem,node_id) )
            {
                //先保存buffer内容，因为在回调时可能会破坏buff
                string strSendData;
                blob_type * blob = (blob_type *) arg1;
                strSendData.assign((char *) blob->data,blob->len);
                int retVal = commu->connect(&NodeItem);
                if( 0 == retVal )
                {
                    to_flow = commu_mng_.GetFlow(serv_type,node_id);

                    if(to_flow > 0 )
                    {
                        //恢复buffer内容
                        blob->len = strSendData.size();
                        memcpy(blob->data,strSendData.data(),blob->len );                                            
                        ret = ctor_->sendto(to_flow, arg1, arg2); 
                    }
                    else
                    {
                        FW_LOG_ERR("Reconnect fail!!!\n");
                        return -5;
                    }
                }
                else
                {
                    FW_LOG_ERR("ReConnect Fail,retval=%d\n",retVal);
                }
            }
            else
            {
                FW_LOG_ERR("GetNodeInfo Fail\n");
            }
        }

        return ret;
    }
    else
    {
        //reconnect
        //连接未建立,重新建立连接
        CTSockCommu * commu = (CTSockCommu * ) ctor_;
        //reconnect
        TNodeInfo NodeItem;
        if( 0 != tbl.GetNodeInfo(NodeItem,node_id) )
        {
            FW_LOG_ERR("call tbl.GetNodeInfo()  Fail\n");
            return -6;
        }    

#ifdef OPEN_PRINT
        printf("#########################\n");
        NodeItem.Show();
        printf("#########################\n");           
#endif

        //先保存buffer内容，因为在回调时可能会破坏buff
        string strSendData;
        blob_type * blob = (blob_type *) arg1;
        strSendData.assign((char *) blob->data,blob->len);
        if( 0 == commu->connect(&NodeItem) )
        {
            to_flow = commu_mng_.GetFlow(serv_type,node_id);
            if(to_flow > 0 )
            {
                //恢复buffer内容
                blob->len = strSendData.size();
                memcpy(blob->data,strSendData.data(),blob->len );
                ret = ctor_->sendto(to_flow, arg1, arg2);
            }
            else
            {
                FW_LOG_ERR("Reconnect fail!!!to_flow=%d\n",to_flow);
                return -5;
            }
        }
        else
        {
            FW_LOG_ERR("ReConnect Fail ###\n");
        }
    }

    return ret;
}

int CDefaultWorker::spp_sendbyflow(unsigned int flow, void * arg1, void * arg2)
{
    if(ctor_ != NULL )
    {
        return ctor_->sendto(flow,arg1,arg2);
    }
    else
    {
        FW_LOG_ERR("Send error,ctor_ is NULL,flow=%u\n",flow);
        return -1;
    }
}

int CDefaultWorker::spp_get_group_num()
{
    return ix_->group_num_;
}

int CDefaultWorker::spp_get_group_id()
{
    return ix_->cur_group_id_;
}

void CDefaultWorker::realrun(int argc, char* argv[])
{
    //初始化配置
    initconf(false);

    struct timeval now;
    unsigned int nowtime, montime, stattime,chkconntime;  

    chkconntime = stattime = get_timebytask(NULL);
    montime = stattime - ix_->moni_inter_time_ - 1;
    FW_LOG_INFO("worker started!\n");

    int t_tick = 0;    
    bool que_idle = false;
    bool fd_idle = false;
    while(true)
    {
        //轮询acceptor
        if(ator_->poll() != 0)
        {
            que_idle = true;
        }
        else
        {
            que_idle = false;
        }

        //轮询connector
        if(ctor_ && ctor_->poll() != 0)
        {
            fd_idle = true;
        }
        else if( ctor_ == NULL )
        {
            fd_idle = true;
        }
        else
        {
            fd_idle = false;
        }

        if(que_idle && fd_idle )
        {
            ++t_tick;
            //轮询N次没有收到数据
            if(t_tick >= 100 )
            {
                t_tick = 1;
                usleep(100);
            }
        }
        else
        {
            t_tick = 1;
        }

        //回调注册的定时器
        get_timeofday(&now, NULL);
        do_timer_callback( now );
            
        nowtime = get_timebytask(NULL);

            
        //统计信息输出
        if(unlikely(nowtime - montime > ix_->moni_inter_time_))        
        {
            TStatObjWrapper statobj;
            //每秒负载
            stat_.query(ATOR_RECV_NUM, &statobj); 
            unsigned long proc_num = STATVAL_READ(statobj.value_[0]);
            CLI_SEND_INFO(&moncli_)->watermark_ = 0;//proc_num / ix_->moni_inter_time_;

            //成功率
            if(proc_num > 0)
            {    
                stat_.query(FAIL_PROC_NUM, &statobj); 
                unsigned long fail_num = STATVAL_READ(statobj.value_[0]);
                CLI_SEND_INFO(&moncli_)->srate_ = (proc_num - fail_num) * 1000 / proc_num;
            }
            else
            {
                CLI_SEND_INFO(&moncli_)->srate_ = 0;
            }

            //平均延时	        
            stat_.query(AVG_PROC_TIME, &statobj);
            unsigned long delay = STATVAL_READ(*(statobj.count_));
            if(delay > 0)
            {    
                delay = STATVAL_READ(statobj.value_[0]) / delay;
                CLI_SEND_INFO(&moncli_)->delay_ = delay;
            }
            else
            {
                CLI_SEND_INFO(&moncli_)->delay_ = 0;
            }

            //stat_.reset();
            stattime = nowtime;

            CLI_SEND_INFO(&moncli_)->timestamp_ = nowtime;//time(NULL);
            moncli_.run();
            montime = nowtime;
            //FW_LOG_DBG("moncli run!\n");

            //取动态内存使用量
            CLI_SEND_INFO(&moncli_)->memused_ = CMisc::getmemused();
        }

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
                assert(conf.FindElem("worker"));
                conf.IntoElem();
                assert(conf.FindElem("module"));
                string module_etc = conf.GetAttrib("etc");
                    
                blob_type blob;
                blob.data = SBUFFER;
                blob.extdata = NULL;
                blob.owner = (void *) ator_;
                sppdll.spp_handle_reloadconfig((void *) module_etc.c_str(),&blob,this);

                FW_LOG_INFO("recv reload module config signal!\n");
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

    FW_LOG_INFO("worker stopped!\n");
}
int CDefaultWorker::initconf(bool reload)
{
    CMarkupSTL commconf,conf;

    commconf.Load(ix_->argv_[1]);
    conf.Load(ix_->argv_[2]);// spp_worker.xml

    assert(commconf.FindElem("common"));
    assert(conf.FindElem("worker"));

	//./spp_worker ../etc/spp_common.xml ./../etc/spp_worker.xml 2 1 2 SVR_TaoGu_Proxy_Work_Flag
    ix_->group_num_ = atoi(ix_->argv_[3]);
    ix_->cur_group_id_ = atoi(ix_->argv_[4]);
    ix_->max_proc_num_ = atoi(ix_->argv_[5]);

    assert(ix_->cur_group_id_ > 0 && ix_->cur_group_id_ < MAX_PROC_GROUP_NUM);

    commconf.IntoElem();
    conf.IntoElem();

    /////////////////////////////////////////////////////////
    //初始化框架日志	实例
    {
        commconf.ResetMainPos();
        assert(commconf.FindElem("fwlog"));

        int log_level = atoi(commconf.GetAttrib("level").c_str());
        int log_type = atoi(commconf.GetAttrib("type").c_str());
        string log_path = commconf.GetAttrib("path");

        //log前缀通过程序名+组号生成,spp_ctrl的前缀直接用程序名
        char * pLogPrefix =  strrchr(ix_->argv_[0],'/');
        assert(pLogPrefix);
        char szLogPrefix[128] = {0};
        sprintf(szLogPrefix,"%s%d",pLogPrefix,ix_->cur_group_id_);
        string name_prefix =szLogPrefix;

        int max_file_size = atoi(commconf.GetAttrib("maxfilesize").c_str());
        int max_file_num = atoi(commconf.GetAttrib("maxfilenum").c_str());

        int log_key_base = strtol(commconf.GetAttrib("key_base").c_str(),0,0 );
        int semkey = log_key_base + ix_->cur_group_id_;
        
        assert((log_level >= LOG_TRACE) && (log_level <= LOG_NONE) 
            && (log_type >= LOG_TYPE_CYCLE) && (log_type <= LOG_TYPE_CYCLE_HOURLY) 
            && (max_file_size > 0) && (max_file_size <= 1024000000) && (max_file_num > 0));

        m_log_internal.LOG_OPEN(log_level, log_type, log_path.c_str(), name_prefix.c_str(), max_file_size, max_file_num, semkey);
        FW_LOG_INFO("Worker start!!!\n");
    }
    //////////////////////////////////////////////////////////////////////


    /////////////////////////////////////////////////////////
    //初始化日志	
    commconf.ResetMainPos();
    assert(commconf.FindElem("log"));

    int log_level = atoi(commconf.GetAttrib("level").c_str());
    int log_type = atoi(commconf.GetAttrib("type").c_str());
    string log_path = commconf.GetAttrib("path");

    //log前缀通过程序名+组号生成,spp_ctrl的前缀直接用程序名
    char * pLogPrefix =  strrchr(ix_->argv_[0],'/');
    assert(pLogPrefix);
    char szLogPrefix[256] = {0};
    sprintf(szLogPrefix,"%s%d",pLogPrefix,ix_->cur_group_id_);
    string name_prefix =szLogPrefix;

    int max_file_size = atoi(commconf.GetAttrib("maxfilesize").c_str());
    int max_file_num = atoi(commconf.GetAttrib("maxfilenum").c_str());

    int log_key_base = strtol(commconf.GetAttrib("key_base").c_str(),0,0 );
    int semkey = log_key_base + ix_->cur_group_id_;
    
    assert((log_level >= LOG_TRACE) && (log_level <= LOG_NONE) 
        && (log_type >= LOG_TYPE_CYCLE) && (log_type <= LOG_TYPE_CYCLE_HOURLY) 
        && (max_file_size > 0) && (max_file_size <= 1024000000) && (max_file_num > 0));

    log_.LOG_OPEN(log_level, log_type, log_path.c_str(), name_prefix.c_str(), max_file_size, max_file_num, semkey);
    //////////////////////////////////////////////////////////////////////

    //acceptor配置	
    if(!reload)
    {
        conf.ResetMainPos();
        assert(conf.FindElem("acceptor"));
        string type = conf.GetAttrib("type");
        string maxconn = conf.GetAttrib("maxconn");
        string maxpkg = conf.GetAttrib("maxpkg");
        string timeout = conf.GetAttrib("timeout");
        string max_poll_pkg = conf.GetAttrib("max_poll_pkg");
        if(type == "shm")
        {
            assert(commconf.FindElem("ShmQueue"));

            int recv_size = atoi(commconf.GetAttrib("send_size").c_str());
            int send_size = atoi(commconf.GetAttrib("recv_size").c_str());
            int shm_queue_key_base = strtol(commconf.GetAttrib("key_base").c_str() ,0,0);
            
            TShmCommuConf shm;
            shm.shmkey_producer_ = shm_queue_key_base + 2 * (ix_->cur_group_id_ - 1) + 1;
            shm.shmsize_producer_ = send_size * (1<<20);

            shm.shmkey_comsumer_ = shm_queue_key_base + 2 * (ix_->cur_group_id_ - 1) ;
            shm.shmsize_comsumer_ = recv_size * (1<<20);
            shm.msg_timeout_ = atoi(timeout.c_str()) ;
            
            //printf("worker group=%d,producer key=0x%08x,comsumer key=0x%08x\n",ix_->cur_group_id_,shm.shmkey_producer_,shm.shmkey_comsumer_);

            if(ix_->max_proc_num_  > 1)
            {
                shm.locktype_ = LOCK_TYPE_PRODUCER | LOCK_TYPE_COMSUMER;
            }
            else
            {
                shm.locktype_ = LOCK_TYPE_NONE;
            }
            
            shm.maxpkg_ = atoi(maxpkg.c_str());
            shm.expiretime_ = 0;
            assert(shm.maxpkg_ >= 0 && shm.expiretime_ >= 0 );
            
            ator_ = new CTShmCommu;
            assert(ator_->init(&shm) == 0);
            ator_->reg_cb(CB_RECVDATA, ator_recvdata, this);            
            ator_->reg_cb(CB_SENDDATA, ator_senddata, this);
            ator_->reg_cb(CB_OVERLOAD, ator_overload, this);
        }        
        else if(type == "socket")
        {
            TSockCommuConf socks;
            memset(&socks, 0x0, sizeof(TSockCommuConf));
            socks.maxconn_ = atoi(maxconn.c_str());
            socks.maxpkg_ = atoi(maxpkg.c_str());
            socks.expiretime_ = atoi(timeout.c_str());
            assert(socks.maxconn_ > 0 && socks.maxpkg_ >= 0 && socks.expiretime_ >= 0);

            if(type == "tcp")
            {
                socks.sockbind_[0].type_ = SOCK_TYPE_TCP;
                socks.sockbind_[0].ipport_.ip_ = CMisc::getip(conf.GetAttrib("if").c_str());
                socks.sockbind_[0].ipport_.port_ = atoi(conf.GetAttrib("port").c_str());            
            }
            else if(type == "udp")
            {
                socks.sockbind_[0].type_ = SOCK_TYPE_UDP;
                socks.sockbind_[0].ipport_.ip_ = CMisc::getip(conf.GetAttrib("if").c_str());
                socks.sockbind_[0].ipport_.port_ = atoi(conf.GetAttrib("port").c_str());            
            }
            else if(type == "unix")
            {
                socks.sockbind_[0].type_ = SOCK_TYPE_UNIX;
                strncpy(socks.sockbind_[0].path_, conf.GetChildAttrib("path").c_str(), 256);
            }
            else
            {
                assert(false);
            }
            
            ator_ = new CTSockCommu;
            assert(ator_->init(&socks) == 0);
            ator_->reg_cb(CB_RECVDATA, ator_recvdata, this); 
            ator_->reg_cb(CB_SENDDATA, ator_senddata, this);
        }
    }
    //acceptor配置

    //connector配置
    if(!reload)
    {
        conf.ResetMainPos();
        if(conf.FindElem("connector"))
        {
            string type = conf.GetAttrib("type");
        
            if(type == "socket")
            {
                route_config_ = new tagTRouteConfig;
                assert(route_config_ != NULL );
                route_config_->max_conn_ = atoi(conf.GetAttrib("maxconn").c_str());
                route_config_->maxpkg_ = atoi(conf.GetAttrib("maxpkg").c_str());//每秒最大包量
                if(route_config_->maxpkg_ < 0 )
                    route_config_->maxpkg_ = 0;//不检查
                    
                route_config_->expiretime_ = atoi(conf.GetAttrib("timeout").c_str());    
                route_config_->check_expire_interval_ = atoi(conf.GetAttrib("check_expire_interval").c_str());//检查连接超时时间间隔
                assert(route_config_->check_expire_interval_ >= 0 );


                string strRouteTable = conf.GetAttrib("route_table");//路由表文件
                assert( 0 == InitRouteConfig(route_config_,strRouteTable,reload) );

                ctor_ = new CTSockCommu;
                ctor_->reg_cb(CB_CONNECT,ctor_connect,this);
                ctor_->reg_cb(CB_CONNECTED, ctor_connected, this);    
                ctor_->reg_cb(CB_DISCONNECT, ctor_disconn, this);    
                ctor_->reg_cb(CB_TIMEOUT, ctor_timeout, this);
                int ret = ctor_->InitExt(route_config_);
                assert( ret == 0);

                //注册回调函数
                ctor_->reg_cb(CB_OVERLOAD, ctor_overload, this);
                ctor_->reg_cb(CB_RECVDATA,ctor_recvdata,this);//延迟注册
                
            }
            else
            {
                //...
            }


        }

    }
    //connector配置	       

    //初始化统计

    commconf.ResetMainPos();
    assert(commconf.FindElem("stat"));
    string mapfile = commconf.GetAttrib("mapfile");
    if(mapfile == "")
    {
        stat_.init_statpool(NULL);
    }
    else
    {
        stat_.init_statpool(mapfile.c_str());
    }
    
    stat_.init_statobj(ATOR_RECV_NUM, STAT_TYPE_SUM);
    stat_.init_statobj(ATOR_SEND_NUM, STAT_TYPE_SUM);
    stat_.init_statobj(AVG_PROC_TIME, STAT_TYPE_AVG);
    stat_.init_statobj(MAX_PROC_TIME, STAT_TYPE_MAX);
    stat_.init_statobj(FAIL_PROC_NUM, STAT_TYPE_SUM);
        
    //初始化监控
    commconf.ResetMainPos();
    assert(commconf.FindElem("moni"));
    ix_->moni_inter_time_ = atoi(commconf.GetAttrib("intervial").c_str());
    assert(ix_->moni_inter_time_ > 0);


    int moni_key_base = strtol(commconf.GetAttrib("key_base").c_str(),0,0);
    key_t mqkey  = moni_key_base;    
        
    CCommu* commu = new CMQCommu(mqkey);
    moncli_.set_commu(commu);    
    memset(CLI_SEND_INFO(&moncli_), 0x0, sizeof(TProcInfo));
    CLI_SEND_INFO(&moncli_)->groupid_ = ix_->cur_group_id_;
    CLI_SEND_INFO(&moncli_)->procid_ = getpid();
    //CLI_SEND_INFO(&moncli_)->timestamp_ = time(NULL);    

    //加载用户服务模块
    conf.ResetMainPos();
    assert(conf.FindElem("module"));
    string module_file = conf.GetAttrib("bin");
    string module_etc = conf.GetAttrib("etc");
    FW_LOG_INFO("load module %s, %s\n", module_file.c_str(), module_etc.c_str());
    if(0 == load_bench_adapter(module_file.c_str()))//加载用户定义库的内容
    {
        assert(sppdll.spp_handle_init((void*)module_etc.c_str(), this) == 0);
        ator_->reg_cb(CB_RECVDATA, ator_recvdata, this);    //数据接收回调注册
    }

    //注册前端通讯事件回调
    ator_->reg_cb(CB_CONNECTED,ator_connected,this);
    ator_->reg_cb(CB_OVERLOAD,ator_overload,this);
    ator_->reg_cb(CB_DISCONNECT,ator_disconn,this);
    ator_->reg_cb(CB_TIMEOUT,ator_timeout,this);
    
    return 0;
}

int CDefaultWorker::InitRouteConfig(TRouteConfig * config, const string & route_file, bool reload)
{
#ifdef OPEN_PRINT
    printf("max_conn=%d,maxpkg=%d,timeout=%d,check_interval=%d\n",
        route_config_->max_conn_,route_config_->maxpkg_,route_config_->expiretime_,route_config_->check_expire_interval_);
#endif

    if(!reload)
    {
        CMarkupSTL conf;
        conf.Load(route_file.c_str() );// route_table.xml
        assert(conf.FindElem("routetable"));
        string base_path = conf.GetAttrib("basepath");
        string route_prefix = conf.GetAttrib("prefix");//读取route_table_00000001.xml
        if(route_prefix == "")
        {
            route_prefix = "route_table";
        }
        char route_cfg[1024];
        while(conf.FindChildElem("group"))
        {
            unsigned int serv_type =strtol(conf.GetChildAttrib("serv_type").c_str(),0,0);//支持0X配置
            sprintf(route_cfg,"%s/%s_%08x.xml",base_path.c_str(),route_prefix.c_str(),serv_type);//route_table_00000001.xml

#ifdef OPEN_PRINT
            printf("serv_type=%d,route_cfg=%s\n",serv_type,route_cfg);// ../dat/route_table/route_table_00000001.xml
#endif

            TRouteTable route_table;
            route_table.serv_type_ = serv_type;
            int retval = InitRouteTable(route_table,route_cfg);    
            assert(retval == 0 );
            route_config_->map_route_table_[serv_type] = route_table;
        }

    }
    return 0;
}


int CDefaultWorker::InitRouteTable(TRouteTable & route_tbl, const string & route_cfg)
{
    CMarkupSTL conf;
    conf.Load(route_cfg.c_str() );//route_table_00000001.xml
    assert(conf.FindElem("group"));

    string strVersion = conf.GetAttrib("version");
    unsigned int serv_type = strtol(conf.GetAttrib("serv_type").c_str(),0,0);
    assert(serv_type == route_tbl.serv_type_ );
    route_tbl.version_ = strVersion;
    string strRoute = conf.GetAttrib("route");// "key % 1"

    AllTrim(strRoute);
    if(strRoute != "" )
    {
        assert(strRoute.substr(0,3) == "key" );        
        route_tbl.route_  = strRoute.substr(3,1);// %
        route_tbl.route_val_ = atoi(strRoute.substr(4,strRoute.size() - 4 ).c_str() );// 1
        FW_LOG_INFO("route=%s,route_arithmetic=%s,route_val=%d\n",strRoute.c_str(),route_tbl.route_.c_str(),route_tbl.route_val_);
    }

    while(conf.FindChildElem("node"))
    {
        string strType = conf.GetChildAttrib("type");
        TNodeInfo nodeinfo; // <node id="1" type="tcp" begin="0" end="0" serv_addr="172.30.8.210" port="6001" />
        nodeinfo.node_id_ = atoi(conf.GetChildAttrib("id").c_str());
        nodeinfo.serv_type_ = route_tbl.serv_type_;
        int ret = 0;

        if(strType == "tcp")
        {
            nodeinfo.bind_.type_ = SOCK_TYPE_TCP;
            ret = CSocketAddr::in_s2n((conf.GetChildAttrib("serv_addr").c_str()), nodeinfo.bind_.ipport_.ip_);
            assert(ret >= 0 );
            nodeinfo.bind_.ipport_.port_ = atoi(conf.GetChildAttrib("port").c_str());            
        }
        else if(strType == "udp")
        {
            nodeinfo.bind_.type_ = SOCK_TYPE_UDP;
            ret = CSocketAddr::in_s2n((conf.GetChildAttrib("serv_addr").c_str()), nodeinfo.bind_.ipport_.ip_);
            assert(ret >= 0 );
            nodeinfo.bind_.ipport_.port_ = atoi(conf.GetChildAttrib("port").c_str());            
        }
        else if(strType == "unix")
        {
            nodeinfo.bind_.type_ = SOCK_TYPE_UNIX;
            strncpy(nodeinfo.bind_.path_, conf.GetChildAttrib("path").c_str(), 256);
        }
        else
        {
            assert(false);
        }

        nodeinfo.begin_ = atoi(conf.GetChildAttrib("begin").c_str());
        nodeinfo.end_ = atoi( conf.GetChildAttrib("end").c_str() );

        route_tbl.map_node_[nodeinfo.node_id_ ] = nodeinfo;
        
    }

    return 0;
}



void CDefaultWorker::do_timer_callback(const struct timeval & nowtime )
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
            sendblob.ext_len = 0;
            sendblob.extdata = NULL;            
            (*(*itr)->func_)(&sendblob,this,(*itr)->args_);
            (*itr)->proctime_ = nowtime;
        }
    }    

}


//ator的一些回调函数

int CDefaultWorker::ator_recvdata(unsigned flow, void* arg1, void* arg2)
{
    int ret = -1;
    blob_type* blob = (blob_type*)arg1;
    CDefaultWorker* worker = (CDefaultWorker*)arg2;

    worker->m_log_internal.LOG_P_LEVEL(LOG_DEBUG, "CDefaultWorker::ator_recvdata_v2, flow=%u,len=%d\n", flow, blob->len);

    //此处应该调用业务so进行完整性检查，暂时忽略
    //spp_handle_input(...)

    if(likely(blob->len > 0))
    {
        worker->m_log_internal.LOG_P_LEVEL(LOG_DEBUG, "CDefaultWorker::ator_recvdata_v2,ator recvdone, flow=%u,len=%d\n", flow, blob->len);
        worker->stat_.step0(ATOR_RECV_NUM, 1);

        DELAY_INIT;
        //这里要加入超时监控，否则容易把worker挂死
        int ret = sppdll.spp_handle_process(flow, PKG_SOURCE_CLIENT,arg1, arg2);
        if(likely(!ret))
        {
            unsigned delay = GET_DELAY;
            worker->stat_.step0(AVG_PROC_TIME, delay);
            worker->stat_.step0(MAX_PROC_TIME, delay);
            //return 0;
            ret = 0;
        }
    	else
        {
            if(likely(ret < 0))  //返回负值，表示处理失败，正值表示成功且主动关闭连接
            {
                worker->stat_.step0(FAIL_PROC_NUM, 1);
            }
            else
            {
                unsigned delay = GET_DELAY;
                worker->stat_.step0(AVG_PROC_TIME, delay);
                worker->stat_.step0(MAX_PROC_TIME, delay);
            }

            CTCommu* commu = (CTCommu*)blob->owner;
            blob_type rspblob;
            rspblob.len = 0;
            rspblob.data = NULL;
            commu->sendto(flow, &rspblob, NULL);
            worker->m_log_internal.LOG_P_LEVEL(LOG_DEBUG, "CDefaultWorker close conn, flow=%u\n", flow);
        }
    }
    return ret;
}

int CDefaultWorker::ator_senddata(unsigned flow, void* arg1, void* arg2)
{
    blob_type* blob = (blob_type*)arg1;
    CDefaultWorker* worker = (CDefaultWorker*)arg2;
    worker->m_log_internal.LOG_P_LEVEL(LOG_DEBUG, "CDefaultWorker,ator senddata, flow=%u,len=%d\n", flow, blob->len);
    worker->stat_.step0(ATOR_SEND_NUM, 1);

    return 0;
}
int CDefaultWorker::ator_overload(unsigned flow, void* arg1, void* arg2)
{
    blob_type* blob = (blob_type*)arg1;
    CDefaultWorker* worker = (CDefaultWorker*)arg2;
    worker->m_log_internal.LOG_P_LEVEL(LOG_ERROR, "warnning!!CDefaultWorker,worker overload ,flow=%u,len=%d\n",flow, blob->len);
    blob->ext_len = 0;
    blob->ext_type = EXT_TYPE_NONE;

    if(blob->len == 0 && blob->ext_type == ET_OVERLOAD && worker->event_func_list_[ET_OVERLOAD] != NULL )
    {
        worker->event_func_list_[ET_OVERLOAD](flow,arg1,arg2,worker->event_func_args_[ET_OVERLOAD]);
    }
    return 0;
}

int CDefaultWorker::ator_connected(unsigned flow, void * arg1, void * arg2)
{
    blob_type* blob = (blob_type*)arg1;
    CDefaultWorker* worker = (CDefaultWorker*)arg2;
    worker->m_log_internal.LOG_P_LEVEL(LOG_DEBUG, "CDefaultWorker,ator_connected,flow=%u, len=%d\n",flow, blob->len);
    blob->ext_len = 0;
    blob->ext_type = EXT_TYPE_NONE;

    if(worker->event_func_list_[ET_CONNECTED] != NULL )
    {
        worker->event_func_list_[ET_CONNECTED](flow,arg1,arg2,worker->event_func_args_[ET_CONNECTED]);
    }
    else
    {
        worker->m_log_internal.LOG_P_LEVEL(LOG_DEBUG,"%s,worker->event_func_list_[ET_CONNECTED] is null\n",__FUNCTION__);
    }
    
    return 0;
}

int CDefaultWorker::ator_timeout(unsigned flow, void * arg1, void * arg2)
{
    blob_type* blob = (blob_type*)arg1;
    CDefaultWorker* worker = (CDefaultWorker*)arg2;
    worker->m_log_internal.LOG_P_LEVEL(LOG_NORMAL, "warnning!!!CDefaultWorker,ator_timeout,flow=%u, len=%d\n",flow, blob->len);
    blob->ext_len = 0;
    blob->ext_type = EXT_TYPE_NONE;

    if(worker->event_func_list_[ET_TIMEOUT] != NULL )
    {
        worker->event_func_list_[ET_TIMEOUT](flow,arg1,arg2,worker->event_func_args_[ET_TIMEOUT]);
    }
    else
    {
        worker->m_log_internal.LOG_P_LEVEL(LOG_NORMAL,"%s,worker->event_func_list_[ET_TIMEOUT] is null\n",__FUNCTION__);
    }
    
    return 0;
}

int CDefaultWorker::ator_disconn(unsigned flow, void * arg1, void * arg2)
{     
    blob_type* blob = (blob_type*)arg1;
    CDefaultWorker* worker = (CDefaultWorker*)arg2;
    worker->m_log_internal.LOG_P_LEVEL(LOG_DEBUG, "CDefaultWorker,ator_disconn,flow=%u, len=%d\n",flow, blob->len);
    blob->ext_len = 0;
    blob->ext_type = EXT_TYPE_NONE;

    if(worker->event_func_list_[ET_DISCONNECT] != NULL )
    {
        worker->event_func_list_[ET_DISCONNECT](flow,arg1,arg2,worker->event_func_args_[ET_DISCONNECT]);
    }
    else
    {
        worker->m_log_internal.LOG_P_LEVEL(LOG_DEBUG,"%s,worker->event_func_list_[ET_DISCONNECT] is null\n",__FUNCTION__);
    }
    
    return 0;
}

//ctor的一些回调函数
int CDefaultWorker::ctor_recvdata(unsigned flow,void * arg1,void * arg2)
{
    blob_type* blob = (blob_type*)arg1;
    CDefaultWorker * worker = (CDefaultWorker*)arg2;
    worker->m_log_internal.LOG_P_LEVEL(LOG_DEBUG, "CDefaultWorker ctor recvdata , flow=%u,len=%d\n", flow, blob->len);
    int total_len=blob->len;
    int processed_len=0;
    int proto_len = -1; 
    int ret=0;

    while(blob->len>0&&(proto_len= sppdll.spp_handle_input(flow, arg1, arg2))>0)
    {
        if(proto_len>blob->len)
        {
            worker->m_log_internal.LOG_P_LEVEL(LOG_ERROR, "CDefaultWorker spp_handle_input error, flow=%u,blob_len=%d,proto_len=%d\n", flow, blob->len, proto_len);
            processed_len=total_len;
            break;    
        }

        ret = 0;

        worker->stat_.step0(CTOR_RECV_NUM, 1);
        processed_len+=proto_len;

        worker->m_log_internal.LOG_P_LEVEL(LOG_DEBUG, "CDefaultWorker,ctor recvdone, flow=%u,blob_len=%d,proto_len=%d\n", flow, blob->len, proto_len);

        blob_type sendblob;
        sendblob.len = proto_len;
        sendblob.data= blob->data ;

        if(sppdll.spp_handle_process != NULL )
        {
            sendblob.len = proto_len;
            sendblob.data = SBUFFER;            
            memcpy(sendblob.data , blob->data, proto_len);
            sendblob.owner =  (CTCommu*) worker->ator_;

            sendblob.ext_len = 0;
            sendblob.extdata = NULL;

            worker->m_log_internal.LOG_P_LEVEL(LOG_DEBUG,"CDefaultWorker,spp_handle_process,flow=%u,data len =%d,ext_len=%d\n",flow,sendblob.len,sendblob.ext_len);

            //本地处理，不需要发送给后端处理	        
            ret = sppdll.spp_handle_process(flow,PKG_SOURCE_SERVER,(void *) &sendblob,arg2);

            worker->m_log_internal.LOG_P_LEVEL(LOG_DEBUG,"CDefaultWorker,spp_handle_process,ret =%d\n",ret);
            if(likely(!ret))
            {
                //process return ok.
            }
            else
            {
                if(likely(ret < 0))  //返回负值，表示处理失败，正值表示成功且主动关闭连接
                {
                    //worker->stat_.step0(FAIL_PROC_NUM, 1);
                }
                else
                {
                    //unsigned delay = GET_DELAY;
                    //worker->stat_.step0(AVG_PROC_TIME, delay);
                    //worker->stat_.step0(MAX_PROC_TIME, delay);
                }

                CTCommu* commu = (CTCommu*)blob->owner;
                blob_type rspblob;
                rspblob.len = 0;
                rspblob.data = NULL;
                commu->sendto(flow, &rspblob, NULL);
                worker->m_log_internal.LOG_P_LEVEL(LOG_DEBUG, "CDefaultWorker,close conn, %u\n", flow);
                break;//连接已关闭，在缓冲中的数据已被丢掉，必须跳出循环
            }
        
        }    
        else
        {
            worker->m_log_internal.LOG_P_LEVEL(LOG_ERROR,"CDefaultWorker,ctor recv data,flow=%d,len=%d,spp_handle_process is null,throw away.",flow,blob->len);
        }

        blob->data+=proto_len;
        blob->len-=proto_len;
    }

    if(proto_len < 0)
        return proto_len;    

    return processed_len;    
}

int CDefaultWorker::ctor_senddata(unsigned flow,void * arg1,void * arg2)
{
    blob_type* blob = (blob_type*)arg1;
    CDefaultWorker* worker = (CDefaultWorker*)arg2;
    worker->m_log_internal.LOG_P_LEVEL(LOG_DEBUG, "CDefaultWorker, ctor_senddata,flow=%d,data_len=%d\n",flow, blob->len);
    return 0;
}

int CDefaultWorker::ctor_overload(unsigned flow, void* arg1, void* arg2)
{
    blob_type* blob = (blob_type*)arg1;
    CDefaultWorker* worker = (CDefaultWorker*)arg2;
    worker->m_log_internal.LOG_P_LEVEL(LOG_ERROR, "CDefaultWorker ctor_overload flow=%d,data_len=%d\n",flow, blob->len);
    blob->ext_type = EXT_TYPE_NONE;
    blob->ext_len = 0;
        
    if(worker->back_event_func_list_[EVENT_OVERLOAD] != NULL )
    {
        unsigned int serv_type = 0;
        serv_type = worker->commu_mng_.GetServerTypeByFlow(flow);
        if(serv_type > 0 )
        {
            blob->data = SBUFFER;
            blob->len = 0;
            worker->back_event_func_list_[EVENT_OVERLOAD](serv_type,flow,arg1,arg2,worker->back_event_func_args_[EVENT_OVERLOAD]);
        }
        else
        {
            worker->m_log_internal.LOG_P_LEVEL(LOG_NORMAL,"%s:conn not found,flow=%u\n",__FUNCTION__,flow);
        }
    }
    return 0;
}
int CDefaultWorker::ctor_connected(unsigned flow, void* arg1, void* arg2)
{
    blob_type* blob = (blob_type*)arg1;
    CDefaultWorker* worker = (CDefaultWorker*)arg2;
    worker->m_log_internal.LOG_P_LEVEL(LOG_DEBUG, "CDefaultWorker ctor_connected,flow = %d,data_len=%d\n",flow, blob->len);    
    blob->ext_type = EXT_TYPE_NONE;
    blob->ext_len = 0;

    if(worker->back_event_func_list_[EVENT_CONNECTED] != NULL )
    {
        unsigned int serv_type = 0;
        serv_type = worker->commu_mng_.GetServerTypeByFlow(flow);
        if(serv_type > 0 )
        {
            blob->data = SBUFFER;
            blob->len = 0;
            worker->back_event_func_list_[EVENT_CONNECTED](serv_type,flow,arg1,arg2,worker->back_event_func_args_[EVENT_CONNECTED]);
        }
        else
        {
            worker->m_log_internal.LOG_P_LEVEL(LOG_DEBUG,"%s:conn not found,flow=%u\n",__FUNCTION__,flow);
        }
    }
    return 0;
}

int CDefaultWorker::ctor_timeout(unsigned flow, void* arg1, void* arg2)
{
    blob_type* blob = (blob_type*)arg1;
    CDefaultWorker* worker = (CDefaultWorker*)arg2;
    worker->m_log_internal.LOG_P_LEVEL(LOG_NORMAL, "CDefaultWorker ctor_timeout flow=%d,data_len=%d\n",flow, blob->len);
    blob->ext_type = EXT_TYPE_NONE;
    blob->ext_len = 0;

    if(worker->back_event_func_list_[EVENT_TIMEOUT] != NULL )
    {
        unsigned int serv_type = 0;
        serv_type = worker->commu_mng_.GetServerTypeByFlow(flow);
        if(serv_type > 0 )
            {
            blob->data = SBUFFER;
            blob->len = 0;
            worker->back_event_func_list_[EVENT_TIMEOUT](serv_type,flow,arg1,arg2,worker->back_event_func_args_[EVENT_TIMEOUT]);
        }
        else
        {
            worker->m_log_internal.LOG_P_LEVEL(LOG_DEBUG,"%s:conn not found,flow=%u\n",__FUNCTION__,flow);
        }
    }

    return 0;
}
int CDefaultWorker::ctor_disconn(unsigned flow, void* arg1, void* arg2)
{
    blob_type* blob = (blob_type*)arg1;
    CDefaultWorker* worker = (CDefaultWorker*)arg2;
    worker->m_log_internal.LOG_P_LEVEL(LOG_DEBUG, "CDefaultWorker ctor_disconn ,flow=%d,data_len=%d\n", flow,blob->len);
    worker->commu_mng_.RMConn(flow);//移除该连接

    blob->ext_type = EXT_TYPE_NONE;
    blob->ext_len = 0;

    if(worker->back_event_func_list_[EVENT_DISCONNECT] != NULL )
    {
        unsigned int serv_type = 0;
        serv_type = worker->commu_mng_.GetServerTypeByFlow(flow);
        if(serv_type > 0 )
        {
             blob->data = SBUFFER;
            blob->len = 0;
            worker->back_event_func_list_[EVENT_DISCONNECT](serv_type,flow,arg1,arg2,worker->back_event_func_args_[EVENT_DISCONNECT]);
        }
        else
        {
            worker->m_log_internal.LOG_P_LEVEL(LOG_DEBUG,"%s:conn not found,flow=%u\n",__FUNCTION__,flow);
        }
    }
    return 0;
}

int CDefaultWorker::ctor_connect(unsigned int flow, void * arg1, void * arg2)
{
    blob_type * blob = (blob_type *) arg1;
    CDefaultWorker * worker = (CDefaultWorker *) arg2;
    TNodeInfo * pNode = (TNodeInfo *) blob->extdata;

    if(pNode != NULL )
    {
        worker->m_log_internal.LOG_P_LEVEL(LOG_DEBUG,"%s:serv_type=%d,node_id=%d,flow=%d\n",__FUNCTION__,
        pNode->serv_type_,pNode->node_id_,flow);

        worker->commu_mng_.AddConn(pNode->serv_type_,pNode->node_id_,flow);
    }
    
    return 0;
}

