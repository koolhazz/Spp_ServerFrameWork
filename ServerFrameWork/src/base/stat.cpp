#include <stdio.h>
#include <time.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/syscall.h>
#include "stat.h"
#include "misc.h"

#if !__GLIBC_PREREQ(2, 3)    
#define __builtin_expect(x, expected_value) (x)
#endif
#ifndef likely
#define likely(x)  __builtin_expect(!!(x), 1)
#endif
#ifndef unlikely
#define unlikely(x)  __builtin_expect(!!(x), 0)
#endif

using namespace spp::stat;
using namespace comm::base;

#define OBJ_INIT(obj, id, type, desc)   \
do {    \
    memset(obj->id_, 0x0, sizeof(obj->id_));    \
    strncpy(obj->id_, id, sizeof(obj->id_) - 1);    \
    if(desc)    \
        strncpy(obj->desc_, desc, sizeof(obj->desc_) - 1);  \
    else    \
        memset(obj->desc_, 0x0, sizeof(obj->desc_));    \
    obj->type_ = type;  \
    obj->next_ = INVALID_HANDLE;    \
}while(0)
        
#define OBJ_ENTRY(obj, ent)	obj = &statpool_->statobjs_[ent];
#define OBJ_WRAPPER(obj, wrapper) \
do {    \
    wrapper.id_ = obj->id_;     \
    wrapper.desc_ = obj->desc_; \
    wrapper.type_ = obj->type_; \
    wrapper.val_size_ = obj->val_size_; \
    wrapper.count_ = &obj->count_;  \
    wrapper.value_ = statpool_->statvals_ + obj->val_offset_;   \
}while(0) 

typedef struct
{
    char* buffer;
    int*  len; 
}cdata;
 
