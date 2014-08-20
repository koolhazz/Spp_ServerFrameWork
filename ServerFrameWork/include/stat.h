/********************************************
//文件名:stat.h
//功能:统计池类
//作者:钟何明
//创建时间:2009.06.11
//修改记录:

*********************************************/
#ifndef _TBASE_TSTAT_H_
#define _TBASE_TSTAT_H_
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "stat_policy.h"

namespace spp
{
namespace stat
{
    
#define STAT_TYPE_SUM     1	            //累加操作 
#define STAT_TYPE_AVG     1<<1	   //平均操作
#define STAT_TYPE_MAX     1<<2	   //最大值操作
#define STAT_TYPE_MIN     1<<3	   //最小值操作
#define STAT_TYPE_COUNT   1<<4	   //计数操作
#define STAT_TYPE_UPDATE 1<<5   //更新操作
#define STAT_TYPE_ALL	  -1	            //通配	
#define STAT_TYPE_NUM     6	            //统计策略个数

#define TSTAT_MAGICNUM    83846584	    //统计池幻数 
#define TSTAT_VERSION	  0x01                    //统计版本号
#define BUCKET_NUM        100	            //统计对象桶个数
#define DEFAULT_STATOBJ_NUM  100	    //统计对象总个数
#define DEFAULT_STATVAL_NUM  (DEFAULT_STATOBJ_NUM*1)  //统计值总个数（一个统计对象至少有一个统计值）
#define STAT_BUFF_SIZE	  1<<14          //统计报表缓冲区大小
#define STAT_MAX_VALSIZE  10                    //统计值最大维数
#define INVALID_HANDLE	  -1

#define ERR_STAT_EXIST    -2000             //统计对象已经存在
#define ERR_STAT_NONE	  -2001	  //统计对象不存在
#define ERR_STAT_FULL	  -2010                //没有空闲内存存储统计对象
#define ERR_STAT_OPENFILE -2020	  //打开内存映射文件失败
#define ERR_STAT_TRUNFILE -2021	 //truncate文件失败
#define ERR_STAT_MAPFILE  -2030	 //映射文件失败
#define ERR_STAT_MEMERROR -2040	 //共享内存区数据损坏


///////////////////////////////////////////////////////////////////////////////////
typedef struct
{
    char id_[STAT_ID_MAX_LEN];            //统计ID
    char desc_[STAT_ID_MAX_LEN*2];   //统计描述
    int type_;                                              //统计类型
    int val_size_;                                        //统计值维数
    TStatVal count_;                                 //次数
    int val_offset_;                                    //统计值偏移量
    int next_;                                              //下一个TStatObj
}TStatObj; //统计对象

///////////////////////////////////////////////////////////////////////////////////
typedef struct
{
    long long magic_;                        //幻数
    int ver_;                                        //版本号
    //char buffer_[STAT_BUFF_SIZE];  //统计报表缓冲区 --不使用了
    int freelist_;                                  //空闲TStatObj链表
    int bucket_[BUCKET_NUM];       //TStatObj哈稀桶
    int statobjs_used_;                      //使用的TStatObj数目
    TStatObj statobjs_[DEFAULT_STATOBJ_NUM];  //TStatObj数组
    int statvals_used_;                        //使用的TStatVal数目
    TStatVal statvals_[DEFAULT_STATVAL_NUM];  //TStatVal数组
}TStatPool; //统计对象池

///////////////////////////////////////////////////////////////////////////////////
//统计池类
class CTStat
{
public:
    CTStat();
    ~CTStat();
    //初始化统计池, 当mapfile != NULL，则使用文件映射的共享内存，否则使用进程堆内存
    int init_statpool(const char* mapfilepath = NULL);

    //初始化统计策略
    //policy:  统计策略类
    //type:    统计策略类型（2的整数次幂）
    int init_statpolicy(CTStatPolicy* policy, int type);

    //初始化统计对象
    //id:	统计ID
    //type:	统计策略类型（可以复合）
    //desc: 统计描述
    //value_size: 统计对象统计值维数
    int init_statobj(const char* id, int type, const char* desc = NULL, int val_szie = 1);

    //一次统计（多个对象）
    //ids:	统计ID数组
    //num:	统计ID个数
    //val:  统计值分量
    //value_idx: 统计值维度
    int step(const char** ids, int num, long val, int val_idx = 1);

    //一次统计（单个对象）
    //id:	统计ID
    //val:  统计值分量
    //value_idx: 统计值维度
    int step0(const char* id, long val, int val_idx = 1);

    //生成统计结果
    //buffer: 指向统计报表缓冲区的指针
    //len:      统计报表长度
    void result(char** buffer, int* len);

    //生成监控上报结果集合
    //buffer: 指向统计报表缓冲区的指针
    //len:      统计报表长度
    void moni_result(char ** buffer,int * len );

    //重置所有统计对象
    void reset();

    //查询统计对象信息
    //id:       统计ID
    //wrapper:  统计对象信息
    int query(const char* id, TStatObjWrapper* wrapper);
protected:
    CTStatPolicy* policy_[STAT_TYPE_NUM];
    int policy_no_[STAT_TYPE_NUM];
    int policy_type_[1<<STAT_TYPE_NUM];
    TStatPool* statpool_;
    int policy_num_;
    bool mapfile_;
    
    inline unsigned hashid(const char* id)
    {
        int len = strlen(id);
        unsigned hash = 1315423911;
        for(int i = 0; i < len; ++i)
        {
            hash ^= ((hash << 5) + id[i] + (hash >> 2));
        }
        return hash % BUCKET_NUM;
    }
    int find_statobj(const char* id, int type = STAT_TYPE_ALL);
    int alloc_statobj(int val_size);
    void insert_statobj(int choice);
    void output_statpool(char* buffer, int* len);
    void moni_statpool(char * buffer,int *len);
    void output_statobj(const TStatObj* obj, long count, const long* values, int val_size, char* buffer, int* len);
    void newpool();
    typedef void (CTStat::*visit_func) (TStatObj*, void*);
    void travel(visit_func visitor, void* data = NULL);
    void do_reset(TStatObj* obj, void* data = NULL);
    void do_result(TStatObj* obj, void* data = NULL);
};

}
}
#endif

