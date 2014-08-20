
#ifndef _ERROR_H_
#define _ERROR_H_


namespace comm
{
namespace commu
{


#define CLEAR_ERROR  
#define SET_ERROR  
#define SET_ERROR_SYS  

//设置错误值, 并函数返回
#define ERROR_RETURN_SYS_ERROR(errNo, errMsg) \
            return errNo;

#define ERRNO_RETURN(errNo); \
    		return errNo;

#define ERROR_RETURN(errNo, errMsg); \
    		return errNo;

#define ERROR_RETURN_1(errNo, errMsg, param1) \
            return errNo;

#define ERROR_RETURN_2(errNo, errMsg, param1, param2) \
            return errNo;

#define ERROR_RETURN_3(errNo, errMsg, param1, param2, param3) \
            return errNo;

#define ERROR_RETURN_NULL(errNo, errMsg); \
    		return NULL;

#define ERROR_RETURN_NULL_1(errNo, errMsg, param1) \
            return NULL;

#define ERROR_RETURN_NULL_2(errNo, errMsg, param1, param2) \
            return NULL;

#define ERROR_RETURN_NULL_3(errNo, errMsg, param1, param2, param3) \
            return NULL;


}
}



#endif

