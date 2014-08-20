/********************************************
//文件名:procmon.h
//功能:监控SERVER及监控CLIENT
//作者:钟何明
//创建时间:2009.06.11
//修改记录:

*********************************************/
#ifndef _TBASE_TPROCMON_H_
#define _TBASE_TPROCMON_H_
#include <unistd.h>
#include <assert.h>
#include <sys/msg.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <signal.h>
#include "list.h"
#include "loadtable.h"

using namespace std;
using namespace comm::load;

namespace spp 
{
namespace procmon 
{

#define MAX_SERVER_FLAG_LEN 64                 //服务标记最大的长度	
#define MAX_FILEPATH_LEN	128			//最大文件名路径
#define BUCKET_SIZE			10			//每进程组的哈希桶个数
#define MAX_PROC_GROUP_NUM  128			//最大进程组个数
#define MAX_MSG_BUFF        100			//消息缓冲区大小
#define MSG_EXPIRE_TIME		120			//消息过期时间
#define MSG_VERSION         0x01		//消息版本号
#define MSG_ID_SERVER       0x01        //SERVER发送的消息ID，CLIENT发送的消息ID大于MSG_ID_SERVER
#define MSG_SRC_SERVER      0x00		//发送者为SERVER
#define MSG_SRC_CLIENT		0x01		//发送者为CLIENT

#define PROCMON_EVENT_PROCDEAD		1	//进程失效
#define PROCMON_EVENT_OVERLOAD 	(1<<1)	//负载过大
#define PROCMON_EVENT_LOWSRATE	(1<<2)  //成功率过低
#define PROCMON_EVENT_LATENCY 	(1<<3)  //延迟太大
#define PROCMON_EVENT_OTFMEM   	(1<<4)	//使用内存过多
#define PROCMON_EVENT_PROCDOWN  (1<<5)  //进程数不足
#define PROCMON_EVENT_PROCUP  	(1<<6)  //进程数太多

#define PROCMON_CMD_KILL                  0x1		//杀死进程
#define PROCMON_CMD_LOAD                 0x2		//调整负载
#define PROCMON_CMD_FORK	        0x4		//创建进程

#define PROCMON_STATUS_OK          	0x0    //正常状态
#define PROCMON_STATUS_OVERLOAD     1      //负载太大
#define PROCMON_STATUS_LOWSRATE    (1<<1)  //成功率过低
#define PROCMON_STATUS_LATENCY     (1<<2)  //延迟太大
#define PROCMON_STATUS_OTFMEM      (1<<3)  //使用内存太多



/////////////////////////////外部可见数据结构///////////////////////////////////////		
typedef struct 
{
    int groupid_;						//进程组ID 
    int adjust_proc_time;
    char basepath_[MAX_FILEPATH_LEN];   //基本路径
    char exefile_[MAX_FILEPATH_LEN];	//进程可执行文件名
    char etcfile_[MAX_FILEPATH_LEN];	//进程配置文件名
    char server_flag_[MAX_SERVER_FLAG_LEN];   //进程标记
    int exitsignal_;					//kill进程的信号
    unsigned maxprocnum_;				//最大进程数
    unsigned minprocnum_;				//最小进程数
    unsigned heartbeat_;				//心跳超时时间
    unsigned maxwatermark_;				//最大负载水位
    unsigned short minsrate_;			//最低成功率
    unsigned short maxdelay_;			//最大延时
    unsigned maxmemused_;				//最大内存使用量
    char reserve_[8];
}TGroupInfo;//进程组信息

typedef struct 
{
    int groupid_;			//进程组序号
    int procid_;			//进程ID
    long timestamp_;		//时间戳
    unsigned watermark_;	//进程负载
    unsigned short srate_;	//成功率
    unsigned short delay_;	//平均延时
    unsigned memused_;      //使用内存
    unsigned curconnnum_;//当前的连接数
    char reserve_[8];
}TProcInfo;//进程信息

typedef struct 
{
    int groupid_;		//进程组序号
    int procid_;		//进程ID
    unsigned cmd_;		//命令ID
    int arg1_;			//参数1
    int arg2_;			//参数2
    char reserve_[8];
}TProcEvent;//事件通知

typedef struct
{
    long msgtype_;		//消息类型，指消息接收者（这里指进程ID）
    long msglen_; 		//消息长度
    long srctype_;		//消息发送者类型最低位0--server, 非0--client，其余位表示版本号
    long timestamp_;	//时间戳
    char msgcontent_[MAX_MSG_BUFF]; //此处存放TProcInfo或者TProcEvent信息
}TProcMonMsg;//服务器端与客户端通讯消息包

//////////////////////////////客户端与服务器端通讯方式///////////////////////////////////		
//通讯基类（由于是本地通讯，且数据量小，因此要求实现为非阻塞方式）
class CCommu
{
public:
    CCommu(){}
    virtual ~CCommu(){}
    //初始化
    //args:	指向参数的指针
    //返回值：0成功，否则失败
    virtual int init(void* args) = 0;
    //接收消息
    //msg:	存放接收的消息
    //msgtype:	消息类型 1发给服务器端的消息，>1发给客户端的消息
    //返回值:  >0接收的消息大小，否则失败
    virtual int recv(TProcMonMsg* msg, long msgtype = 1) = 0;
    //发送消息
    //msg:  要发送的消息
    //返回值:  0成功，否则失败
    virtual int send(TProcMonMsg* msg) = 0;
};

//消息队列通讯类（默认使用方式）
class CMQCommu : public CCommu
{
public:
    CMQCommu();
    CMQCommu(key_t mqkey);
    ~CMQCommu();
    int init(void* args);
    int recv(TProcMonMsg* msg, long msgtype = 0);
    int send(TProcMonMsg* msg);
protected:
    int mqid_;
};
//......扩展更多的通讯类，如udp, fifo, share memory等等.....


////////////////////////////////内部使用数据结构//////////////////////////////////////		
typedef struct 
{
    TGroupInfo groupinfo_;				//进程组信息
    list_head_t bucket_[BUCKET_SIZE];	//进程哈希桶数组
    int curprocnum_;				//当前进程数
    int errprocnum_;				//死亡进程数
    int last_check_group_time_;          //上次检查进程组中进程个数的时间
}TProcGroupObj;//进程组对象

typedef struct 
{
    TProcInfo procinfo_;		//进程信息
    int status_;				//进程状态
    int notifytime_;			//上次告警时间
    list_head_t list_;
}TProcObj;//进程对象

/////////////////////////////////服务器端接口///////////////////////////////////////	
typedef void (*monsrv_cb)(const TGroupInfo* groupinfo /*发出事件的进程所属进程组对象*/,
                                        const TProcInfo* procinfo   /*发出事件的进程对象*/, 
                                        int event /*事件*/, 
                                        void* arg /*自定义参数*/);
typedef struct 
{
    TProcGroupObj* group;  //进程组，存放指向进程组对象的指针
    TProcObj**	proc;	   //所属的进程集合，存放指向各个进程对象的指针，以NULL作为结束
}TProcQueryObj;

class CTProcMonSrv
{
public:
    CTProcMonSrv();
    virtual ~CTProcMonSrv();
    //设置通讯对象
    //commu:  通讯对象指针
    void set_commu(CCommu* commu);

