#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "default_update_client.h"
#include "MarkupSTL.h"
#include "likelydef.h"
#include "protocol_pack.h"
#include "hexdump.h"
#include "util.h"

using namespace protocol;
using namespace comm::base;
using namespace comm::util;

namespace spp
{
namespace base
{

#define UPK_HEADER(upk, blob, h)  do\
                                    {\
                                    int ret = upk.Init(blob->data, blob->len);\
                                    if(ret != 0)\
                                    {\
                                        m_log_internal.LOG_P_LEVEL(LOG_ERROR,"[%s:%d] Invalid pack.ret=%d, blob_len=%d", __FUNCTION__, __LINE__, ret, blob->len);\
                                        return -1;\
                                    }\
                                    upk.GetHeader(h);\
                                    }while(0)

const unsigned int MAX_BUFFER_LEN = (1<<24); //16M
static char SBUFFER[MAX_BUFFER_LEN];

const unsigned short CFG_UPDATE_NOTIFY = 0x0700;
const unsigned short CFG_CHECK_VERSION = 0x0701;
const unsigned short CFG_GET_CONFIGINFO = 0x0702;

static unsigned int GetReqSeq()
{
    static unsigned int seq = 0;
    return seq++;
}

static int CreateGetVerRequest(char * buf,int len,std::vector<unsigned int> ServerTypeList)
{

    FixedPacker pk;
    int ret = pk.Init(buf,len);
    if(ret != 0 )
        return ret;

    MsgHeader h;
    h.main_cmd = CFG_CHECK_VERSION;
    h.seq = GetReqSeq();

    unsigned short check_count = ServerTypeList.size();
    pk.PackWord(check_count);
    for(int i = 0;i< check_count;++i)
    {
        pk.PackDWord(ServerTypeList[i]);
    }


     int pack_len = 0;
     pk.GetPack(pack_len,h);

     return pack_len;
    
}

static int CreateGetConfigRequest(char * buf,int len,std::vector<unsigned int> ServerTypeList)
{
    FixedPacker pk;
    int ret = pk.Init(buf,len);
    if(ret != 0 )
        return ret;

    MsgHeader h;
    h.main_cmd = CFG_GET_CONFIGINFO;
    h.seq = GetReqSeq();

    unsigned short get_count = ServerTypeList.size();
    pk.PackWord(get_count);
    for(int i = 0;i< get_count;++i)
    {
        pk.PackDWord(ServerTypeList[i]);
    }


     int pack_len = 0;
     pk.GetPack(pack_len,h);

     return pack_len;
}

int HandleInput(const char *buf, unsigned int len)
{
	if(len < 7)
	{
		return 0;
	}
	unsigned int *pn = (unsigned int *)(buf+3);
	return ntohl(*pn);
}


CDefaultUpdateClient::CDefaultUpdateClient():TcpClient_(NULL)
{

}

CDefaultUpdateClient::~CDefaultUpdateClient()
{
	if(TcpClient_)
		delete TcpClient_;
	TcpClient_ = NULL;
}

int CDefaultUpdateClient::SendBlob(blob_type * blob)
{  

    if(TcpClient_ == NULL )
    {
        TcpClient_ = new CTcpClient(m_strServerip.c_str(),m_nServerport,1000);       
    }

     if(blob->len == 0 )
        return 0;

    unsigned int nWrite = 0;
    int ret = TcpClient_->SendN(nWrite,(void *) blob->data,blob->len);

    if( ret <= 0 )
    {
        if( 0 == TcpClient_->Reconnect(1000) )
        {
            ret = TcpClient_->SendN(nWrite,(void *) blob->data,blob->len);
        }
        
    }

    if(ret <= 0 )
    {
        m_log_internal.LOG_P_LEVEL(LOG_ERROR,"%s:Send Errno,ret=%d\n",__FUNCTION__,ret);
    }

    return ret;
}
void CDefaultUpdateClient::realrun(int argc, char * argv [ ])
{
    //初始化配置
    initconf(false);

    unsigned int nowtime, last_check_time;
    nowtime = last_check_time = time(NULL);

    while(true)
    {
        nowtime = time(NULL);

        if( TcpClient_ != NULL   )
        {
            //DoRecv
            unsigned int nRead = 0;
            int nRecvByte = TcpClient_->RecvPack(HandleInput,(unsigned int) MSG_HEADER_LEN,nRead,(void *) SBUFFER,MAX_BUFFER_LEN,100,0);
            if(nRecvByte > 0 )
            {
                //HexDumpImp(SBUFFER,nRecvByte);
                //recv ok
                blob_type blob;
                blob.len = nRecvByte;
                blob.data = SBUFFER;
                blob.owner = (void *) TcpClient_;
                OnProcess(&blob);
            }
            else
            {
                if(nRecvByte != -2)
                {
                    //RECONNECT
                    if( 0 != TcpClient_->Reconnect(1000) )
                    {
                        delete TcpClient_;
                        TcpClient_ = new CTcpClient(m_strServerip.c_str(),m_nServerport,1000); 
                    }
                }
            }


            if( (nowtime - last_check_time) >= (unsigned int) m_nCheckVerInterval )
            {
                //DoCheck
                blob_type blob;
                blob.len = 0;
                blob.data = SBUFFER;
                blob.owner = (void *) TcpClient_;
                this->DoCheckVersion(&blob);
                last_check_time = nowtime;
            }
        				
        }
        else
        {
            //printf("error,TcpClient Is Null\n");
            m_log_internal.LOG_P_LEVEL(LOG_ERROR,"error,TcpClient Is Null\n");
        }

        usleep(1000);//休眠0.1秒


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

    }

}

int CDefaultUpdateClient::initconf(bool reload /*= false*/)
{
    CMarkupSTL conf;
    conf.Load(ix_->argv_[1]);
    bool bFlag = conf.FindElem("updateconfig");
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
        
        string name_prefix ="spp_update";
        int log_level = atoi(commconf.GetAttrib("level").c_str());
        int log_type = atoi(commconf.GetAttrib("type").c_str());
        string log_path = commconf.GetAttrib("path");        
        int max_file_size = atoi(commconf.GetAttrib("maxfilesize").c_str());
        int max_file_num = atoi(commconf.GetAttrib("maxfilenum").c_str());
        int log_key_base = strtol(commconf.GetAttrib("key_base").c_str(),0,0);

        int semkey  = (log_key_base & 0xffff0000) |(0x0000ff01);
       
        assert((log_level >= LOG_TRACE) && (log_level <= LOG_NONE) &&
        		(log_type >= LOG_TYPE_CYCLE) && (log_type <= LOG_TYPE_CYCLE_HOURLY) &&
        		(max_file_size > 0) && (max_file_size <= 1024000000) && (max_file_num > 0));
        m_log_internal.LOG_OPEN(log_level, log_type, log_path.c_str(), name_prefix.c_str(), max_file_size, max_file_num,semkey);

    }

