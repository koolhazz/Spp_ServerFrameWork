/********************************************
//文件名:sockcommu.h
//功能:SOCKET通讯类
//作者:钟何明
//创建时间:2009.06.11
//修改记录:

*********************************************/
#ifndef _COMM_SOCKCOMMU_SOCKCOMMU_H_
#define _COMM_SOCKCOMMU_SOCKCOMMU_H_
#include <asm/atomic.h>
#include <time.h>
#include "commu.h"
#include <vector>
#include <string>
#include <map>
using namespace std;

//#define LOAD_CHECK_ENABLE
#ifdef LOAD_CHECK_ENABLE
#include "load.h"
#endif



using namespace comm::commu;

namespace comm 
{
namespace sockcommu 
{

#define SOCK_TYPE_TCP		0x1
#define SOCK_TYPE_UDP		0x2
#define SOCK_TYPE_UNIX		0x4
#define SOCK_MAX_BIND			100

const unsigned char  SOCK_FLAG_UNKNOW = 0x00;
const unsigned char SOCK_FLAG_SERVER = 0x01;
const unsigned char SOCK_FLAG_CLIENT = 0x02;

typedef struct 
{
    unsigned int ip_;			//ip address
    unsigned short port_;		//ip port
}TIpport;

typedef struct 
{
    int type_;					//tcp or udp or unix socket
    union
    {
        TIpport ipport_;        //ip and port
        char path_[256];		//unix socket path
    };
}TSockBind;	

typedef struct tagTSockCommuConf
{
    tagTSockCommuConf()
    {
        maxconn_ = 0;
        maxpkg_ = 0;
        expiretime_ = 0;
        check_expire_interval_ = 0;	
    }
    TSockBind sockbind_[SOCK_MAX_BIND]; //绑定信息
    int maxconn_;		//最大连接数 > 0
    int maxpkg_; 		//最大包量，如果为0则不检查
    int expiretime_;		//超时时间， > 0
    int check_expire_interval_;//检查超时的时间间隔，默认60秒,0不检查
}TSockCommuConf;


//////////////////////////////////////
//路由节点信息
typedef struct
{
    void Show() const
    {
        printf("serv_type=%u,node_id=%u,begin_=%d,end=%d\n",serv_type_,node_id_,begin_,end_);
    }
    unsigned int serv_type_;//所属的服务类型
    unsigned int node_id_;//节点ID
    unsigned int begin_;//路由段的起始值
    unsigned int end_;//路由段的结束值
    TSockBind bind_;//服务器的绑定信息,ip,port
}TNodeInfo;

typedef std::map<unsigned int,TNodeInfo> MapNode;
typedef MapNode::iterator NodeIter;
//路由表信息
typedef struct tagTRouteTable
{
    unsigned int serv_type_;//服务类型
    string version_;//当前版本号
    string route_;//路由算法
    int route_val_;//对route_运算的操作数

    int GetNodeInfo(TNodeInfo & info,unsigned int node_id);	

    unsigned int GetNodeID(unsigned int node_key);	

    void Show();

    MapNode  map_node_;
}TRouteTable;

typedef std::map<unsigned int,TRouteTable> MapRouteTable;
typedef MapRouteTable::iterator RouteTableIter;

//后端服务连接配置信息
typedef struct tagTRouteConfig
{
    tagTRouteConfig()
    {
        max_conn_ = 1000;
        maxpkg_ = 0;
        expiretime_ = 60;
        check_expire_interval_ = 60;
    }
    ~tagTRouteConfig(){}

    //获取节点信息
    //serv_type:服务类型
    //node_id:节点ID
    //返回值:成功,指向节点信息的指针，失败NULL
    int GetNodeInfo(TNodeInfo & node,unsigned int serv_type,unsigned int node_id);	

    int GetRouteTable(TRouteTable & tbl,unsigned int serv_type);

    int max_conn_;//最大连接数
    int maxpkg_; 	//每秒最大包量，如果为0则不检查
    int expiretime_;//超时时间>0
    int check_expire_interval_;//检查超时时间间隔

    //服务信息
    MapRouteTable map_route_table_;
}TRouteConfig;



//必须注册CB_RECVDATA回调函数
class CTSockCommu : public CTCommu
{
public:
    CTSockCommu();
    ~CTSockCommu();

    /*
    * 功能:初始化
    * 参数说明:
    * [in] config:配置结构信息内存指针  
    * 返回值:
    * 0:成功,-1失败
    */
    int init(const void* config);

    /*
    * 功能:轮询收数据
    * 参数说明:
    * [in] block:是否阻塞，目前暂时没有用到
    * 返回值:
    * 0:成功,<0失败
    */
    int poll(bool block = false);

    /*
    * 功能:发数据
    * 参数说明:
    * [in] flow: 连接唯一标识
    * [in] arg1:数据块blob对象
    * [in] arg2:目前保留
    * 返回值:
    * 0:成功,<0失败
    */
    int sendto(unsigned flow, void* arg1, void* arg2);

    /*
    * 功能:控制
    * 参数说明:
    * [in] flow:连接唯一标识
    * 返回值:
    * 0:成功,-1失败
    */
    int ctrl(unsigned flow, ctrl_type type, void* arg1, void* arg2);


    /*
    *功能:初始化后端连接
    *参数说明:
    *[in] config:配置结构的内存地址
    *返回值:
    * 0成功，非0失败
    */
    int InitExt(const void * config);

    /*
    *功能:连接服务器
    *参数说明:
    *[in] pNode:连接的节点信息
    *返回:=0 成功，否则失败
    */
    int connect(const TNodeInfo * pNode);	


protected:

    TSockBind sockbind_[SOCK_MAX_BIND];

    int maxconn_;
    int expiretime_;
    int check_expire_interval_;

    int lastchecktime_;
    int maxbindfd_;
    unsigned flow_;
    blob_type buff_blob_;
    TConnExtInfo extinfo_;
#ifdef LOAD_CHECK_ENABLE			
    CLoad myload_;
#endif
    struct TInternal;
    struct TInternal* ix_;	
    		 	
    void fini();
    void check_expire();
    int create_sock(const TSockBind* s);
    int create_sock(int sock_type);

    void handle_accept(int fd);
    void handle_accept_udp(int fd);
};

}
}
#endif

