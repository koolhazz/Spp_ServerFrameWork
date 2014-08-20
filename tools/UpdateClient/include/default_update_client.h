/***************************************
*文件名:default_update_client.h
*功能:配置更新程序应用类
*创建者:钟何明
*创建时间:2009.06.10
*修改记录
****************************************/
#ifndef _SPP_DEF_UPDATE_SVR_
#define _SPP_DEF_UPDATE_SVR_

#include <map>

#include "serverbase.h"
#include "base_socket.h"

using namespace std;
using namespace comm::basesock;

namespace spp
{
namespace base
{

typedef struct 
{
    int len;
    char * data;
    void * owner;
}blob_type;


class CDefaultUpdateClient:public CServerBase
{
public:
	CDefaultUpdateClient();
	virtual ~CDefaultUpdateClient();

	virtual void realrun(int argc, char * argv [ ]);

	//初始化配置
	int initconf(bool reload = false);

	int SendBlob(blob_type * blob);

protected:
        int OnProcess(blob_type * blob);
        int DoCheckVersion(blob_type * blob);
        int OnUpdateConfigNotify(blob_type * blob);
        int OnCheckVersionRsp(blob_type * blob);
        int OnGetConfigInfoRsp(blob_type * blob);

        int GetRouteTableVersion(unsigned int & version, const char * file);


protected:	

        std::string m_strServerip;// Server Ip Address
	unsigned short m_nServerport;//Server Port
        unsigned int m_nCheckVerInterval;//检查新版本的时间间隔 
        std::string m_strRouteTableBasePath;//路由表存储的目录
        std::string m_strRouteTablePrefix;//路由表文件的前缀

        //first:serv_type,second:version
        typedef  std::map<unsigned int,unsigned int> MapConfigVer;
        typedef MapConfigVer::iterator ITER;
        MapConfigVer  m_mapConfigVersion;

        //TcpClient组件
	CTcpClient  * TcpClient_;
    

};


}
}

#endif