    //初始化共享内存及信号锁
    //shmkey:共享内存的KEY
    //shmsize:共享内存的大小
    //semkey:信号锁的KEY
    void init_shm(int shmkey,int shmsize,int semkey);

    //设置公共配置文件路径
    //comm_cfg_file:配置文件路径
    void SetCommFilePath(const char * comm_cfg_file );

    void SetGroupNum(unsigned int val){group_num_ = val;}

    void SetCheckGroupInterval(unsigned int val){check_group_interval_ = val;}

    //notify:  事件通知回调函数
    void set_notify(monsrv_cb notify, void* arg);
    //加入进程组信息
    //groupinfo:	进程组信息
    //返回值: 0成功，否则失败
    int add_group(const TGroupInfo* groupinfo);
    //修改进程组信息
    //groupid:		进程组ID
    //groupinfo:	进程组信息
    //返回值: 0成功，否则失败
    int mod_group(int groupid, const TGroupInfo* groupinfo);
    //运行服务器端接收数据和检查各个进程组项和进程项
    void run();
    //取统计信息
    //buf:	统计信息缓冲区
    //buf_len:  统计信息长度
    void stat(char* buf, int* buf_len);	
    //向所有被监控的进程发信号
    void killall(int signo);
    //查询进程组和所有进程信息
    void query(TProcQueryObj*& result, int& num);  
protected:

    CLoadTable loadwriter_;
    string common_config_file_;
    unsigned int group_num_;//进程组总数
    unsigned int check_group_interval_;//检查组内进程数的时间间隔

    CCommu* commu_;	
    TProcGroupObj proc_groups_[MAX_PROC_GROUP_NUM];
    int cur_group_;
    TProcMonMsg msg_[2]; //0-收，1-发
    monsrv_cb notify_;   //使用者回调函数
    void* notify_arg_;   //回调函数的附加参数指针

    bool do_recv(long msgtype);
    bool do_check();
    int add_proc(int groupid, const TProcInfo* procinfo);
    TProcObj* find_proc(int groupid, int procid);
    void del_proc(int groupid, int procid);	
    //以下方法可以根据具体需要重载，但是最好还是要调用父类的同名方法
    virtual void check_group(TGroupInfo* group, int curprocnum);
    virtual bool check_proc(TGroupInfo* group, TProcInfo* proc);
    virtual bool do_event(int event, void* arg1, void* arg2);
    //virtual void do_fork(const char* basepath, const char* exefile, const char* etcfile, int num);
    //virtual void do_fork(const char * basepath,const char * exefile,const char * etcfile,const char * serv_flag,int num);
    virtual void do_fork(const char * basepath,const char * exefile,const char * etcfile,
               int groupid,int max_procnum,const char * serv_flag,int num);
    virtual void do_kill(int procid, int signo = SIGKILL);
    virtual void do_order(int groupid, int procid, int eventno, int cmd, int arg1 = 0, int arg2 = 0);
    bool check_groupbusy(int groupid);

};

//////////////////////////////////客户端接口///////////////////////////////////////////	
typedef void (*moncli_cb)(int event /*事件*/, 
						  void* arg /*自定义参数*/);
class CTProcMonCli
{
public:
    CTProcMonCli();
    virtual ~CTProcMonCli();
    //设置通讯对象
    //commu:  通讯对象指针
    void set_commu(CCommu* commu);
    //设置通讯对象
    //notify:  事件通知回调函数
    void set_notify(moncli_cb notify, void* arg);
    //发送数据到服务器端，并接收数据
    void run();
    TProcMonMsg msg_[2]; //0--发，1--收
protected:
    CCommu* commu_;	
    moncli_cb notify_;
    void*	notify_arg_;
};

#define CLI_SEND_INFO(cli)  ((TProcInfo*)(cli)->msg_[0].msgcontent_)  //TProcInfo，进程信息，客户端使用
#define CLI_RECV_INFO(cli)  ((TProcEvent*)(cli)->msg_[1].msgcontent_) //TProcEvent,  命令信息，客户端使用

}
}

#endif