    conf.ResetMainPos();

    //connector配置
    if(!reload)
    {
        //connector配置
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

                if(TcpClient_)
                    delete TcpClient_;
                TcpClient_ = new CTcpClient(ip.c_str(),port,timeout);
                if( TcpClient_->IsValid() )
                {
#ifdef OPEN_PRINT
                    printf("ip=%s,port=%d,timeout=%d fd=%d\n",ip.c_str(),port,timeout,TcpClient_->GetFD() );
#endif
                }

                m_strServerip = ip;
                m_nServerport = port;

            }
        }
        else if(type =="shm")
        {
            //...
            assert(false);
        }
        else
        {
            assert(false);
        }
    }


    conf.ResetMainPos();

    if(conf.FindElem("routetable"))
    {
        string strPathName = conf.GetAttrib("etc");
        m_nCheckVerInterval = strtol(conf.GetAttrib("check_interval").c_str(),0,0);
        CMarkupSTL routeconf;
        routeconf.Load(strPathName.c_str());
        if(routeconf.FindElem("routetable"))
        {
            m_strRouteTableBasePath = routeconf.GetAttrib("basepath");
            m_strRouteTablePrefix = routeconf.GetAttrib("prefix");
            routeconf.IntoElem();

            unsigned int serv_type;
            unsigned int version;
            char route_table[1024];

            while(routeconf.FindElem("group"))
            {
                memset(route_table,0,sizeof(route_table));
                serv_type = strtol(routeconf.GetAttrib("serv_type").c_str(),0,0);
                version = 0;
                sprintf(route_table,"%s/%s_%x.xml",m_strRouteTableBasePath.c_str(),m_strRouteTablePrefix.c_str(),serv_type);
                if(GetRouteTableVersion(version,route_table) != 0 )
                    version = 0;

                m_mapConfigVersion[serv_type] = version;
            }
        }
    }
    
    return 0;
}


int CDefaultUpdateClient::OnProcess(blob_type * blob)
{
    UnPacker upk;
    MsgHeader h;
    UPK_HEADER(upk, blob, h);

    int ret = 0;
    
    switch(h.main_cmd)
    {
        case CFG_UPDATE_NOTIFY:
        {
            ret = this->OnUpdateConfigNotify(blob);
         }
         break;
         
         case CFG_CHECK_VERSION:
         {
            ret = this->OnCheckVersionRsp(blob);
         }
         break;

         case CFG_GET_CONFIGINFO:
         {
            ret = this->OnGetConfigInfoRsp(blob);
         }
         break;
         
        default:
        {
            ret = -1;
        }
        break;

    }          

    return ret;

}

 int CDefaultUpdateClient::DoCheckVersion(blob_type * blob)
 {
    std::vector<unsigned int > ServerList;
    ITER itr = m_mapConfigVersion.begin(),last = m_mapConfigVersion.end();
    for(;itr != last; ++itr)
    {
        ServerList.push_back(itr->first);
    }
    int pack_len = CreateGetVerRequest(blob->data,MAX_BUFFER_LEN,ServerList);
    if(pack_len <= 0 )
    {
        return -1;
    }

    blob->len = pack_len;
    SendBlob(blob);
    return 0;
 }

