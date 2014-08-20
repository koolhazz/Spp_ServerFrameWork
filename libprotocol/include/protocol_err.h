/********************************
*文件名：protocol_err.h
*功能：协议错误码定义
*作者：张荐林
*版本：1.0
*最后更新时间：2009.05.20
**********************************/


#ifndef _PROTOCOL_ERR_H_
#define _PROTOCOL_ERR_H_

#ifdef _WIN32
#define _WIN32_WINNT 0x0400
#include <hash_map>
using namespace stdext;
#else
#include <ext/hash_map>
using namespace __gnu_cxx;
#endif

//#include <ext/hash_map>
//using namespace __gnu_cxx;

#include <string>
using std::string;

namespace protocol
{

const unsigned int RET_OK = 0;
/*协议错误*/
const unsigned int ERR_PROTOCOL = 1;
/*不支持的加密方式*/
const unsigned int ERR_ENC_TYPE = 2;
/*不支持的版本号*/
const unsigned int ERR_PROTOCOL_VER = 3;
/*操作数据库错误*/
const unsigned int ERR_DB = 4;
/*网络错误*/
const unsigned int ERR_NETWORK = 5;


//DB批量操作相关错误码
//0x0000ff00 ~ 0x0000fffe
//DB批量操作时，部分操作失败
const unsigned int ERR_DB_PART = 0x0000ff00;



//未知错误
const unsigned int IM_ERR_UNKNOWN = 0x0000FFFF;


/*
*错误码描述的注册与查询
*/
class ErrInfoMng
{
public:

	/*根据错误码查找对应的错误描述*/
	static string GetErrInfo(unsigned int err_code)
	{
		hash_map<unsigned int, string>::iterator it = m_ErrInfo.find(err_code);		
		 return (it == m_ErrInfo.end()) ? string(""):it->second;
	}

	/*注册错误码的描述信息*/
	static int RegErrInfo(unsigned int err_code, const string &err_info)
	{
		m_ErrInfo[err_code] = err_info;
		return 0;
	}

    /*注册错误码的描述信息*/
	static int RegErrInfo(unsigned int err_code, const char* err_info)
	{
		m_ErrInfo[err_code] = err_info;
		return 0;
	}

private:
	//static __gnu_cxx::hash_map<unsigned int, string> m_ErrInfo;
    static hash_map<unsigned int, string> m_ErrInfo;
};


/*
example:
	ErrInfoMng::RegErrInfo(ERR_IM_AUTH, "Auth error");
	string err_info = ErrInfoMng::GetErrInfo(ERR_IM_AUTH);
*/

}

#endif



