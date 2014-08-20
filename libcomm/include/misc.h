
#ifndef _COMM_BASE_MISC_H_
#define _COMM_BASE_MISC_H_

#include <sys/time.h>



namespace comm 
{ 
namespace base 
{

//void get_time_task();
int get_timebytask(const char* tmfile="./time_mgrtask");
void get_timeofday(struct timeval* pval,const char* tmfile="./time_mgrtask");


#define DELAY_INIT  \
struct timeval begin;   \
get_timeofday(&begin); 


#define GET_DELAY  CMisc::getdelay(begin)

class CMisc
{
public:	
    static unsigned getip(const char* ifname);	
    static unsigned getdelay(const struct timeval& begin);
    static unsigned getmemused();
};

}
}
#endif

