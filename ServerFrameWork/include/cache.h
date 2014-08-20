/********************************************
//文件名:cache.h
//功能:纯cache对象
//作者:钟何明
//创建时间:2009.06.11
//修改记录:

*********************************************/
#ifndef _COMM_SOCKCOMMU_CACHE_H_
#define _COMM_SOCKCOMMU_CACHE_H_

#include <sys/time.h>
#include "mempool.h"
#include "socket.h"


namespace comm 
{
namespace sockcommu
{

//纯cache对象
class CRawCache
{
public:
    CRawCache(CMemPool& mp);
    ~CRawCache();

    char* data();
    unsigned data_len();
    void append(const char* data, unsigned data_len);
    void skip(unsigned length);

private:
    //内存池对象
    CMemPool& _mp;
    //内存基址
    char* _mem;
    //内存大小
    unsigned _block_size;
    //实际使用内存起始偏移量
    unsigned _data_head;
    //实际使用内存长度
    unsigned _data_len;
};

//连接对象cache
class ConnCache
{
public:
    ConnCache(CMemPool& mp) : _flow(0), _fd(0), _access(0), _type(0), _r(mp), _w(mp){}
    ~ConnCache(){}

    //连接唯一标示	
    unsigned _flow;
    //相关fd
    int _fd;
    //时间戳
    time_t _access;
    //连接类型: TCP_SOCKET\UDP_SOCKET\UNIX_SOCKET
    int _type;
    //对端信息: 
    CSocketAddr _addr;
    //读请求cache
    CRawCache _r;
    //写回复cache
    CRawCache _w;
};
		
}
}
#endif

