
/********************************
*文件名:main.cpp
*功能:main函数
*作者:钟何明
*修改日期:2009.06.10
*********************************/
#include "default_update_client.h"
#include "misc.h"

using namespace spp::base;
using namespace comm::base;


int main(int argc,char * argv[])
{
	CDefaultUpdateClient * pUpdateClient = new CDefaultUpdateClient;
	pUpdateClient->run(argc,argv);	
	return 0;
}


