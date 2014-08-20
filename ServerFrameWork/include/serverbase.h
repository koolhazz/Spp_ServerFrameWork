
/***************************************
*文件名:serverbase.h
*功能:服务基类
*创建者:钟何明
*创建时间:2009.06.10
*修改记录
****************************************/
#ifndef _SPP_COMM_SERVERBASE_H_
#define _SPP_COMM_SERVERBASE_H_
#include <sys/resource.h>
#include <unistd.h>
#include <signal.h>
#include <list>
#include "log.h"			//日志
#include "stat.h"			//统计
#include "procmon.h"		//监控
#include "likelydef.h"
#include <string>
#include <vector>
#include <map>
#include <assert.h>

using namespace std;

using namespace comm::log;
using namespace spp::stat;
using namespace spp::procmon;

namespace spp 
{
namespace base 
{

#ifndef SIGUSR3
#define SIGUSR3 (SIGRTMIN + 1)
#endif

#define RUN_FLAG_QUIT		0x01
#define RUN_FLAG_RELOAD		0x02
#define RUN_FLAG_RELOAD_MODULE 0x04

#define SERVER_TYPE_UNKNOWN 0x00
#define SERVER_TYPE_PROXY   0x01
#define SERVER_TYPE_WORKER  0x02
#define SERVER_TYPE_CTRL	0x04

const unsigned short PKG_SOURCE_CLIENT = 1;//从客户端过来的包
const unsigned short PKG_SOURCE_SERVER = 2;//从后端服务器过来的包


//前端通讯事件回调函数类型
typedef enum 
{
    ET_CONNECTED = 0,	//连接建立
    ET_DISCONNECT,		//连接断开			
    ET_OVERLOAD,		//过载
    ET_TIMEOUT,		        //连接超时(在指定时间内没有收到数据)
}event_type;

typedef enum
{
    EVENT_CONNECTED = 0,//连接建立
    EVENT_DISCONNECT,//连接断开
    EVENT_OVERLOAD,//过载
    EVENT_TIMEOUT,//连接超时
}back_event_type;


//定时回调函数类型
//arg1:通用参数指针1，一般指向数据blob
//arg2:通用参数指针2,指向服务容器对象
//arg3:通用参数指针3,用户注册回调时候传入的参数指针
typedef void * (*cb_timer_func)(void * arg1,void * arg2,void * arg3);

//事件回调函数类型
//flow: 数据包唯一标示
//arg1: 通用参数指针1，一般指向数据blob
//arg2:通用参数指针2,指向服务容器对象
//arg3: 通用参数指针3，用户注册回调函数传入的自定义参数指针
//返回值:0成功，否则失败
typedef int (*cb_event_func)(unsigned int flow,void * arg1,void * arg2,void * arg3);

//worker后端通讯事件回调函数类型
//serv_type:服务类型
//flow: 数据包唯一标识
//arg1: 通用参数指针1，一般指向数据blob
//arg2:通用参数指针2,指向服务容器对象
//arg3: 通用参数指针3，用户注册回调函数传入的自定义参数指针
//返回值:0成功，否则失败
typedef int (*cb_back_event_func)(unsigned int serv_type,unsigned int flow,void * arg1,void * arg2,void * arg3);

//////////////////////////////////////////////////////////////
#if 0
typedef struct tagTContext
{
    unsigned int timestamp_;//时间戳
    unsigned int timeout_;//超时时间
    unsigned int data_len;//data的长度
    void * data;//用户数据指针，内存由使用者分配及释放
}TContext;

/*********************************************************
//上下文超时回调类型
//v:队列中超时的上下文列表信息的KEY
//arg1:目前保留
//arg2:服务器容器对象(CServerBase *)
//返回0表示成功，非0失败
*********************************************************/
typedef int (*cb_context_timeout_list)(const std::vector<string> & v,void * arg1,void * arg2);

//通用的上下文队列
class CContextQueue
{
public:
    CContextQueue(){max_len_ = 10000;}
    ~CContextQueue(){}

    int SetMaxQueueLen(unsigned int len){max_len_ = len;return 0;}

    //队列相关操作
    int AddItem(const string & key,const TContext * context);
    TContext * GetItem(const string & key,bool bRemove = false);

    //取得超时的列表
    int GetTimeOutList(std::vector<string> & v);


protected:
    typedef std::map<std::string,TContext * > MapContext;
    typedef MapContext::iterator Iter;
    MapContext ctx_map_;

    unsigned int max_len_;

};

#endif

/////////////////////////////////////////////////////////////

//服务器程序基础类，包含运行环境初始化、日志、统计、监控对象
class CServerBase
{
public:
    CServerBase();
    virtual ~CServerBase();
    virtual void ShowVerInfo();
    virtual void run(int argc, char* argv[]);
    virtual void startup(bool bg_run = true);
    //实际的运行函数
    virtual void realrun(int argc, char* argv[]){}
    //返回服务容器的类型
    virtual int servertype(){return SERVER_TYPE_UNKNOWN;}


