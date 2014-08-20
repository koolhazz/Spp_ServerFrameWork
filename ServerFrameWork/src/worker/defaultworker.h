#ifndef _SPP_WORKER_DEFAULT_H_
#define _SPP_WORKER_DEFAULT_H_
#include <dlfcn.h>
#include <algorithm>

#include "serverbase.h"
#include "shmcommu.h"
#include "sockcommu.h"
#include "benchapiplus.h"
#include "commumng.h"



using namespace spp::base;
using namespace comm::commu;
using namespace comm::commu::shmcommu;
using namespace comm::sockcommu;

namespace spp
{
namespace worker 
{
class CDefaultWorker : public CServerBase
{
public:
    CDefaultWorker();
    ~CDefaultWorker();

    //实际运行函数
    void realrun(int argc, char* argv[]);
    //服务容器类型
    int servertype() {return SERVER_TYPE_WORKER;}
    //初始化配置
    int initconf(bool reload = false);

	virtual int spp_sendto(unsigned int& to_flow, unsigned int serv_type, unsigned int route_key,void * arg1, void * arg2);

//    virtual int spp_sendto(unsigned int serv_type, unsigned int route_key,void * arg1, void * arg2);

    virtual int spp_sendbyflow(unsigned int flow, void * arg1, void * arg2);


    virtual int spp_get_group_num();

    virtual int spp_get_group_id();

    //一些回调函数
    static int ator_recvdata(unsigned flow, void* arg1, void* arg2);	//必要
    static int ator_senddata(unsigned flow, void* arg1, void* arg2);	//非必要
    static int ator_overload(unsigned flow, void* arg1, void* arg2);	//非必要

    static int ator_connected(unsigned flow, void* arg1, void* arg2);   //非必要
    static int ator_timeout(unsigned flow, void* arg1, void* arg2);	//非必要
    static int ator_disconn(unsigned flow, void* arg1, void* arg2);	//非必要


    //connector的一些回调函数
    static int ctor_recvdata(unsigned flow,void * arg1,void * arg2);
    static int ctor_senddata(unsigned flow,void * arg1,void * arg2);

    static int ctor_overload(unsigned flow, void* arg1, void* arg2);	//非必要
    static int ctor_connected(unsigned flow, void* arg1, void* arg2);   //非必要

    static int ctor_timeout(unsigned flow, void* arg1, void* arg2);	//非必要
    static int ctor_disconn(unsigned flow, void* arg1, void* arg2);	//非必要

    static int ctor_connect(unsigned int flow,void * arg1,void * arg2);//必要

protected:

    //重写基类的超时回调
    virtual void do_timer_callback(const struct timeval & nowtime );


    /*************************************
    //初始化路由表
    //TRouteTable:路由表信息	
    //route_cfg:路由表文件
    //返回值:成功0，-1失败
    **************************************/
    int InitRouteTable(TRouteTable & route_tbl,const string & route_cfg);

    int InitRouteConfig(TRouteConfig * config,const string & route_file,bool reload);


    //接受者
    CTCommu* ator_;

    //后端连接管理
    CCommuMng commu_mng_;

    //连接者
    CTCommu * ctor_;


    TRouteConfig * route_config_;//路由配置信息

};

}
}
#endif

