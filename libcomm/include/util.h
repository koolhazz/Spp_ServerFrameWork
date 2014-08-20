/********************************************
//文件名:util.h
//功能:工具函数
//作者:钟何明
//创建时间:2009.08.14
//修改记录:

*********************************************/

#ifndef _COMM_UTIL_H_
#define _COMM_UTIL_H_

#include <sys/stat.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <string>

namespace comm
{
namespace util
{

/*
*功能:判断文件夹是否存在
*参数:
*[in] pszDir:文件夹的路径
*返回值:
*若文件夹存在返回true,否则false
*/
bool IsDirExist(const char *pszDir);

/*
*功能:判断文件是否存在
*参数:
*[in] pszName:文件的路径
*返回值:
*若文件存在返回true,否则false
*备注:文件包括普通文件，FIFO，目录，DEVICE等
*/
bool IsFileExist(const char *pszName);

/*
*功能:创建文件夹
*参数:
*[in] pszName:待创建的文件夹
*返回值:
*若文件夹存在返回true,否则false
*/
bool MkDir(const char *pszName, mode_t mode = 0755);


/*
*功能:将字符串转换为小写
*参数:
*[in] s:待转换的字符串
*返回值:
*转换后的字符串指针
*/
char *strlower(char *s);


/*
*功能:将字符串转换为大写
*参数:
*[in] s:待转换的字符串
*返回值:
*转换后的字符串指针
*/
char *strupper(char *s);

}
}

#endif
