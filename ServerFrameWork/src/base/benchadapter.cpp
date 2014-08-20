#include <stdarg.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <errno.h>
#include <assert.h>
#include <sys/types.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <unistd.h>
#include <time.h>
#include <stdio.h>
#include <dlfcn.h>
#include "protocol_pack.h"
#include "benchadapter.h"
#include "benchapi.h"
#include "benchapiplus.h"
#include "commu.h"
#include "hexdump.h"

using namespace comm::commu;
using namespace comm::util;

/////////////////////////////////////////////////////////////////////////////////
//so library
////////////////////////////////////////////////////////////////////////////////
spp_dll_func_t sppdll = {NULL};

int log_init (const char* dir, int lvl, u_int size, const char* pre_name);

/*****************************************************************************
//数据接收（必须实现）
//flow:	请求包标志
//arg1:	数据块对象
//arg2:	服务器容器对象
//返回正数表示数据已经接收完整且该值表示数据包的长度，
//0值表示数据包还未接收完整，负数表示出错
*******************************************************************************/
extern "C"  int default_handle_input(unsigned flow, void* arg1, void* arg2)
{
    const unsigned int MSG_HEADER_LEN = 29;
    blob_type* blob = (blob_type*)arg1;
    if( (unsigned int) blob->len <= MSG_HEADER_LEN)
    {
        //包还没有收完
        return 0;
    }


    unsigned int *p = (unsigned int *)(blob->data + 3);
    unsigned int packlen = ntohl(*p);
    #ifdef OPEN_PRINT
    printf("%s:pack_len=%d,blob_len=%d\nBegin print Packet:\n",__FUNCTION__,packlen,blob->len);
    HexDumpImp(blob->data,blob->len);
    printf("end print of packet\n");
    #endif

    if( (unsigned int)blob->len < packlen)
    {
        return 0;
    }

    return packlen;
}

/***********************************************************
//共享内存管道满（可选实现）
//flow:	请求包标志
//group_id:内存管道的分组ID
//arg1:	数据块对象
//arg2:	服务器容器对象(CServerBase *)
//返回值:
// 0:表示继续保留在框架中
// <0:表示将请求包丢弃，并关闭连接
// >0:表示将请求包丢弃，不关闭连接
//说明:该接口仅proxy需要调用，若无该接口，框架默认将数据保留
//例外:当proxy中用于缓存socket数据的内存池用完后，会丢弃该socket所有缓存的数据，并关闭连接
************************************************************/
extern "C" int default_handle_queue_full(unsigned int flow,unsigned int group_id, void* arg1, void* arg2)
{
    return 0;
}

/*******************************************
//获取心跳包接口 (可选实现）
********************************************/
//serv_type:服务类型
//arg1:	数据块对象
//arg2:	服务器容器对象(CServerBase *)
//返回值0表示成功，非0失败
//说明：框架在需要发送心跳信息时，会调用该接口获取心跳数据包，业务应该在数据块中填充心跳包然后返回
//说明：目前框架中已有默认的实现，若不是特殊的业务数据包，则不需要实现
extern "C"  int default_handle_heartbeat(unsigned int serv_type,void * arg1,void * arg2)
{
    blob_type* blob = (blob_type*)arg1;
    static unsigned int seq = 0;

    static protocol::MsgHeader header;
    static protocol::Packer pack;
    const unsigned short IM_KEEP_ALIVE = 0x0005;

    pack.Reset();
    header.main_cmd = IM_KEEP_ALIVE;
    header.seq = ++seq;

    string strMsg = pack.GetPack(header);

    memcpy(blob->data,strMsg.data(),strMsg.size());
    blob->len = strMsg.size();

    return 0;
    
}

/*******************************************
//事件路由接口 (可选实现）
********************************************/
//flow:数据包唯一标志
//event:事件类型
//arg1:	数据块对象
//arg2:	服务器容器对象(CServerBase *)
//返回值:
//0表示将网络事件路由到所有的组
//>0表示将网络事件路由到指定的组
//<0表示不关心该网络事件
extern "C" int default_handle_event_route(unsigned int flow,unsigned int event,void * arg1,void * arg2)
{
    return -1;
}


int load_bench_adapter(const char* file)
{

    if(unlikely(sppdll.handle != NULL))
    {
        dlclose(sppdll.handle);
    }

    memset(&sppdll, 0x0, sizeof(spp_dll_func_t));

    void* handle = dlopen(file, RTLD_NOW);
    //重试三次
    int nRetryCnt = 3;
    while(nRetryCnt-- && handle == NULL )
    {
        printf("dlerro info=%s,retry...\n",dlerror());
        handle = dlopen(file, RTLD_NOW);
    }

    assert(handle != NULL);

    sppdll.spp_handle_init = (spp_handle_init_t)dlsym(handle, "spp_handle_init");

    sppdll.spp_handle_input = (spp_handle_input_t)dlsym(handle,"spp_handle_input");
    if(sppdll.spp_handle_input == NULL)
    {
        sppdll.spp_handle_input = (spp_handle_input_t)default_handle_input;
    }

    sppdll.spp_handle_route = (spp_handle_route_t)dlsym(handle, "spp_handle_route");

    sppdll.spp_handle_queue_full = ( spp_handle_queue_full_t) dlsym(handle,"spp_handle_queue_full");
    if(sppdll.spp_handle_queue_full == NULL )
    {
        sppdll.spp_handle_queue_full = (spp_handle_queue_full_t ) default_handle_queue_full;
    }

    sppdll.spp_handle_reloadconfig = (spp_handle_reloadconfig_t) dlsym(handle,"spp_handle_reloadconfig");

    sppdll.spp_handle_event_route = (spp_handle_event_route_t) dlsym(handle,"spp_handle_event_route");
    if(sppdll.spp_handle_event_route == NULL )
    {
        sppdll.spp_handle_event_route = (spp_handle_event_route_t) default_handle_event_route;
    }

    sppdll.spp_handle_process = (spp_handle_process_t)dlsym(handle, "spp_handle_process");

    sppdll.spp_handle_fini = (spp_handle_fini_t)dlsym(handle, "spp_handle_fini");

    assert(sppdll.spp_handle_input != NULL && sppdll.spp_handle_process != NULL );
    sppdll.handle = handle;
    return 0;
}


