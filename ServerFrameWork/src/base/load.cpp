#include <stdio.h>
#include "load.h"
#include "misc.h"

using namespace comm::commu;
using namespace comm::base;

CLoad::CLoad():maxload_(DEFAULT_MAX_LOAD), curtime_(0)
{
    reset();
}
CLoad::CLoad(int maxload):curtime_(0)
{
    if(maxload > 0)
        maxload_ = maxload;
    else
        maxload_ = DEFAULT_MAX_LOAD;

    reset();
}
CLoad::~CLoad()
{
}
void CLoad::reset()
{
    for(int i = 0; i < LOADGRID_NUM; ++i)
    {
        atomic_set(&loadgrid_[i], 0);
        atomic_set(&loadgrid2_[i], 0);
    }
    direct_ = true;
    curloadgrid_ = loadgrid_;
}
void CLoad::trans()
{
    if(direct_)
    {
        direct_ = false;
        curloadgrid_ = loadgrid2_; 
    }
    else
    {
        direct_ = true;
        curloadgrid_ = loadgrid_;
    }
    
    for(int i = 0; i < LOADGRID_NUM; ++i)
    {
        atomic_set(&curloadgrid_[i], 0);
    }
}
int CLoad::maxload(int n)
{
    if(n > 0)
        maxload_ = n;

    return maxload_;
}
void CLoad::grow_load(int n)
{
    static const int base = 1000000 / LOADGRID_NUM;
    struct timeval now;
    //gettimeofday(&now, NULL);
    get_timeofday(&now, NULL);
    int idx = now.tv_usec / base;
    atomic_add(n, &curloadgrid_[idx]);
}
bool CLoad::check_load()
{
    struct timeval now;
    ///gettimeofday(&now, NULL);
    get_timeofday(&now, NULL);
    bool overload = false;
    if(peek_load(&now) > maxload_)
        overload = true;
    else
        overload = false;

    return overload;
}
int CLoad::peek_load(struct timeval* nowtime)
{

    int timediff = 0;
    if(!nowtime)
    {
        struct timeval now;
        //gettimeofday(&now, NULL);
        get_timeofday(&now, NULL);
        timediff = now.tv_sec - curtime_;    
    }
    else
    {
        timediff = nowtime->tv_sec - curtime_;
    }
    if(timediff >= DEFAULT_INTERVAL)
    {
        trans();
        curtime_ += timediff;
    }
    atomic_t* curloadgrid = NULL;
    if(direct_)
    {
        curloadgrid = loadgrid2_;    
    }
    else
    {
        curloadgrid = loadgrid_;
    }
    int curload = 0;
    for(int i = 0; i < LOADGRID_NUM; ++i)
    {
        curload += atomic_read(&curloadgrid[i]);
    }
    return curload / DEFAULT_INTERVAL;
}


