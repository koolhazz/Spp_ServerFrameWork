#ifndef _SPP_CTRL_DEFAULT_H_
#define _SPP_CTRL_DEFAULT_H_
#include "serverbase.h"
#include "MarkupSTL.h"
using namespace spp::procmon;
using namespace spp::base;
using namespace comm::base;

namespace spp
{
namespace ctrl
{
class CDefaultCtrl : public CServerBase
{
public:
    CDefaultCtrl();
    ~CDefaultCtrl();
    
    //实际运行函数
    void realrun(int argc, char* argv[]);
    //服务容器类型
    int servertype() {return SERVER_TYPE_CTRL;}

    //初始化配置
    int initconf(bool reload = false);
    
    //监控srv回调函数	
    static void notify(const TGroupInfo* groupinfo, const TProcInfo* procinfo, int event, void* arg);    
protected:
    int WritePidFile(const char * pszName);
    int WriteStatResult(const char * pszName,const char * buffer,unsigned int buf_len);

    int do_fork_ex(const char * basepath,const char * proc_name,const char * etcfile,const char * flag);

    virtual void do_timer_callback(const struct timeval & nowtime){};

    int InitProcMon(CMarkupSTL &conf,bool reload = false);

    //监控srv	
    CTProcMonSrv monsrv_;

        
    //统计结果输出路径
    string stat_output_file_;

    //统计结果输出的时间间隔
    unsigned int stat_output_interval_;

    //统计值重置时间[00:00:00 ~ 23:59:59]
    //string stat_reset_time_;
    int stat_reset_hour_;
    int stat_reset_minute_;
    int stat_reset_second_;

    //统计值重置的天数
    unsigned int stat_reset_days_;

};

}
}
#endif

