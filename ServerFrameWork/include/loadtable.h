/****************************************
*文件名:loadtable.h
*功能:负载表对象
*创建者:钟何明
*修改记录:
*****************************************/
#ifndef _COMM_LOAD_TAB_H_
#define _COMM_LOAD_TAB_H_

#include <string>
#include <map>
#include "lock.h"

using namespace comm::lock;

namespace comm
{
namespace load
{

const unsigned char MAX_KEY_LEN = 32;//KEY的最大长度

typedef struct tagItemInfo
{
    unsigned short len_;//item len
    unsigned pos_;//item pos
}ItemInfo;
typedef  std::map<std::string,ItemInfo> MapItems;
typedef MapItems::iterator Iter;


//共享内存中的负载信息表
class CLoadTable
{
public:
    CLoadTable();
    ~CLoadTable();

        /*
    *功能:初始化共享内存块
    *参数说明:
    *[in] shmkey:共享内存的key
    *[in] shmsize:共享内存的大小
    *[in] semkey:信号量的key
    *返回值:
    *  成功:0,失败:非0
    */
    int init(int shmkey,int shmsize,int semkey = -1);

        /*
    *功能:初始化负载项
    *参数说明:
    *key:负载项的KEY
    *len:负载项需要使用的最大储存空间，单位字节
    *返回值:
    *成功:>0,表示该负载项实际的存储空间,失败:<=0
    */
    int Init_LoadItem(const char * key,unsigned short len);

    /****************************************
    //从共享内存中读取数据到buf中
    //buf:接收数据的BUF
    //buf_len:buf缓冲区的长度
    //key:
    //timeout:过期时间，单位秒
    //返回值:>=0:实际读取数据的长度，<0读取失败
    *******************************************/
    int read(void * buf,unsigned short buf_len,const char * key,int timeout = 60);

        /*********************************************
    *写数据到共享内存中
    *key:负载项的KEY,必须先初始化后使用
    *buf:待写入的buf
    *len:buf的长度
    *返回值:
    *>=0:实际写入的长度，<0写入失败	
    *********************************************/
    int write(const char * key,void * buf,unsigned short len);

protected:

    //查找块block
    //key:
    //len:数据块长度
    //返回:
    //成功:相对数据块的偏移量
    unsigned findblock(const char * key,unsigned short &len);
    	
    //shmkey:
    //shmsize:
    //return:>=0成功，<0:失败
    int getmemory(int shmkey,int shmsize);
    void fini();

    int shmkey_;//IPC KEY
    int shmsize_;//共享内存大小
    int shmid_;
    void * shmmem_;//attached share memory
    unsigned * tail_;//数据块尾部
    char * block_;//数据块
    unsigned blocksize_;//数据块大小

    MapItems items_;

    CLock *lock_;

};

}
}


#endif