CTStat::CTStat() : statpool_(NULL), policy_num_(0), mapfile_(false)
{
    memset(policy_, 0x0, STAT_TYPE_NUM * sizeof(long));
    memset(policy_no_, 0x0, STAT_TYPE_NUM * sizeof(int));
    memset(policy_type_, 0x0, (1<<STAT_TYPE_NUM) * sizeof(int));
        
    init_statpolicy(new CTStatPolicySum, STAT_TYPE_SUM);
    init_statpolicy(new CTStatPolicyAvg, STAT_TYPE_AVG);
    init_statpolicy(new CTStatPolicyMax, STAT_TYPE_MAX);
    init_statpolicy(new CTStatPolicyMin, STAT_TYPE_MIN);
    init_statpolicy(new CTStatPolicyCount, STAT_TYPE_COUNT);
    init_statpolicy(new CTStatPolicyUpdate,STAT_TYPE_UPDATE);
}
CTStat::~CTStat()
{
    for(int i = 0; i < STAT_TYPE_NUM; ++i)
    {
        if(policy_[i])
            delete policy_[i];
    }

    if(statpool_)
    {
        if(mapfile_)
        {
            munmap(statpool_, sizeof(TStatPool));
        }
        else
        {
            free(statpool_);
        }
    }
}
int CTStat::init_statpool(const char* mapfilepath)
{
    if(!mapfilepath)
    {
        mapfile_ = false;    
        statpool_ = (TStatPool*)malloc(sizeof(TStatPool));
        newpool();
    }
    else
    {
        mapfile_ = true;
        bool fileexist = false;

        if(!access(mapfilepath, R_OK))
            fileexist = true;

        int fd =  open(mapfilepath, O_CREAT | O_RDWR, 0666);
        if(unlikely(fd < 0))
            return ERR_STAT_OPENFILE;
        
    	if(unlikely(!fileexist && ftruncate(fd, sizeof(TStatPool)) < 0))
        {
            close(fd);
            return ERR_STAT_TRUNFILE;
        }

        statpool_ = (TStatPool*)mmap(NULL, sizeof(TStatPool), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
        if(unlikely(MAP_FAILED == statpool_))
        {
            close(fd);
            statpool_ = NULL;
            return ERR_STAT_MAPFILE;
        }
        
        close(fd);    

        if(fileexist)
        {
            if(unlikely(statpool_->magic_ != TSTAT_MAGICNUM))
            {
                close(fd);
                munmap(statpool_, sizeof(TStatPool));
                statpool_ = NULL;
                return ERR_STAT_MEMERROR;
            }
        }
        else 
        {
            newpool();    
        }
    }    
    
    return 0;
}
int CTStat::init_statpolicy(CTStatPolicy* policy, int type)
{
    assert(policy_num_ < STAT_TYPE_NUM);         
    int i = 0; 
    int j = type;
    while((j = (j >> 1))) 
        ++i;
    policy_no_[i] = type;
    policy_[i] = policy; 
    policy_type_[type] = i;
    policy_num_++;
    return 0;
}
int CTStat::init_statobj(const char* id, int type, const char* desc, int val_size)
{
    if(find_statobj(id) != INVALID_HANDLE)
        return ERR_STAT_EXIST;

    TStatObj* obj = NULL;
    int choice = INVALID_HANDLE;

    for(int i = 0; i < STAT_TYPE_NUM; ++i)
    {        
        if(type & policy_no_[i])
        {
            choice = alloc_statobj(val_size);
            if(choice != INVALID_HANDLE)
            {
                OBJ_ENTRY(obj, choice);
                OBJ_INIT(obj, id, policy_no_[i], desc);
                insert_statobj(choice);
            }
            else
                return ERR_STAT_FULL;
        }
    }
    return 0;
}
int CTStat::step(const char** ids, int num, long val, int val_idx)
{
    TStatObj* obj = NULL;
    TStatObjWrapper wrapper;
    int choice = INVALID_HANDLE;
    int policy_no = 0;
    for(int i = 0; i < num; ++i)
    {
        choice = find_statobj(ids[i]);
        while(choice != INVALID_HANDLE)
        {
            OBJ_ENTRY(obj, choice); 
            if(!strcmp(obj->id_, ids[i]))
            {    
                policy_no = policy_type_[obj->type_];
                OBJ_WRAPPER(obj, wrapper);    
                policy_[policy_no]->__step__(&wrapper, val, val_idx);
                choice = obj->next_;
            }
            else
                break;    
        }
    }
    return 0;
}
int CTStat::step0(const char* id, long val, int val_idx)
{
    return step(&id, 1, val, val_idx);    
}
void CTStat::result(char** buffer, int* len)
{
    *len = 0;    
    output_statpool(*buffer, len);
    cdata buff = {*buffer, len};    
    travel(&CTStat::do_result, &buff);
}

void CTStat::moni_result(char **  buffer, int * len)
{
    *len = 0;    
    moni_statpool(*buffer, len);
    cdata buff = {*buffer, len};    
    travel(&CTStat::do_result, &buff);
}

void CTStat::reset()
{
    travel(&CTStat::do_reset);
}
int CTStat::query(const char* id, TStatObjWrapper* wrapper)
{
    int choice = find_statobj(id, STAT_TYPE_ALL); //fixed me! defect, only return the first statobj matched 
    if(likely(choice != INVALID_HANDLE))
    {
        TStatObj* obj = NULL;
        OBJ_ENTRY(obj, choice);
        OBJ_WRAPPER(obj, (*wrapper));
        return 1;
    }
    else
        return ERR_STAT_NONE;
}
void CTStat::travel(visit_func visitor, void* data)
{
    int next = 0;
    for(int i = 0; i < BUCKET_NUM; ++i)
    {
        next = statpool_->bucket_[i];
        TStatObj* obj = NULL;
        while(1)
        {
            if(INVALID_HANDLE == next)
                break;
            else
            {
                OBJ_ENTRY(obj, next);
                (this->*visitor)(obj, data);
                next = obj->next_;
            } 
        }
    }
}
void CTStat::do_reset(TStatObj* obj, void* data)
{
    TStatObjWrapper wrapper;
    int policy_no = 0;
    policy_no = policy_type_[obj->type_];
    OBJ_WRAPPER(obj, wrapper);    
    policy_[policy_no]->__reset__(&wrapper);    
}
void CTStat::do_result(TStatObj* obj, void* data)
{
    int policy_no = 0;
    long values[STAT_MAX_VALSIZE];
    long val_size = 0;
    long count = 0; 
    cdata* buff = (cdata*)data;
            
    policy_no = policy_type_[obj->type_];
    TStatObjWrapper wrapper;
    OBJ_WRAPPER(obj, wrapper);    
    count = policy_[policy_no]->__result__(&wrapper, values, &val_size);
    output_statobj(obj, count, values, val_size, buff->buffer, buff->len);
}
int CTStat::find_statobj(const char* id, int type)
{
    int bucketid  = hashid(id);
    int next = statpool_->bucket_[bucketid];
    TStatObj* obj = NULL;
    while(1)
    {
        if(INVALID_HANDLE == next)
            break;
        else
        {    
            OBJ_ENTRY(obj, next);
            if(!strcmp(obj->id_, id) && ((type == STAT_TYPE_ALL) || (type == obj->type_)))
                break; 
            else
                next = obj->next_;
        }
    }
    return next;
}
int CTStat::alloc_statobj(int val_size)
{
    assert(val_size >= 1 && statpool_->statobjs_used_ < DEFAULT_STATOBJ_NUM && statpool_->statvals_used_ < DEFAULT_STATVAL_NUM - val_size);

    TStatObj* obj = NULL;
    int choice = statpool_->freelist_;

    OBJ_ENTRY(obj, choice);
    obj->val_size_ = val_size;
    obj->val_offset_ = statpool_->statvals_used_;
    //	obj->value_ = statpool_->statvals_ + obj->val_offset_;
    statpool_->freelist_ = obj->next_;
    statpool_->statobjs_used_++;
    statpool_->statvals_used_ += val_size;    
    return choice;
}
void CTStat::insert_statobj(int choice)
{
    TStatObj* obj = NULL;
    OBJ_ENTRY(obj, choice);

    int bucketid = hashid(obj->id_);
    int next = statpool_->bucket_[bucketid];
    obj->next_ = next;
    statpool_->bucket_[bucketid] = choice;
}
void CTStat::newpool()
{
    memset(statpool_, 0x0, sizeof(TStatPool));
    statpool_->magic_ = TSTAT_MAGICNUM;
    statpool_->ver_ = TSTAT_VERSION;
    statpool_->freelist_ = 0;
    statpool_->statobjs_used_ = 0;
    statpool_->statvals_used_ = 0;

    int i;
    for(i = 0; i < BUCKET_NUM; ++i)
        statpool_->bucket_[i] = INVALID_HANDLE;

    for(i = 0; i < DEFAULT_STATOBJ_NUM - 1; ++i)
        statpool_->statobjs_[i].next_ = i + 1;

    statpool_->statobjs_[DEFAULT_STATOBJ_NUM - 1].next_ = INVALID_HANDLE;
}
void CTStat::output_statpool(char* buffer, int* len)
{
    time_t now = get_timebytask(NULL);
    struct tm tmm;
    localtime_r(&now, &tmm); 

    *len += sprintf(buffer + *len, "\nTStat[%-5d],%04d-%02d-%02d %02d:%02d:%02d\nUsed StatObj Num: %d\tUsed StatVal Num: %d\n", (int)syscall(__NR_gettid), 
            tmm.tm_year + 1900, tmm.tm_mon + 1, tmm.tm_mday, tmm.tm_hour, tmm.tm_min, tmm.tm_sec,
            statpool_->statobjs_used_, statpool_->statvals_used_);

    *len += sprintf(buffer + *len, "%-20s|%-20s|%-8s|%-8s|%s\n", "StatID", "Desc", "Type", "Count", "Values");
}

void CTStat::moni_statpool(char * buffer,int *len)
{
    time_t now = get_timebytask(NULL);
    struct tm tmm;
    localtime_r(&now, &tmm); 
    *len += sprintf(buffer + *len, "TStat[%-5d],%04d-%02d-%02d %02d:%02d:%02d\tUsed StatObj Num: %d\tUsed StatVal Num: %d\n", (int)syscall(__NR_gettid), 
        tmm.tm_year + 1900, tmm.tm_mon + 1, tmm.tm_mday, tmm.tm_hour, tmm.tm_min, tmm.tm_sec,
        statpool_->statobjs_used_, statpool_->statvals_used_);

    *len += sprintf(buffer + *len, "%-20s|%-20s|%-8s|%-8s|%s\n", "StatID", "Desc", "Type", "Count", "Values");
}
void CTStat::output_statobj(const TStatObj* obj, long count, const long* values, int val_size, char* buffer, int* len)
{
    *len += sprintf(buffer + *len, "%-20s|%-20s|%-8s|%-8ld|", obj->id_, obj->desc_, policy_[policy_type_[obj->type_]]->__tag__(), count);
    for(int i = 0; i < val_size; ++i)
        *len += sprintf(buffer + *len, "%-11ld", values[i]);
    
    *len += sprintf(buffer + *len, "\n");
}

