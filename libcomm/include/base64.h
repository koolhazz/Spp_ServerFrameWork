/********************************************
//文件名:base64.h
//功能:base64编码解码
//作者:钟何明
//创建时间:2009.06.11
//修改记录:

*********************************************/
#ifndef _BASE64_H_INCLUDED_
#define _BASE64_H_INCLUDED_

#include <string>

namespace comm
{
namespace util
{

/********************************************************
//功能:base64编码
//参数说明:
//[in] src:源数据
//[in] src_len:数据长度
//[out] dst:接收编码后数据的缓冲区
//返回值:无
*********************************************************/
void base64_encode(const char *src, int src_len, char *dst);

/********************************************************
//功能:base64解码
//参数说明:
//[in] src:源数据
//[in] src_len:数据长度
//[out] dst:接收解码后数据的缓冲区
//返回值:无
*********************************************************/
void base64_decode(const char *src, int src_len, char *dst);

void base64_encode(const char *src, int src_len, std::string &dst);
void base64_decode(const char *src, int src_len, std::string &dst);

inline void base64_encode(const std::string &src, std::string &dst)
{
    base64_encode(src.c_str(), src.size(), dst);
}

inline void base64_decode(const std::string &src, std::string &dst)
{
    base64_decode(src.c_str(), src.size(), dst);
}

}
}


#endif // BASE64_H_INCLUDED