    /***********************************************************
    //发送数据给后端服务器
    //serv_type:服务类型
    //route_key:包路由的key,框架通过该值来选择通讯链路
    //arg1:通用参数指针，一般指向blob对象
    //arg2::自定义参数,目前保留
    //返回值:>=0成功表示发送的字节数，<0失败
    ************************************************************/
	virtual int spp_sendto(unsigned int& to_flow, unsigned int serv_type, unsigned int route_key,void * arg1, void * arg2) {return -1;}

//    virtual int spp_sendto(unsigned int serv_type,unsigned int route_key,void * arg1,void * arg2) {return -1;}

    /***********************************************************
    //功能:通过flow发送数据给后端服务器
    //flow:通讯唯一标识
    //arg1:通用参数指针，一般指向blob对象
    //arg2::自定义参数,目前保留
    //返回值:>=0成功表示发送的字节数，<0失败
    ************************************************************/
    virtual int spp_sendbyflow(unsigned int flow,void * arg1,void * arg2){return -1;}

    /***********************************************************
    //功能:取得Group数量
    //参数:无
    //返回值:
    //>=0:成功，< 0:失败
    ************************************************************/
    virtual int spp_get_group_num(){return -1;}

    /***********************************************************
    //功能:取得当前进程所属的组号
    //参数:无
    //返回值:
    //>=0:成功，< 0:失败
    ************************************************************/
    virtual int spp_get_group_id(){return -1;}

    /*****************************************************************
    //保存上下文
    //key:上下文的唯一标识
    //context:用户自定义上下文
    //返回值:0成功，<0失败
    ******************************************************************/
    //virtual int spp_set_context(const string & key,const TContext * context);

    /*****************************************************************
    //获取上下文
    //key:上下文的唯一标识
    //bRemove:是否从队列中移除上下文信息
    //返回值：成功：key所指向的上下文指针，失败：NULL
    ******************************************************************/
    //TContext * spp_get_context(const string & key,bool bRemove);

    //注册timer
    //interval:调用间隔时间,单位为毫秒
    //func: 回调函数
    //args: 用户自定义参数指针, 作为回调函数的通用参数传递
    //返回值:0成功，否则失败
    int  reg_timer_proc(int interval,cb_timer_func func,void * args = NULL);

    //注册事件通知回调
    //et:事件类型
    //func:回调函数
    //args: 用户自定义参数指针, 作为回调函数的第3个通用参数传递
    //返回值:0成功，非0失败
    int reg_event_proc(event_type et,cb_event_func func,void * args = NULL);

    //注册后端通讯事件通知回调
    //et:事件类型
    //func:回调函数
    //args: 用户自定义参数指针, 作为回调函数的第3个通用参数传递
    //返回值:0成功，非0失败
    int reg_back_event_proc(back_event_type et,cb_back_event_func func,void * args = NULL);

    //日志
    CLog log_;
    //统计
    CTStat stat_;
    //监控
    CTProcMonCli moncli_;	
    //内存分配器。。。等等其它设施	
	
protected:

    virtual void do_timer_callback(const struct timeval & nowtime) = 0;

    struct TInternal;
    struct TInternal* ix_;

    //框架使用日志
    CLog m_log_internal;

    typedef struct tagTimerInfo
    {
        int interval_;
        cb_timer_func func_;
        void * args_;
        struct timeval proctime_;//上次调用的时间
    }TimerInfo,*LPTimerInfo;

    typedef std::list<LPTimerInfo> TimerList;
    typedef TimerList::iterator TimerListItr;
    TimerList timerlist_;

    cb_event_func  event_func_list_[ET_TIMEOUT + 1];
    void *	event_func_args_[ET_TIMEOUT + 1];

    cb_back_event_func back_event_func_list_[EVENT_TIMEOUT + 1];
    void * back_event_func_args_[EVENT_TIMEOUT + 1];
public:
    ///////////////////////////////////////////////////////////////////////
    static bool reload();
    static bool quit();
    static bool reloadmoduleconfig();
    static void sigusr1_handle(int signo);
    static void sigusr2_handle(int signo);
    static void sigusr3_handle(int signo);
    static unsigned char flag_;
    static char version_desc[64];
};

#ifdef _DEBUG_STD_
#define FW_DBG_STD(fmt, args...)  do{printf(fmt, ##args);printf("\n");}while(0)
#else
#define FW_DBG_STD(fmt, args...) 
#endif

#define FW_LOG_DBG(fmt, args...) do{m_log_internal.LOG_P_LEVEL(LOG_DEBUG,fmt,##args);FW_DBG_STD(fmt,##args);}while(0)
#define FW_LOG_INFO(fmt, args...) do{m_log_internal.LOG_P_LEVEL(LOG_NORMAL,fmt,##args);FW_DBG_STD(fmt,##args);}while(0)
#define FW_LOG_ERR(fmt, args...) do{m_log_internal.LOG_P_LEVEL(LOG_ERROR,fmt,##args);FW_DBG_STD(fmt,##args);}while(0)
#define FW_LOG_FATAL(fmt, args...) do{m_log_internal.LOG_P_LEVEL(LOG_FATAL,fmt,##args);FW_DBG_STD(fmt,##args);}while(0)
#define FW_LOG_BIN                      m_log_internal.LOG_P_BIN

}
}
#endif

