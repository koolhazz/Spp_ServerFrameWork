#ifndef _SPP_PROXY_DEFAULT_H_
#define _SPP_PROXY_DEFAULT_H_
#include "serverbase.h"
#include "shmcommu.h"
#include "sockcommu.h"
#include "benchapiplus.h"
#include <vector>
#include <set>
#include <dlfcn.h>
#define IPLIMIT_DISABLE	  0x0
#define IPLIMIT_WHITE_LIST    0x1
#define IPLIMIT_BLACK_LIST    0x2

using namespace spp::base;
using namespace comm::commu;
using namespace comm::commu::shmcommu;
using namespace comm::sockcommu;
using namespace std;

namespace spp
{
namespace proxy
{
class CDefaultProxy : public CServerBase
{
public:
    CDefaultProxy();
    ~CDefaultProxy();

    //实际运行函数
    void realrun(int argc, char* argv[]);
    //服务容器类型
    int servertype() {return SERVER_TYPE_PROXY;}
    //初始化配置
    int initconf(bool reload = false);

    virtual int spp_get_group_num();

    virtual int spp_get_group_id();

    //一些回调函数
    static int ator_recvdata(unsigned flow, void* arg1, void* arg2);    //必要

    static int ctor_recvdata(unsigned flow, void* arg1, void* arg2);    //必要

    static int ator_overload(unsigned flow, void* arg1, void* arg2);    //非必要
    static int ator_connected(unsigned flow, void* arg1, void* arg2);   //非必要

    static int ator_timeout(unsigned flow, void* arg1, void* arg2);    //非必要
    static int ator_disconn(unsigned flow, void* arg1, void* arg2);    //非必要

    //static int ator_recvdone(unsigned flow, void* arg1, void* arg2);    //非必要
    //static int ctor_recvdone(unsigned flow, void* arg1, void* arg2);    //非必要



    //接受者
    CTCommu* ator_;
    //连接者
    vector<CTCommu*> ctor_;
    //ip限制类型
    unsigned char iplimit_;
    //ip集合
    set<unsigned> iptable_;    
    spp_handle_local_process_t local_handle; 

protected:
    virtual void do_timer_callback(const struct timeval & nowtime);

    struct IProxyParams;//代理的私有数据结构
    IProxyParams * IParam_;

};

}
}
#endif

