
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "defaultreportsvr.h"
#include "MarkupSTL.h"
#include "likelydef.h"
#include "protocol_pack.h"
#include "hexdump.h"

using namespace protocol;
using namespace comm::base;
using namespace comm::util;

namespace spp
{
namespace reportsvr
{

const unsigned short LB_CONNECT = 0x0100;
const unsigned short LB_SUBMIT_LOADINFO = 0x0101;
const unsigned short LB_EXIT_NOTIFY = 0x0102;
const unsigned short LB_KEEP_ALIVE = 0x0103;


static int HandleInput(const char *buf, unsigned int len)
{
    if(len < 7)
    {
        return 0;
    }
    unsigned int *pn = (unsigned int *)(buf+3);
    return ntohl(*pn);
}

    
static int MakeReportReq(char * buf,int buf_len,unsigned int seq,unsigned int ip,unsigned short tcp_port,unsigned short udp_port,
        unsigned short server_type,unsigned char net_type,unsigned int conn_num,unsigned int max_conn_num,const string & strExtInfo)
{
    FixedPacker pk;
    int ret = pk.Init(buf,buf_len);
    if(ret != 0 )
    {
        return ret;
    }

    MsgHeader h;
    h.main_cmd = LB_SUBMIT_LOADINFO;
    h.sub_cmd = 0;
    h.seq = seq;

    pk.PackDWord(ip);
    pk.PackWord(tcp_port);
    pk.PackWord(udp_port);
    pk.PackWord(server_type);
    pk.PackByte(net_type);
    pk.PackDWord(conn_num);
    pk.PackDWord(max_conn_num);
    unsigned short ext_len = (unsigned short ) strExtInfo.size();
    pk.PackWord(ext_len);
    pk.PackBinary(strExtInfo.data(),ext_len);

    int pack_len = 0;
    pk.GetPack(pack_len,h);
    return pack_len;
    
}


CReportServerConf::CReportServerConf():serverport_(0),ipaddress_(0),tcpport_(0),udpport_(0),maxconn_num_(100000)
{
}

CReportServerConf::~CReportServerConf()
{
}

CDefaultReportServer::CDefaultReportServer():TcpClient_(NULL)
{

}

CDefaultReportServer::~CDefaultReportServer()
{
    if(TcpClient_)
        delete TcpClient_;
    TcpClient_ = NULL;
}

void CDefaultReportServer::realrun(int argc, char * argv [ ])
{
    initconf(false);
    int ret = 0;
    const unsigned  MAX_BUF_LEN = 10240;
    static char req_buf[MAX_BUF_LEN];
    static char rsp_buf[MAX_BUF_LEN];
    int rsp_len = MAX_BUF_LEN - 1;

    static unsigned int packet_seq = 0;

    //////////////////////////////////////
    m_log_internal.LOG_P_LEVEL(LOG_DEBUG,"Prepare login\n");
    if(TcpClient_ && TcpClient_->IsValid() )
    	m_log_internal.LOG_P_LEVEL(LOG_DEBUG,"connect server ok!\n");
    	
    /////////////////////////////////////

    const char * key = "cur_connect_num";
    loadreader_.Init_LoadItem(key,sizeof(unsigned int));

    unsigned int nowtime, reporttime;
    nowtime = reporttime = time(NULL);

    unsigned int nConnectNum = 0; 
    const unsigned int send_timeout = 1000; // 1秒                    

    string strExtInfo = "";
    
    while(true)
    {
        //检查reload信号
        if(unlikely(CServerBase::reload()))
        {	
            m_log_internal.LOG_P_LEVEL(LOG_NORMAL, "recv reload signal\n");
            initconf(true);
        }

        //检查quit信号
        if(unlikely(CServerBase::quit()))
        {	
            m_log_internal.LOG_P_LEVEL(LOG_NORMAL, "recv quit signal\n");
            break;	
        }

        //检查reloadmoduleconfig
	if(unlikely(CServerBase::reloadmoduleconfig() ) )
	{
	    m_log_internal.LOG_P_LEVEL(LOG_NORMAL, "recv reloadmoduleconfig signal\n");
	    //DoReloadModuleConfig
	}

        
        sleep(1);//休眠1秒
        nowtime = time(NULL);

        if(likely(TcpClient_ != NULL ) )
        {
            if( (nowtime - reporttime) >= (unsigned int) ix_->load_inter_time_   && loadreader_.read( (void *) &nConnectNum,4,key) != 0)
            {
                //读取当前连接数
                m_log_internal.LOG_P_LEVEL(LOG_DEBUG,"#####cur_connect_num=%d\n",nConnectNum);
 
                ret = MakeReportReq(req_buf,MAX_BUF_LEN, ++packet_seq,conf_.ipaddress_,conf_.tcpport_,conf_.udpport_,
                    conf_.server_type_,conf_.net_type_,nConnectNum,conf_.maxconn_num_,strExtInfo);
                if(ret <= 0 )
                {
                    m_log_internal.LOG_P_LEVEL(LOG_ERROR,"%s:call  MakeReportReq() return fail,ret=%d\n",__FUNCTION__,ret);
                    continue;
                }
                
                ret = TcpClient_->SendRecvWithRetry(req_buf, ret, HandleInput, MSG_HEADER_LEN, rsp_buf, rsp_len,send_timeout);

                if(ret <= 0 )
                {
                    m_log_internal.LOG_P_LEVEL(LOG_ERROR,"%s:call TcpClient->SendRecvWithRetry() fail,ret=%d\n",__FUNCTION__,ret);
                    continue;
                }

                m_log_internal.LOG_P_LEVEL(LOG_DEBUG,"recv data len=%d\n",ret);

                UnPacker upk;//解包
                ret = upk.Init(rsp_buf,ret);               
                if(ret != 0 )
                {
                    m_log_internal.LOG_P_LEVEL(LOG_ERROR,"recv a invalid pkg from loadbalance !!!\n");
                    continue;
                }

                ret = upk.UnPackDWord();
                string strRetString = upk.UnPackString();
                m_log_internal.LOG_P_LEVEL(LOG_DEBUG,"report to loadbalance server,ret=%d,info=%s\n",ret,strRetString.c_str());
                reporttime = nowtime;
            }

        }
       

    }

}

int CDefaultReportServer::InitLoadTable(const char * comm_config)
{
    if(!comm_config)
        return -1;

    CMarkupSTL conf;
    conf.Load(comm_config);
    bool bFlag = conf.FindElem("common");
    assert(bFlag);
    conf.IntoElem();

    bFlag = conf.FindElem("moni");
    assert(bFlag);

    int shmsize = atoi(conf.GetAttrib("shmsize").c_str() );
    int moni_key_base = strtol(conf.GetAttrib("key_base").c_str(),0,0);    
    
    //共享内存的KEY,SIZE,SEMKEY初始化
    int shmkey = moni_key_base;
    int semkey = moni_key_base;
    
#ifdef OPEN_PRINT
    printf("shmread init,shmkey=0x%08x,shmsize=%d,semkey=0x%08x\n",shmkey,shmsize,semkey);	
#endif

    assert( shmkey != -1 && semkey != -1);

    int ret = loadreader_.init(shmkey,shmsize,semkey);
    assert(ret == 0 );

    return ret;
}

int CDefaultReportServer::initconf(bool reload /*= false*/)
{
    CMarkupSTL conf;
    conf.Load(ix_->argv_[1]);
    bool bFlag = conf.FindElem("spp_report");
    assert(bFlag);
    conf.IntoElem();    
   
    if(!reload)
    {
        conf.ResetMainPos();
        bFlag = conf.FindElem("common");
        assert(bFlag);
        string comm_etc = conf.GetAttrib("pathname");
        assert(comm_etc != "" );
        
        CMarkupSTL commconf;
        commconf.Load(comm_etc.c_str());
        bFlag = commconf.FindElem("common");
        assert(bFlag);

        commconf.IntoElem();
        //初始化日志
        commconf.ResetMainPos();
        bFlag = commconf.FindElem("fwlog");
        assert(bFlag);
        
        string name_prefix ="spp_report";
        int log_level = atoi(commconf.GetAttrib("level").c_str());
        int log_type = atoi(commconf.GetAttrib("type").c_str());
        string log_path = commconf.GetAttrib("path");        
        int max_file_size = atoi(commconf.GetAttrib("maxfilesize").c_str());
        int max_file_num = atoi(commconf.GetAttrib("maxfilenum").c_str());
        int log_key_base = strtol(commconf.GetAttrib("key_base").c_str(),0,0);

        int semkey  = (log_key_base & 0xffff0000) |(0x0000ff02);
       
        assert((log_level >= LOG_TRACE) && (log_level <= LOG_NONE) &&
        		(log_type >= LOG_TYPE_CYCLE) && (log_type <= LOG_TYPE_CYCLE_HOURLY) &&
        		(max_file_size > 0) && (max_file_size <= 1024000000) && (max_file_num > 0));
        m_log_internal.LOG_OPEN(log_level, log_type, log_path.c_str(), name_prefix.c_str(), max_file_size, max_file_num,semkey);


    }

    
    if(!reload)
    {
        conf.ResetMainPos();
        assert(conf.FindElem("common"));
        string path_name = conf.GetAttrib("pathname");
        #ifdef OPEN_PRINT
            printf("common config file,pathname=%s\n",path_name.c_str());
        #endif
        assert( 0 == InitLoadTable(path_name.c_str()) );
    }


    //connector配置
    if(!reload)
    {
        //connector配置
        conf.ResetMainPos();
        bFlag = conf.FindElem("connector");
        assert(bFlag);

        string type = conf.GetAttrib("type");
        if(type == "socket")
        {
            int timeout = atoi(conf.GetAttrib("timeout").c_str());
            if(conf.FindChildElem("entry"))
            {
                string type = conf.GetChildAttrib("type");
                assert(type == "tcp");
                string ip = conf.GetChildAttrib("server");//连接的服务器IP
                unsigned short port = atoi(conf.GetChildAttrib("port").c_str());//端口

#ifdef OPEN_PRINT
                printf("in config,ip=%s,port=%d\n",ip.c_str(),port);
#endif

                if(TcpClient_)
                    delete TcpClient_;
                TcpClient_ = new CTcpClient(ip.c_str(),port,timeout);
                if( TcpClient_->IsValid() )
                {
#ifdef OPEN_PRINT
                    printf("ip=%s,port=%d,timeout=%d fd=%d\n",ip.c_str(),port,timeout,TcpClient_->GetFD() );
#endif
                }

                conf_.serverip_ = ip;
                conf_.serverport_ = port;

            }
        }
        else if(type =="shm")
        {
            //...
        }
    }

    //初始化负载上报信息
    //初始化负载上报时间间隔
    conf.ResetMainPos();
    bFlag = conf.FindElem("load");
    assert(bFlag);
    ix_->load_inter_time_ = atoi(conf.GetAttrib("interval").c_str());
    assert(ix_->load_inter_time_ > 0);

    std::string strIpAddress = conf.GetAttrib("ipaddress");
    if(strIpAddress != "" )
    {
        conf_.ipaddress_ = ntohl(inet_addr(strIpAddress.c_str()));
#ifdef OPEN_PRINT
        printf("ipaddress=%s ,inetaddr=%u\n",strIpAddress.c_str(),conf_.ipaddress_);
#endif
    }
    else
    {
        conf_.ipaddress_ = 0;
    }

    conf_.tcpport_ = atoi(conf.GetAttrib("tcpport").c_str());
    conf_.udpport_ = atoi(conf.GetAttrib("udpport").c_str());
    conf_.server_type_ = atoi(conf.GetAttrib("servertype").c_str());
    conf_.net_type_  = atoi(conf.GetAttrib("nettype").c_str());
    conf_.maxconn_num_ = atoi(conf.GetAttrib("maxconn").c_str());
    
    return 0;
}


}
}