int CDefaultUpdateClient::OnCheckVersionRsp(blob_type * blob)
{
    UnPacker upk;
    MsgHeader h;
    UPK_HEADER(upk, blob, h);
    unsigned short server_num = upk.UnPackWord();
    std::vector<unsigned int > ServerList;
    for(int i = 0;i< server_num;++i)
    {
        unsigned int serv_type = upk.UnPackDWord();
        unsigned int version = upk.UnPackDWord();
        ITER itr = m_mapConfigVersion.find(serv_type);
        if(itr != m_mapConfigVersion.end() )
        {
            if(version != itr->second )
            {
                ServerList.push_back(serv_type);
            }
        }
    }

    if(ServerList.size() > 0 )
    {
        int pack_len = CreateGetConfigRequest(blob->data,MAX_BUFFER_LEN,  ServerList );
        if(pack_len < 0 )
        {
            return -1;
        }

        blob->len = pack_len;       

        SendBlob(blob);
        
    }
    
    return 0;
}

int CDefaultUpdateClient::OnGetConfigInfoRsp(blob_type * blob)
{
    UnPacker upk;
    MsgHeader h;
    UPK_HEADER(upk, blob, h);    
    //HexDumpImp(blob->data,blob->len);

    unsigned short server_num = upk.UnPackWord();

    MkDir( m_strRouteTableBasePath.c_str() );
    for(int i = 0;i< server_num;++i)
    {
        unsigned int server_type = upk.UnPackDWord();
        unsigned int version = upk.UnPackDWord();
        string strConfig = upk.UnPackString();
        
        ITER itr = m_mapConfigVersion.find(server_type);
        if(itr != m_mapConfigVersion.end() )
        {
            m_mapConfigVersion[server_type] = version;
            //写文件
            char buf[1024];
            sprintf(buf,"%s/%s_%08x.xml",m_strRouteTableBasePath.c_str(),m_strRouteTablePrefix.c_str(),server_type);
            int fd = open(buf, O_WRONLY | O_CREAT , S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
            if(fd == -1)
            {
                m_log_internal.LOG_P_LEVEL(LOG_ERROR,"open file fail,file=%s,errno=%d,msg=%s\n",buf,errno,strerror(errno));
                continue;
            }
            
            m_log_internal.LOG_P_LEVEL(LOG_DEBUG,"write route table config file to:%s\n",buf);

           ssize_t size = write(fd, strConfig.c_str(),strlen(strConfig.c_str() ) );
           if( size == -1 )
           {
                m_log_internal.LOG_P_LEVEL(LOG_ERROR,"write to file:%s fail,errno=%d\n",buf,errno);
                continue;
           }
            
        }
        
        m_log_internal.LOG_P_LEVEL(LOG_DEBUG,"servertype=%08x,version = %08x,config=%s\n",server_type,version,strConfig.c_str());
    }

    //通知Ctrl更新配置
    if(server_num > 0 )
    {
        //Notify Ctrl Update RouteTable Config
    }
    return 0;
}

int CDefaultUpdateClient::OnUpdateConfigNotify(blob_type * blob)
{
    UnPacker upk;
    MsgHeader h;
    UPK_HEADER(upk, blob, h);
    //HexDumpImp(blob->data,blob->len);
    unsigned short server_num = upk.UnPackWord();
    std::vector<unsigned int > ServerList;
    for(int i = 0;i< server_num;++i)
    {
        unsigned int serv_type = upk.UnPackDWord();
        unsigned int version = upk.UnPackDWord();
        ITER itr = m_mapConfigVersion.find(serv_type);
        if(itr != m_mapConfigVersion.end() )
        {
            if(version != itr->second )
            {
                ServerList.push_back(serv_type);
            }
        }
    }

    if(ServerList.size() > 0 )
    {
        int pack_len = CreateGetConfigRequest(blob->data,MAX_BUFFER_LEN,  ServerList );
        if(pack_len < 0 )
        {
            printf("%s:CreateGetConfigRequest return fail!!!\n",__FUNCTION__);
            return 0;
        }

        blob->len = pack_len;
        SendBlob(blob);        
    }
    return 0;
}


int CDefaultUpdateClient::GetRouteTableVersion(unsigned int & version, const char * file)
{
    CMarkupSTL routetable;
    routetable.Load(file);
    if(routetable.FindElem("routetable"))
    {
        version = strtol(routetable.GetAttrib("version").c_str(),0,0);
        return 0;
    }
    
    return -1;
}

}
}

