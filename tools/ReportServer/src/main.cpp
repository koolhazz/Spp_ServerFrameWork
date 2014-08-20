
/********************************
*文件名:main.cpp
*功能:main函数
*作者:钟何明
*修改日期:2009.06.10
*********************************/
#include "defaultreportsvr.h"
#include "misc.h"

using namespace spp::reportsvr;
using namespace comm::base;


int main(int argc,char * argv[])
{
    CServerBase * server = new CDefaultReportServer;
    server->run(argc,argv);

    return 0;
}


