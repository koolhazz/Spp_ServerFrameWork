/***************************************
*文件名:defaultreportsvr.h
*功能:负载上报进程应用类
*创建者:钟何明
*创建时间:2009.06.10
*修改记录
****************************************/
#ifndef _SPP_DEF_RPT_SVR_
#define _SPP_DEF_RPT_SVR_

#include "serverbase.h"
#include "loadtable.h"
#include "base_socket.h"

using namespace comm::load;
using namespace comm::basesock;

namespace spp
{
namespace reportsvr
{

//配置参数
class CReportServerConf
{
public:
	CReportServerConf();
	~CReportServerConf();


	std::string serverip_;//LoadBalance Server Ip Address
	unsigned short serverport_;//LoadBalance Server Port
	
	unsigned int ipaddress_;//IP地址
	unsigned short tcpport_;//TCP PORT
	unsigned short udpport_;//UDP PORT
	unsigned short server_type_;//SERVER TYPE
	unsigned char net_type_;//NET TYPE
	unsigned int maxconn_num_;//最大连接数	
};

class CDefaultReportServer:public CServerBase
{
public:
	CDefaultReportServer();
	virtual ~CDefaultReportServer();

	virtual void realrun(int argc, char * argv [ ]);

	//初始化配置
	int initconf(bool reload = false);


protected:

        int InitLoadTable(const char * comm_config);

	CLoadTable loadreader_;

	//TcpClient组件
	CTcpClient  * TcpClient_;

	CReportServerConf conf_;

};


}
}

#endif

