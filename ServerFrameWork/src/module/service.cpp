//必须包含spp的头文件
#include "sppinc.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "protocol_pack.h"
#include "hexdump.h"
#include "commu.h"

using namespace comm::commu;

#define MODULE_PROC_NUM	"module_proc_num"

//初始化方法（可选实现）
//arg1:	配置文件
//arg2:	服务器容器对象
//返回0成功，非0失败
extern "C" int spp_handle_init(void* arg1, void* arg2)
{
    //插件自身的配置文件
    const char* etc = (const char*)arg1;
    //服务器容器对象
    CServerBase* base = (CServerBase*)arg2;
    base->log_.LOG_P_LEVEL(LOG_DEBUG, "spp_handle_init, %s, %d\n", etc, base->servertype());    
    //建立一个统计项, 统计策略为累加
    base->stat_.init_statobj(MODULE_PROC_NUM, STAT_TYPE_SUM);    

    return 0;
}
//数据接收（可选实现）
//flow:	请求包标志
//arg1:	数据块对象
//arg2:	服务器容器对象
//返回正数表示数据已经接收完整且该值表示数据包的长度，
//0值表示数据包还未接收完整，负数表示出错
/*
extern "C" int spp_handle_input(unsigned flow, void* arg1, void* arg2)
{
    //数据块对象，结构请参考commu.h
    blob_type* blob = (blob_type*)arg1;
    //extinfo有扩展信息
    TConnExtInfo* extinfo = (TConnExtInfo*)blob->extdata;
    //服务器容器对象
    CServerBase* base = (CServerBase*)arg2;
    base->log_.LOG_P_LEVEL(LOG_DEBUG, "spp_handle_input, %d, %d, %s\n", flow, blob->len, inet_ntoa(*(struct in_addr*)&extinfo->remoteip_));    
    return blob->len;
}
*/
//路由选择（可选实现）
//flow:	请求包标志
//arg1:	数据块对象
//arg2:	服务器容器对象
//返回值表示worker的组号

extern "C" int spp_handle_route(unsigned flow, void* arg1, void* arg2)
{
    blob_type * blob = (blob_type * ) arg1;
    unsigned short      main,sub;
    protocol::GetPackCmd((const unsigned char * ) blob->data,main, sub);
    //服务器容器对象
    CServerBase* base = (CServerBase*)arg2;
    base->log_.LOG_P_LEVEL(LOG_DEBUG, "spp_handle_route, %d,main_cmd=%d,sub_cmd=%d\n", flow,main,sub);    
    return 1;    
}

//数据处理（必须实现）
//flow:	请求包标志
//pkg_source:包来源标记,如：1:客户端，2：后端服务等
//arg1:	数据块对象
//arg2:	服务器容器对象
//返回0表示成功，非0失败
extern "C" int spp_handle_process(unsigned flow, unsigned short pkg_source,void* arg1, void* arg2)
{

    //数据块对象，结构请参考commu.h
    blob_type* blob = (blob_type*)arg1;
    CServerBase * pServerBase = (CServerBase * ) arg2;    
    unsigned short      main,sub;    
    protocol::GetPackCmd((const unsigned char * )blob->data,main, sub);

    comm::util::HexDumpImp(blob->data,blob->len);

        //外网的ip及端口信息
    if(blob->ext_type == EXT_TYPE_CONNEXTINFO )
    {
        if(blob->ext_len >= sizeof(TConnExtInfo) )
        {
            TConnExtInfo * extInfo = (TConnExtInfo * ) blob->extdata;
    
            pServerBase->log_.LOG_P_LEVEL(LOG_DEBUG,"Get client Remote ip = %s,port=%d\n",inet_ntoa(*((struct in_addr*)&extInfo->remoteip_)),extInfo->remoteport_);
        }
        
    }

    //服务器容器对象
    CServerBase* base = (CServerBase*)arg2;
    base->log_.LOG_P_LEVEL(LOG_DEBUG, "spp_handle_route, %d,main_cmd=%d,sub_cmd=%d\n", flow,main,sub);    
    //数据来源的通讯组件对象
    CTCommu* commu = (CTCommu*)blob->owner;
    int ret = 0;
    base->log_.LOG_P_PID(LOG_DEBUG, "spp_handle_process, %d, %d\n", flow, blob->len);

    protocol::MsgHeader h;
    protocol::UnPacker upk;
    ret = upk.Init(blob->data,blob->len);
    if(ret != 0 )
    {
        base->log_.LOG_P_PID(LOG_ERROR,"%s:invalie packet!!!!flow=%u\n",__FUNCTION__,flow);
        return -1;
    }

#if 0
    upk.GetHeader(h);
    protocol::FixedPacker pk;
    pk.Init(blob->data,MAX_BLOB_DATA_LEN);
    pk.PackByte((unsigned char ) 0 );
    pk.PackByte((unsigned char ) 1 );
    int pack_len = 0;
    pk.GetPack(pack_len,h);
    blob->len = pack_len;
    #endif

    ret = commu->sendto(flow, arg1, NULL);
    if(unlikely(ret))
        base->log_.LOG_P_PID(LOG_ERROR, "base sendto error, %d\n", ret);

    //统计项加1
    base->stat_.step0(MODULE_PROC_NUM, 1);

    return 0;
}

//数据处理（可选实现）
//flow:	请求包标志
//arg1:	数据块对象
//arg2:	服务器容器对象
//返回0表示成功，非0失败
extern "C" int spp_handle_local_process(unsigned flow, void* arg1, void* arg2)
{

    //数据块对象，结构请参考commu.h
    blob_type* blob = (blob_type*)arg1;
    //服务器容器对象
    CServerBase* base = (CServerBase*)arg2;
    //数据来源的通讯组件对象
    CTCommu* commu = (CTCommu*)blob->owner;
    int ret = 0;
    base->log_.LOG_P_PID(LOG_DEBUG, "spp_handle_process, %d, %d\n", flow, blob->len);    

    ret = commu->sendto(flow, arg1, NULL);
    if(unlikely(ret))
        base->log_.LOG_P_PID(LOG_ERROR, "base sendto error, %d\n", ret);

    //统计项加1
    base->stat_.step0(MODULE_PROC_NUM, 1);

    return 0;
}


/*****************************************************************
//上下文超时回调
//v:队列中超时的上下文列表信息的KEY
//arg1:目前保留
//arg2:服务器容器对象(CServerBase *)
//返回0表示成功，非0失败
*****************************************************************/
#if 0
extern "C" int spp_context_timeout_list(const std::vector<string> & v,void * arg1,void * arg2)
{
    CServerBase * base = (CServerBase * ) arg2;
    TContext * context = NULL;

    //删除过时的上下文
    int nCount = v.size();
    for(int i = 0;i< nCount;++i)
    {
        string strKey = v[i];
        context = base->spp_get_context(strKey,true);
        if(context != NULL )
        {
            delete ((char *) context->data);
            delete context;
        }
    }

    return 0;
}
#endif

//析构资源（可选实现）
//arg1:	保留参数
//arg2:	服务器容器对象
extern "C" void spp_handle_fini(void* arg1, void* arg2)
{
    //服务器容器对象
    CServerBase* base = (CServerBase*)arg2;
    base->log_.LOG_P_LEVEL(LOG_DEBUG, "spp_handle_fini\n");
}

/***********************************************************
//功能:打印业务组件的版本号
//参数:无
//返回值:
//无
//说明：
************************************************************/
extern "C" void print_version()
{
    printf("Version:TestModule_v1.0.0 (linux)\n");
    printf("Build Date:%s\n",__DATE__);
}
