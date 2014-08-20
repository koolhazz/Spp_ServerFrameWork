/********************************************
//文件名:load.h
//功能:负载对象
//作者:钟何明
//创建时间:2009.06.11
//修改记录:

*********************************************/
#ifndef _COMM_COMMU_LOAD_H_
#define _COMM_COMMU_LOAD_H_

#include <asm/atomic.h>
#include <time.h>
#include <sys/time.h>

namespace comm
{
namespace commu 
{

#define DEFAULT_MAX_LOAD	(1<<16)		//默认每秒最大load值
#define DEFAULT_INTERVAL	5			//计算周期（秒）
#define LOADGRID_NUM     	2			//使用的计数单元个数，防止溢出


class CLoad
{
public:
    CLoad();
    CLoad(int maxload);
    ~CLoad();

    //设置最大负载(每秒)	
    int maxload(int n);	
    //增加或者减少负载
    void grow_load(int n);
    //检查是否过载
    bool check_load();
    //取当前负载(每秒)
    int peek_load(struct timeval* nowtime = NULL);
protected:
    void reset();
    void trans();
    	
    int maxload_;
    int curtime_;
    bool direct_;
    atomic_t* curloadgrid_;
    atomic_t loadgrid_[LOADGRID_NUM];
    atomic_t loadgrid2_[LOADGRID_NUM];
};

}
}
#endif 

