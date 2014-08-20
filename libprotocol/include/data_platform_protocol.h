
/********************************
*文件名：data_platform_protocol.h
*功能：数据平台协议解析类
*作者：张荐林
*版本：1.0
*最后更新时间：2009.06.09
**********************************/

#ifndef _DATA_PLATFORM_PROTOCOL_H_
#define _DATA_PLATFORM_PROTOCOL_H_

#include <stdint.h>
#include <stdlib.h>
#include <string>
#include <vector>

using std::string;
using std::vector;

#include "protocol_pack.h"

namespace protocol
{
#define DP_ADD_COOKIE(req,ck) do{if(ck){int r = ck->Pack(req.Cookie());if(r == -1)return r;}}while(0)
/*TLV定义
T(Type)：数据类型，1字节
		0：integer8，1字节
		1：integer16，2字节
2：integer32，4字节
3：integer64，8字节
4：real，8字节
5：string，变长
6：binary，变长
L(Length)：value数据长度，4字节
V(value)：字节数据
*/
const unsigned char TLV_BYTE = 0;
const unsigned char TLV_WORD = 1;
const unsigned char TLV_DWORD = 2;
const unsigned char TLV_QWORD = 3;
const unsigned char TLV_REAL = 4;
const unsigned char TLV_STRING = 5;
const unsigned char TLV_BINARY = 6;




//标记位定义
const unsigned char SYNC_FLAG_BIT = 1;
const unsigned short SYNC_FLAG_MASK = 0x0001;

const unsigned char PRIMARY_KEY_FLAG_BIT = 2;
const unsigned char PRIMARY_KEY_FLAG_MASK = 0x0002;


//数据平台基本命令字定义
const unsigned short INSERT_MAIN_CMD = 0x0500;
const unsigned short UPDATE_MAIN_CMD = 0x0501;
const unsigned short DELETE_MAIN_CMD = 0x0502;
const unsigned short QUERY_MAIN_CMD=0x0503;
const unsigned short COUNT_MAIN_CMD = 0x0504;
const unsigned short DELTA_MAIN_CMD = 0x0505;


//扩展应用命令字定义

//COMBO服务器命令字
const unsigned short COMBO_REQUEST_MAIN_CMD = 0x0900;

//查找服务器命令字
const unsigned short COND_FIND_MAIN_CMD = 0x0981;

//数据平台status状态码定义
const unsigned short DP_RET_OK = 0;                             //成功
const unsigned short DP_ERR_GENERAL = 0x0001;       //一般错误
const unsigned short DP_ERR_NOT_FOUND = 0x0002; //记录不存在
const unsigned short DP_ERR_DUPLICATE_ID = 0x0003;  //重复键
const unsigned short DP_ERR_DB = 0x0004;  //DB错误
const unsigned short DP_ERR_PROTOCOL = 0x0005;  //协议格式错误
const unsigned short DP_ERR_TOO_BIG = 0x0006;  //结果集过大
const unsigned short DP_ERR_DATA_ERR = 0x0007;// for RET_DB_ERR_PARAM and RET_DB_ERR_FIELD
const unsigned short DP_ERR_TIME_OUT = 0x0008;  
const unsigned short DP_ERR_NETWORK_ERR = 0x0009;
const unsigned short DP_ERR_CMD_UNKNOWN = 0x0081;  //未知命令



//条件表达式的操作符定义
const unsigned char OP_EQUAL = 0;           // =
const unsigned char OP_NOT_EQUAL = 1;   // <>
const unsigned char OP_ABOVE = 2;           // >
const unsigned char OP_BELLOW = 3;          // <
const unsigned char OP_ABOVE_EQUAL = 4; // >=
const unsigned char OP_BELLOW_EQUAL = 5;    //<=
const unsigned char OP_BIT_ALL = 6; //条件中所有为1的bit位，都满足cond_val == (cond_val & value)
const unsigned char OP_BIT_ONE = 7;//条件中为1的bit位，至少有一个0 != (cond_val & value)
const unsigned char OP_LIKE = 8;    //sql like

class TLV
{
public:
    unsigned char type;
    unsigned int length;
    uint64_t int_value; //整数值
    double real_value; //实数值
    string string_value; //字符串值或二进制值

public:
    TLV(){}
    TLV(unsigned char v){Assign(v);}
    TLV(char v){Assign((unsigned char)v);}

    TLV(unsigned short v){Assign(v);}
    TLV(short v){Assign((unsigned short)v);}

    TLV(unsigned int v){Assign(v);}
    TLV(int v){Assign((unsigned int)v);}

    TLV(uint64_t v){Assign(v);}
    TLV(int64_t v){Assign((uint64_t)v);}

    TLV(double v){Assign(v);}
    TLV(const string& v){Assign(v);}
    TLV(const char *v){Assign(v);}
    TLV(const void *v, unsigned int len){Assign(v, len);}

    TLV(const TLV &r){*this=r;}
    TLV &operator=(const TLV &r)
    {
        this->type = r.type;
        this->length = r.length;
        this->int_value = r.int_value;
        this->real_value = r.real_value;
        if( (r.type == TLV_STRING) || (r.type == TLV_BINARY))
        {
                this->string_value = r.string_value;
        }
        return *this;
    }

    //整数赋值操作
    void Assign(unsigned char v){type = TLV_BYTE;length=1;int_value=v;}
    void Assign(unsigned short v){type = TLV_WORD; length=2;int_value=v;}
    void Assign(unsigned int v){type = TLV_DWORD;length = 4; int_value = v;}
    void Assign(uint64_t v){type = TLV_QWORD; length = 8; int_value = v;}
        
    //浮点数赋值操作
    void Assign(double v){type = TLV_REAL; length = 8; real_value = v;}

    //字符串赋值操作
    void Assign(const string &v){type = TLV_STRING; length = v.size(); string_value = v;}
    void Assign(const char *v)
    {
        type = TLV_STRING; 
        if(v)
        {
            length = strlen(v); 
            string_value = v;
        }
    }

    //二进制赋值操作    
    void Assign(const void *v, unsigned int len)
    {   
        type = TLV_BINARY; 
        length = len;
        if(len > 0)
        {
            string_value.assign((const char *)v, len);
        }
    }         

};

class ConditionExpression
{
public:
    unsigned int field_id;
    unsigned char op_code;      //操作码
    TLV data;

    ConditionExpression(){}

    ConditionExpression(unsigned int fid, unsigned char op, const TLV &d)
    :field_id(fid),op_code(op), data(d)
    {
    }

    ConditionExpression(const ConditionExpression &r){*this = r;}
    ConditionExpression &operator=(const ConditionExpression &r)
    {
        field_id = r.field_id;
        op_code = r.op_code;
        data = r.data;
        return *this;
    }

};

#if 0
class DataPlatformProtocol
{
public:
    /*
    *功能：在数据包生成后设置数据库路由KEY
    *OUT pack:数据包
    *IN key:要放到包头中的key
    *IN unique:是否是唯一key.  
                true:是， false:不是，这种情况要全表扫描
    *返回值：0成功，非0失败
    */
    static int SetRouteKey(string &pack, unsigned int key, bool unique=true);

    /*
    *功能：生成只有一个条件的查询请求.

    *OUT pack:输出参数，返回生成的请求包
    *IN is_primary_key:查询条件是否是主键
            true代表是主键，先查CACHE，不命中就再查DB
            false代表非主键,不查CACHE，直接查DB.
     !!!：调用时必须准确填写这个标记，
                否则可能得到不准确的数据(dirty data).
            
    *IN cond: 条件表达式
    *IN ret_field_num:需要返回的字段数
    *IN ret_fieldid_set:需要返回的字段ID集
    *IN seq:包的序列号
    *IN cookie_len:cookie的长度
    *IN cookie: cookie的内容
    *返回值：0成功，其它出错，具体错误请参照协议错误码定义
    */
    static int MakeQueryReq(string &pack,bool is_primary_key, const ConditionExpression &cond, 
                                                    unsigned short ret_field_num,unsigned int *ret_filedid_set, unsigned int seq);
    static int MakeQueryReq(string &pack,bool is_primary_key, const ConditionExpression &cond, 
                                                    unsigned short ret_field_num,unsigned int *ret_filedid_set, unsigned int seq,
                                                    unsigned short cookie_len, const void *cookie);

    /*
    *功能：生成有多个条件的查询请求.

    *OUT pack:输出参数，返回生成的请求包

    *IN is_primary_key:查询条件是否是主键
            true代表是主键，先查CACHE，不命中就再查DB
            false代表非主键,不查CACHE，直接查DB.
     !!!：调用时必须准确填写这个标记，
                否则可能得到不准确的数据(dirty data).
            
    *IN cond_num: 条件表达式的个数
    *IN condtions:多个条件表达式的数组
    *IN logic_expression:多个条件之间的逻辑表达式
            例：$1 AND $2
    *IN ret_field_num:需要返回的字段数
    *IN ret_fieldid_set:需要返回的字段ID集
    *IN seq:包的序列号
    *IN cookie_len:cookie的长度
    *IN cookie: cookie的内容

    *返回值：0成功，其它出错，具体错误请参照协议错误码定义
    */
    static int MakeQueryReq(string &pack, bool is_primary_key, 
                    unsigned short cond_num, const ConditionExpression *condtions,  
                    const char *logic_expression, unsigned short ret_field_num,
                    unsigned int *ret_filedid_set, unsigned int seq);

    static int MakeQueryReq(string &pack, bool is_primary_key, 
                    unsigned short cond_num, const ConditionExpression *condtions,  
                    const char *logic_expression, unsigned short ret_field_num,
                    unsigned int *ret_filedid_set, unsigned int seq, unsigned short cookie_len, const void *cookie);

    /*
    *功能：从响应包里面取出主命令字和子命令字
    *main_cmd:输出参数，取出的主命令字
    *sub_cmd:输出参数，取出的子命令字
    *pack:输入参数，响应包
    *pack_len:输入参数，响应包长度

    *返回值：0代表成功
    */
    static int GetRspCmd(unsigned short &main_cmd, unsigned short &sub_cmd, 
                const char *pack, unsigned int pack_len);


    /*
    *功能：解析Query响应包
    *OUT status:输出参数，错误码
    *OUT data:输出参数，记录集
    *IN pack:输入参数，响应包
    *IN pack_len:输入参数，响应包长度
    *DEFAULT OUT MsgHeader *h:输出参数，如果为NULL表示不需要。
            不为NULL表示要得到消息头

    *返回值：0代表成功
    */
    static int ParseQueryRsp(unsigned short &status, std::vector<unsigned int> &fileds,
                std::vector<TLV> &data, const char *pack, unsigned int pack_len,
                MsgHeader *h = NULL);   
    /*
    *   OUT cookie: 从后端原样返回的cookie数据
    */
    static int ParseQueryRsp(string &cookie, unsigned short &status, std::vector<unsigned int> &fileds,
                std::vector<TLV> &data, const char *pack, unsigned int pack_len,
                MsgHeader *h = NULL);   


    /*
    *功能：生成count请求

    *OUT pack:输出参数，返回生成的请求包

            
    *IN cond_num: 条件表达式的个数
    *IN condtions:多个条件表达式的数组
    *IN logic_expression:多个条件之间的逻辑表达式
            例：$1 AND $2
    *IN seq:包的序列号
    *IN cookie_len:cookie的长度
    *IN cookie: cookie的内容

    *返回值：0成功，其它出错，具体错误请参照协议错误码定义
    */
    static int MakeCountReq(string &pack, 
                    unsigned short cond_num, const ConditionExpression *condtions,  
                    const char *logic_expression, unsigned int seq);

    static int MakeCountReq(string &pack, 
                    unsigned short cond_num, const ConditionExpression *condtions,  
                    const char *logic_expression, unsigned int seq, 
                    unsigned short cookie_len, const void *cookie);

    /*
    *功能：解析count响应包
    *status:输出参数，错误码
    *record_num:输出参数，记录数
    *pack:输入参数，响应包
    *pack_len:输入参数，响应包长度
    *MsgHeader *h:输出参数，如果为NULL表示不需要。
            不为NULL表示要得到消息头

    *返回值：0代表成功
    */
    static int ParseCountRsp(unsigned short &status, unsigned int &record_num, 
        const char *pack, unsigned int pack_len,MsgHeader *h = NULL);   

    /*
    *   OUT cookie: 从后端原样返回的cookie数据
    */
    static int ParseCountRsp(string &cookie, unsigned short &status, unsigned int &record_num, 
        const char *pack, unsigned int pack_len,MsgHeader *h = NULL);   


    /*
    *功能：生成delete请求

    *OUT pack:输出参数，返回生成的请求包


     !!!：key必须是主键.
    *IN is_asyn:是否异步入库
                true:异步入库，响应快速，但有可能失败，
                    虽然机率很小，但还是有可能发生，
                    对核心数据，建议不要使用异步方式。
                false:同步入库，响应慢，但能确保成功.                    
    *IN cond_num: 条件表达式的个数
    *IN condtions:多个条件表达式的数组
    *IN logic_expression:多个条件之间的逻辑表达式
            例：$1 AND $2
    *IN seq:包的序列号
    *IN cookie_len:cookie的长度
    *IN cookie: cookie的内容

    *返回值：0成功，其它出错，具体错误请参照协议错误码定义
    */
    static int MakeDeleteReq(string &pack, bool is_async,
                    unsigned short cond_num, const ConditionExpression *condtions,  
                    unsigned int seq);

    static int MakeDeleteReq(string &pack, bool is_async,
                    unsigned short cond_num, const ConditionExpression *condtions,  
                    unsigned int seq, unsigned short cookie_len, const void *cookie);


    /*
    *功能：解析delete响应包
    *status:输出参数，错误码
    *record_num:输出参数，删除的记录数
    *pack:输入参数，响应包
    *pack_len:输入参数，响应包长度
    *MsgHeader *h:输出参数，如果为NULL表示不需要。
            不为NULL表示要得到消息头
    *返回值：0代表成功
    */
    static int ParseDeleteRsp(unsigned short &status, unsigned int &record_num, 
        const char *pack, unsigned int pack_len, MsgHeader *h = NULL);   

    /*
    *   OUT cookie: 从后端原样返回的cookie数据
    */
    static int ParseDeleteRsp(string &cookie, unsigned short &status, unsigned int &record_num, 
        const char *pack, unsigned int pack_len, MsgHeader *h = NULL);   

    /*
    *功能：生成update请求

    *OUT pack:输出参数，返回生成的请求包

     !!!：key必须是主键.
    *IN is_asyn:是否异步入库
                true:异步入库，响应快速，但有可能失败，
                    虽然机率很小，但还是有可能发生，
                    对核心数据，建议不要使用异步方式。
                false:同步入库，响应慢，但能确保成功.                    
    *IN cond_num: 条件表达式的个数
    *IN condtions:多个条件表达式的数组
    *IN logic_expression:多个条件之间的逻辑表达式
            例：$1 AND $2
    *IN seq:包的序列号
    *IN cookie_len:cookie的长度
    *IN cookie: cookie的内容

    *返回值：0成功，其它出错，具体错误请参照协议错误码定义
    */
    static int MakeUpdateReq(string &pack, bool is_async,
                    unsigned short cond_num, const ConditionExpression *condtions,  
                    const unsigned int update_field_num, const unsigned int *update_field_set,
                    const TLV *update_value_set,  unsigned int seq);

    static int MakeUpdateReq(string &pack, bool is_async,
                    unsigned short cond_num, const ConditionExpression *condtions,  
                    const unsigned int update_field_num, const unsigned int *update_field_set,
                    const TLV *update_value_set,  unsigned int seq,
                    unsigned short cookie_len, const void *cookie);

    /*
    *功能：解析update响应包
    *status:输出参数，错误码
    *record_num:输出参数，update的记录数
    *pack:输入参数，响应包
    *pack_len:输入参数，响应包长度
    *MsgHeader *h:输出参数，如果为NULL表示不需要。
            不为NULL表示要得到消息头
    *返回值：0代表成功
    */
    static int ParseUpdateRsp(unsigned short &status, unsigned int &record_num, 
        const char *pack, unsigned int pack_len, MsgHeader *h = NULL);   

    /*
    *   OUT cookie: 从后端原样返回的cookie数据
    */
    static int ParseUpdateRsp(string &cookie, unsigned short &status, unsigned int &record_num, 
        const char *pack, unsigned int pack_len, MsgHeader *h = NULL);   

    
    /*
    *功能：生成insert请求

    *OUT pack:输出参数，返回生成的请求包

     !!!：key必须是主键.
    *IN is_asyn:是否异步入库
                true:异步入库，响应快速，但有可能失败，
                    虽然机率很小，但还是有可能发生，
                    对核心数据，建议不要使用异步方式。
                false:同步入库，响应慢，但能确保成功.                    
    *IN ins_field_num:输入参数，要插入的字段个数
    *IN ins_field_set:输入参数，要插入的字段ID列表
    *IN ins_value_set:输入参数，要插入的字段值列表
    *IN seq:包的序列号
    *IN cookie_len:cookie的长度
    *IN cookie: cookie的内容

    *返回值：0成功，其它出错，具体错误请参照协议错误码定义
    */
    static int MakeInsertReq(string &pack, bool is_async,
                    const unsigned int ins_field_num, const unsigned int *ins_field_set,
                    const TLV *ins_value_set,  unsigned int seq);

    static int MakeInsertReq(string &pack, bool is_async,
                    const unsigned int ins_field_num, const unsigned int *ins_field_set,
                    const TLV *ins_value_set,  unsigned int seq, 
                    unsigned short cookie_len, const void *cookie);

    /*
    *功能：解析insert响应包
    *status:输出参数，错误码
    *record_num:输出参数，insert的记录数
    *pack:输入参数，响应包
    *pack_len:输入参数，响应包长度
    *MsgHeader *h:输出参数，如果为NULL表示不需要。
            不为NULL表示要得到消息头
    *返回值：0代表成功
    */
    static int ParseInsertRsp(unsigned short &status, unsigned int &record_num, 
        const char *pack, unsigned int pack_len, MsgHeader *h = NULL);   

    /*
    *   OUT cookie: 从后端原样返回的cookie数据
    */
    static int ParseInsertRsp(string &cookie, unsigned short &status, unsigned int &record_num, 
        const char *pack, unsigned int pack_len, MsgHeader *h = NULL);   

};

#endif


//======New API.



class BaseCookie
{
public:
        virtual ~BaseCookie(){}
       /*
        *功能：生成cookie
        *OUT cookie:生成的cookie数据
        *返回值：cookie长度
        */
       virtual int Pack(string &cookie) = 0;

       /*
        *功能：生成cookie
        *OUT buf:生成的cookie数据
        *IN len:buf的大小
        *返回值：生成的cookie大小
        */
       virtual int Pack(char *buf, unsigned int len) = 0;

       /*
        *功能：解析cookie
        *IN buf:cookie数据
        *IN len:buf的大小
        *返回值：0成功，非0失败
        */
       virtual int UnPack(const char *buf, unsigned int len) = 0;    
    
};


class CommCookie : public BaseCookie
{
public:
        /*
        *功能：默认构造函数。数据没有初始化
        */
        CommCookie():m_pReqPack(0),m_ReqPackLen(0){}

        CommCookie( unsigned short source, unsigned int flow,
                                        const char *pReqPack, unsigned int reqPackLen 
                                        ) : m_Source(source), m_Flow(flow),
                                        m_pReqPack(pReqPack), m_ReqPackLen(reqPackLen)
                                        {}

        CommCookie( unsigned short source, unsigned int flow,
                                        const char *pReqPack, unsigned int reqPackLen, 
                                        const string &extInfo) : m_Source(source), m_Flow(flow),
                                        m_pReqPack(pReqPack), m_ReqPackLen(reqPackLen),
                                        m_strExtInfo(extInfo){}

        virtual ~CommCookie(){}
       /*
        *功能：生成cookie
        *OUT cookie:生成的cookie数据
        *返回值：cookie长度
        */
       virtual int Pack(string &cookie) ;

       /*
        *功能：生成cookie
        *OUT buf:生成的cookie数据
        *IN len:buf的大小
        *返回值：生成的cookie大小
        */
       virtual int Pack(char *buf, unsigned int len) ;

       /*
        *功能：解析cookie
        *IN buf:cookie数据
        *IN len:buf的大小
        *返回值：0成功，非0失败
        */
       virtual int UnPack(const char *buf, unsigned int len);    
       virtual int UnPack(const string & ck){return UnPack(ck.data(), ck.size());}    
       
       unsigned short m_Source;     //cookie的来源
       unsigned int m_Flow;         //原请求的flow
       const char *m_pReqPack;             //原请求的包。可以为空
       unsigned int m_ReqPackLen;

       string m_strExtInfo;         //业务自定的扩展数据。可以为空

};


/*
*字段集类
*/
class DpFieldSet : public vector<unsigned int>
{
public:
     typedef vector<unsigned int> parent;
public:
    /*
    *功能：新增一个字段
    *IN id: 字段ID
    *返回值：无
    */
     void Add(unsigned int id){ parent::push_back(id);}
};

/*
*字段  操作 数据   集类
*/
class DpFieldValueSet : public vector<ConditionExpression>
{
public:
        typedef vector<ConditionExpression> parent;
        
public:
        /*
        *功能：新增一个字段操作数据
        *IN fid:字段ID
        *IN optype:操作类型
        *IN v:数据
        *返回值：无
        */
       void Add(unsigned int fid, unsigned char optype, const TLV &v)
       {
            parent::push_back(ConditionExpression(fid, optype,v));
       }

        /*
        *功能：新增一个字段操作数据,操作类型为=
        *IN fid:字段ID
        *IN v:数据
        *返回值：无
        */
       void Add(unsigned int fid, const TLV &v)
       {
           Add(fid, OP_EQUAL, v);
       }
};          
   

//数据平台请求包的基类
class BaseDpReq
{
public:
        /*
        *功能：默认构造函数。初始化了seq 和 sub_cmd
        */
       BaseDpReq(){m_Header.seq = m_DefaultSeq++; m_Header.sub_cmd = 0;}
       virtual ~BaseDpReq(){}

        /*
        *功能：生成数据包
        *OUT req:生成的数据包
        *IN pCookie:cookie数据. 
        *返回值：生成的数据包的大小。<=0出错
        */
       virtual int MakeReq(string &req)=0;

        /*
        *功能：生成数据包
        *OUT buf:生成的数据包
        *IN buf_len:buf的大小
        *IN pCookie:cookie数据. 
        *返回值：生成的数据包的大小。<=0出错
        */
       virtual int MakeReq(char *buf, unsigned int buf_len) = 0;
        
       /*
        *功能：生成表的路由KEY。 
        *IN rk:路由KEY。 
        *IN isUnique:是否是唯一表。
        *返回值：无
       */
       void SetRouteKey(unsigned int rk, bool isUnique)
        {
            m_Header.to_uin  =rk;
            m_Header.source_type = isUnique ? 0 : 1;
        }

       /*
        *功能：生成表的路由KEY。 
        *IN rk:路由KEY。 
        *IN isUnique:是否是唯一表。
        *返回值：无
       */
       void SetRouteKey(const char *rk, bool isUnique);
       void SetRouteKey(const string &rk, bool isUnique)
        {
            return SetRouteKey(rk.c_str(),isUnique);
        }
        
        static unsigned int MakeRouteKey(const char *rk);
        static unsigned int MakeRouteKey(const string &rk){return MakeRouteKey(rk.c_str());}    

       /*
        *功能：取路由KEY。 
        *返回值：路由key
       */
       unsigned int GetRouteKey(){return m_Header.to_uin;}

       //可以省掉，构造函数中会自动生成。
       void SetSeq(unsigned int seq){m_Header.seq = seq;}
    
       /*
        *功能：读或者写cookie
        *返回值：cookie引用
       */
      string &Cookie(){return m_Cookie;}

protected:
       void PackCookie(Packer &pk)
        {
            unsigned short cookie_len = m_Cookie.size();
            pk.PackWord(cookie_len);
            pk.PackBinary(m_Cookie.data(), cookie_len);
        }
       void PackCookie(FixedPacker &pk)
        {
            unsigned short cookie_len = m_Cookie.size();
            pk.PackWord(cookie_len);
            pk.PackBinary(m_Cookie.data(), cookie_len);
        }

protected:
       static unsigned int m_DefaultSeq;
        string m_Cookie;
        MsgHeader m_Header;
};

//数据平台响应包的基类    
class BaseDpRsp
{
public:
        /*
        *功能：默认构造函数。初始化了m_RetCode 和 m_RecordNum
        */
       BaseDpRsp():m_RetCode(0xffff),m_RecordNum(0){}
       virtual ~BaseDpRsp(){}

        /*
        *功能：解析数据平台的响应
        *IN buf:响应包
        *IN buf_len:响应包的长度
        *返回值：0成功，非0失败
        */
       virtual int ParseRsp(const char *buf, unsigned int buf_len) ;

    
        /*
        *功能：取返回的错误码
        *返回值：返回的错误码
        */
       unsigned short GetRetCode(){return m_RetCode;}

        /*
        *功能：取返回的记录数
        *返回值：返回的记录数
        */
       unsigned int GetRecordNum(){return m_RecordNum;}


        /*
        *功能：取响应包中的命令字
        *OUT main_cmd:主命令字
        *OUT sub_cmd:子命令字
        *IN pack:数据包
        *IN pack_len:数据包的大小
        *返回值：0成功，非0 失败
        */
        static int GetRspCmd(unsigned short &main_cmd, unsigned short &sub_cmd, 
                const char *pack, unsigned int pack_len);

       /*
        *功能：读或者写cookie
        *返回值：cookie引用
       */
      string &Cookie(){return m_Cookie;}

protected:
       void UnPackCookie(UnPacker &upk)
        {
            unsigned short cookie_len = upk.UnPackWord();
            upk.UnPackBinary(m_Cookie, cookie_len);
        }

        string m_Cookie;
       unsigned short m_RetCode;
       unsigned int m_RecordNum;
};
 
//数据平台query请求类
class DpQueryReq : public BaseDpReq
{
public:
    /*
    *功能：默认构造函数。
        初始化了m_LogicExp 和 m_bPrimaryKey
    */
    DpQueryReq():m_LogicExp("$1"),m_bPrimaryKey(true){m_Header.main_cmd = QUERY_MAIN_CMD;}
    virtual ~DpQueryReq(){}

    /*
    *功能：生成数据包
    *OUT req:生成的数据包
    *IN pCookie:cookie数据. 
    *返回值：生成的数据包的大小。<=0出错
    */
   virtual int MakeReq(string &req);

    /*
    *功能：生成数据包
    *OUT buf:生成的数据包
    *IN buf_len:buf的大小
    *IN pCookie:cookie数据. 
    *返回值：生成的数据包的大小。<=0出错
    */
    virtual int MakeReq(char *buf, unsigned int buf_len);

    /*
    *功能：添加条件
    *IN fid:字段ID
    *IN op_type:条件类型
    *IN TLV:数据值. 
    *返回值：无
    */
    void AddCond(unsigned int fid, unsigned char optype, const TLV &v){m_CondSet.Add(fid,optype,v);}

    /*
    *功能：添加要返回的字段ID
    *IN fid:字段ID
    *返回值：无
    */
    void AddRetField(unsigned int id){m_RetFieldSet.Add(id);}

    /*
    *功能：设置逻辑表达式.
                        example: $1 AND $2 OR $3 limit 100
    *IN exp:逻辑表达式
    *返回值：无
    */
    void SetLogicExp(const char *exp){m_LogicExp = exp;}

    /*
    *功能：设置主键
    *IN isPrimaryKey:是否为主键
    *返回值：无
    */
    void SetPrimaryKey(bool isPrimaryKey){m_bPrimaryKey = isPrimaryKey;}

private: 
 
    DpFieldSet m_RetFieldSet;
    DpFieldValueSet m_CondSet;
    string m_LogicExp;
    bool m_bPrimaryKey;
};
    

//数据平台update请求类    
class DpUpdateReq : public BaseDpReq
{
public:

    DpUpdateReq():m_bAsyn(false){m_Header.main_cmd = UPDATE_MAIN_CMD;}
    virtual ~DpUpdateReq(){}

    /*
    *功能：生成数据包
    *OUT req:生成的数据包
    *IN pCookie:cookie数据. 
    *返回值：生成的数据包的大小。<=0出错
    */
   virtual int MakeReq(string &req);

    /*
    *功能：生成数据包
    *OUT buf:生成的数据包
    *IN buf_len:buf的大小
    *IN pCookie:cookie数据. 
    *返回值：生成的数据包的大小。<=0出错
    */
    virtual int MakeReq(char *buf, unsigned int buf_len);

    /*
    *功能：添加要更新的字段
    *IN fid:字段ID
    *IN TLV:数据值. 
    *返回值：无
    */
    void AddUpdData(unsigned int fid, const TLV &v){m_UpdFieldSet.Add(fid, v);}

    /*
    *功能：添加条件
    *IN fid:字段ID
    *IN op_type:条件类型
    *IN TLV:数据值. 
    *返回值：无
    */
    void AddCond(unsigned int fid,unsigned char optype, const TLV &v){m_CondFieldSet.Add(fid,optype, v);}

    /*
    *功能：设置逻辑表达式.
                        目前JCACHE不支持逻辑表达式.
                        只允许用主键
    *IN exp:逻辑表达式
    *返回值：无
    */
    void SetLogicExp(const char *exp){m_LogicExp = exp;}

    /*
    *功能：设置JCACHE的操作方式是否为异步
    *IN b:是否异步
    *返回值：无        
    */
    void SetAsync(bool b){m_bAsyn = b;}

protected:
    DpFieldValueSet m_UpdFieldSet;
    DpFieldValueSet m_CondFieldSet;             
    string m_LogicExp;
    bool m_bAsyn;
};  


//数据平台insert请求类
class DpInsertReq : public BaseDpReq
{
public:

        DpInsertReq():m_bAsyn(false){m_Header.main_cmd = INSERT_MAIN_CMD;}
        virtual ~DpInsertReq(){}
        /*
        *功能：生成数据包
        *OUT req:生成的数据包
        *IN pCookie:cookie数据. 
        *返回值：生成的数据包的大小。<=0出错
        */
       virtual int MakeReq(string &req);

        /*
        *功能：生成数据包
        *OUT buf:生成的数据包
        *IN buf_len:buf的大小
        *IN pCookie:cookie数据. 
        *返回值：生成的数据包的大小。<=0出错
        */
       virtual int MakeReq(char *buf, unsigned int buf_len);

        /*
        *功能：添加要插入的字段
        *IN fid:字段ID
        *IN TLV:数据值. 
        *返回值：无
        */
        void AddInsData(unsigned int fid, const TLV &v){m_InsFieldSet.Add(fid, v);}

        void SetAsync(bool b){m_bAsyn = b;}

protected:
    DpFieldValueSet m_InsFieldSet;

    bool m_bAsyn;

};

//数据平台delete请求类
class DpDeleteReq : public BaseDpReq
{
public:
        DpDeleteReq():m_bAsyn(false){m_Header.main_cmd = DELETE_MAIN_CMD;}
        virtual ~DpDeleteReq(){}
        /*
        *功能：生成数据包
        *OUT req:生成的数据包
        *IN pCookie:cookie数据. 
        *返回值：生成的数据包的大小。<=0出错
        */
       virtual int MakeReq(string &req);

        /*
        *功能：生成数据包
        *OUT buf:生成的数据包
        *IN buf_len:buf的大小
        *IN pCookie:cookie数据. 
        *返回值：生成的数据包的大小。<=0出错
        */
       virtual int MakeReq(char *buf, unsigned int buf_len);
    
        /*
        *功能：添加条件
        *IN fid:字段ID
        *IN op_type:条件类型
        *IN TLV:数据值. 
        *返回值：无
        */
        void AddCond(unsigned int fid,unsigned char optype, const TLV &v){m_CondFieldSet.Add(fid,optype, v);}

        //目前只能用主键，数据平台会忽略逻辑表达式.
        void SetLogicExp(const char *exp){m_LogicExp = exp;}

protected:
    DpFieldValueSet m_CondFieldSet;
    string m_LogicExp;
    bool m_bAsyn;
};

//数据平台计数统计请求类
class DpCountReq : public BaseDpReq
{
public:
        DpCountReq():m_bAsyn(false){m_Header.main_cmd = COUNT_MAIN_CMD;}
        virtual ~DpCountReq(){}
        /*
        *功能：生成数据包
        *OUT req:生成的数据包
        *IN pCookie:cookie数据. 
        *返回值：生成的数据包的大小。<=0出错
        */
       virtual int MakeReq(string &req);

        /*
        *功能：生成数据包
        *OUT buf:生成的数据包
        *IN buf_len:buf的大小
        *IN pCookie:cookie数据. 
        *返回值：生成的数据包的大小。<=0出错
        */
       virtual int MakeReq(char *buf, unsigned int buf_len);
    
        /*
        *功能：添加条件
        *IN fid:字段ID
        *IN op_type:条件类型
        *IN TLV:数据值. 
        *返回值：无
        */
        void AddCond(unsigned int fid,unsigned char optype, const TLV &v){m_CondFieldSet.Add(fid,optype, v);}

        /*
        *功能：设置逻辑表达式.
        *IN exp:逻辑表达式
        *返回值：无
        */
        void SetLogicExp(const char *exp){m_LogicExp = exp;}

protected:
    DpFieldValueSet m_CondFieldSet;
    string m_LogicExp;
    bool m_bAsyn;

};

//操作一个字段加或减一个整数值。
class DpDeltaReq : public DpUpdateReq
{
public:
    DpDeltaReq(){m_Header.main_cmd = DELTA_MAIN_CMD;}
    virtual ~DpDeltaReq(){}

    /*
    *功能：对某个整数字段增加或减少一个数值。
    *注意：V的类型必须与字段的类型一致
    */
    void Delta(unsigned int fid,  char v){m_UpdFieldSet.Add(fid, TLV(v));}
    void Delta(unsigned int fid,  short v){m_UpdFieldSet.Add(fid, TLV(v));}
    void Delta(unsigned int fid,  int v){m_UpdFieldSet.Add(fid, TLV(v));}
    void Delta(unsigned int fid,  int64_t v){m_UpdFieldSet.Add(fid, TLV(v));}
};  

//数据平台响应包基类
class DpQueryRsp : public BaseDpRsp
{
public:
 
    /*
    *功能：解析数据平台的响应
    *IN buf:响应包
    *IN buf_len:响应包的长度
    *返回值：0成功，非0失败
    */
     virtual int ParseRsp(const char *buf, unsigned int buf_len);

     /*
    *功能：取数据
    *如果数组越界，会throw异常。
    *IN row_idx:行号(从0开始计数)
    *IN col_idx:列号(从0开始计数)
    *返回值：取得的数据
    */
     TLV &GetData(unsigned int row_idx, unsigned int col_idx)
     {
      return m_RetData[row_idx*m_RetFields.size() + col_idx];
     }

     /*
    *功能：取返回的字段个数
    *返回值：字段个数
    */
    unsigned int GetFieldNum(){return m_RetFields.size();}

    /*
    *功能:取得返回的字段ID
    *返回值:字段ID
    */
    unsigned int GetFieldID(unsigned int col_idx)
    {
        return m_RetFields[col_idx];
    }

    
private:
     DpFieldSet m_RetFields;
     vector<TLV> m_RetData;
};


//数据平台update响应类
typedef BaseDpRsp DpUpdateRsp;

//数据平台insert响应类
typedef BaseDpRsp DpInsertRsp;

//数据平台delete响应类
typedef BaseDpRsp DpDeleteRsp;

//数据平台count响应类
typedef BaseDpRsp DpCountRsp;

//数据平台delta响应类
typedef BaseDpRsp DpDeltaRsp;



//组合请求类
class DpComboReq : public BaseDpReq
{
public:
    DpComboReq()
    {
        m_Timeout = 0;
        m_Header.main_cmd = COMBO_REQUEST_MAIN_CMD;
    }
    virtual ~DpComboReq(){}
    
    void SetTimeout(unsigned short t){m_Timeout = t;}

    /*
    *功能：生成数据包
    *OUT req:生成的数据包
    *IN pCookie:cookie数据. 
    *返回值：生成的数据包的大小。<=0出错
    */
   virtual int MakeReq(string &req)
    {
        Packer pk;
        MakeReq(pk);
        req = pk.GetPack(m_Header);
        return req.size();
    }

    /*
    *功能：生成数据包
    *OUT buf:生成的数据包
    *IN buf_len:buf的大小
    *IN pCookie:cookie数据. 
    *返回值：生成的数据包的大小。<=0出错
    */
   virtual int MakeReq(char *buf, unsigned int buf_len)
    {
        FixedPacker pk;
        int ret = pk.Init(buf, buf_len);
        if(ret != 0)
        {
            return -1;
        }
        MakeReq(pk);
        unsigned int err = pk.GetErr();
        if(err != 0)
        {
            return -1;
        }
        int len = 0;
        pk.GetPack(len, m_Header);
        return len;
    }


    /*
    *功能：添加一条数据平台请求包
    *IN jcache_server_type:要发送到哪个jcache
    *IN p:请求包
    *返回值：无
    */
    void Add(unsigned int jcache_server_type, BaseDpReq *p)
    {
        m_ComboReq.push_back(std::make_pair<BaseDpReq *, unsigned int>(p,jcache_server_type));
    }
    

protected:
    template<class PACKER>
    void MakeReq(PACKER &pk)
    {
        pk.PackWord(m_Timeout);
        unsigned short req_num = m_ComboReq.size();
        pk.PackWord(req_num);     //条件表达式数目

        unsigned int table_id = 0;
        BaseDpReq *pReq = 0;
        for(unsigned short i = 0; i < req_num; i++)
        {
            table_id = m_ComboReq[i].second;
            pReq = m_ComboReq[i].first;
            unsigned int route_key = pReq->GetRouteKey();
            pk.PackDWord(table_id);
            pk.PackDWord(route_key);
            string req;
            pReq->MakeReq(req);
            unsigned int req_size = req.size();
            pk.PackDWord(req_size);
            pk.PackBinary(req.data(), req_size);
        }

        PackCookie(pk);
    }

    typedef std::pair<BaseDpReq *, unsigned int> item_type;
    vector<item_type> m_ComboReq;    
    unsigned short m_Timeout;
};

//组合响应包
class DpComboRsp : public BaseDpRsp
{
public:
    DpComboRsp(){}
    virtual ~DpComboRsp();

    /*
    *功能：解析数据平台的响应
    *IN buf:响应包
    *IN buf_len:响应包的长度
    *返回值：0成功，非0失败
    */
    virtual int ParseRsp(const char *buf, unsigned int buf_len);

    /*
    *功能：取组合中的某一个响应
    *OUT cmd:响应包的类型
    *IN idx:第几个响应包
    *返回值：响应包. NULL失败
    */
    BaseDpRsp *GetRsp( unsigned short &cmd, unsigned int idx)
    {
        if(idx >= m_RecordNum)
        {
            return NULL;
        }
        item_type &t = m_ComboRsp[idx];
        cmd = t.second;
        return t.first;                    
    }

    /*
    *功能:是否所有子请求都成功了。
    *返回值:true:是，　false:不是
    */
    bool IsAllCmdOK()
    {
        unsigned short cmd=0;
        for(unsigned int i = 0; i < m_RecordNum; i++)
        {
            BaseDpRsp *p = GetRsp(cmd, i);
            if(!p)
            {
                return false;
            }
            if(p->GetRetCode() != 0)
            {
                return false;
            }
        }
        return true;
    }
protected:
    typedef std::pair<BaseDpRsp *, unsigned int> item_type;
    vector<item_type> m_ComboRsp;

};


//模糊查找请求类
class DpCondFindReq:public BaseDpReq
{
public:

    /*
    *功能:构造函数
    */
    DpCondFindReq():m_nLimitNumber(0),m_nStatus(0){m_Header.main_cmd = COND_FIND_MAIN_CMD;}

    /*
    *功能:析构函数
    */
    virtual ~DpCondFindReq(){}

    /*
    *功能：生成数据包
    *OUT req:生成的数据包
    *返回值：生成的数据包的大小。<=0出错
    */
   virtual int MakeReq(string &req);

    /*
    *功能：生成数据包
    *OUT buf:生成的数据包
    *IN buf_len:buf的大小
    *返回值：生成的数据包的大小。<=0出错
    */
    virtual int MakeReq(char *buf, unsigned int buf_len);

    

    /*
    *功能：添加条件
    *IN fid:字段ID
    *IN op_type:条件类型
    *IN TLV:数据值. 
    *返回值：无
    *备注:查找结果集中只有UIN
    */
    void AddCond(unsigned int fid, unsigned char optype, const TLV &v){m_CondSet.Add(fid,optype,v);}

    /*
    *功能:添加条件
    *IN limit_number:最大返回的记录数
    *IN status:在线状态：0-所有，1-离线，2-在线
    *返回值:无
    */
    void AddCond(unsigned short limit_number,unsigned short status){m_nLimitNumber = limit_number;m_nStatus = status;}


  private:
    DpFieldValueSet m_CondSet;
    unsigned short m_nLimitNumber;
    unsigned short m_nStatus;

    
};

class DpCondFindRsp:public BaseDpRsp
{
public:
    DpCondFindRsp(){}
    virtual ~DpCondFindRsp(){}

    /*
    *功能：解析数据平台的响应
    *IN buf:响应包
    *IN buf_len:响应包的长度
    *返回值：0成功，非0失败
    */
    virtual int ParseRsp(const char *buf, unsigned int buf_len);

    /*
    *功能:取在线状态的结果数
    *返回值:在线的结果数
    */
    unsigned int GetOnlineRecordNum(){return m_RetOnLineUIN.size();}

     /*
    *功能:取在线状态的结果数
    *返回值:在线的结果数
    */
    unsigned int GetOffLineRecordNum(){return m_RetOffLineUIN.size();}

    /*
    *功能:取在线用户的UIN
    *参数说明
    *index:在线用户列表下标
    *返回值:uin
    */
    unsigned int GetOnLineUin(unsigned int index){return m_RetOnLineUIN[index];}

    /*
    *功能:取离线用户的UIN
    *参数说明
    *index:在线用户列表下标
    *返回值:uin
    */
    unsigned int GetOffLineUin(unsigned int index){return m_RetOffLineUIN[index];}

private:
    
     vector<unsigned int> m_RetOnLineUIN;//在线用户结果集
     vector<unsigned int> m_RetOffLineUIN;//离线用户结果集
    
};


//模糊查找应答类
};


#if 0
//for example:
using namespace protocol;
int GetBlogArticle(unsigned int UIN, unsigned int aid)
{
        string pack;
        bool is_primary_key = true;
        unsigned short cond_num = 2;
        ConditionExpression exp[2];

        exp[0].field_id = FIELD_ID_BLOG_ARTICLE_UIN;
        exp[0].op_code = OP_EQUAL;
        exp[0].data.type = TLV_DWORD;
        exp[0].data.length = 4;
        exp[0].data.int_value = UIN;

        exp[1].field_id = FIELD_ID_BLOG_ARTICLE_AID;
        exp[1].op_code = OP_EQUAL;
        exp[1].data.type = TLV_DWORD;
        exp[1].data.length = 4;
        exp[1].data.int_value = aid;

        const char *logic_expression = "$1 AND $2";
        unsigned short ret_field_num = 2;
        unsigned int ret_filedid_set[2];
        ret_filedid_set[0] = FIELD_ID_BLOG_ARTICLE_TITLE;
        ret_filedid_set[1] = FIELD_ID_BLOG_ARTICLE_CONTENT;

        static unsigned int seq = 0;

        int ret = DataPlatformProtocol::PackQueryReq(pack, 
            is_primary_key, cond_num, exp, logic_expression, 
            ret_field_num, ret_filedid_set, seq++);
        if(ret != RET_OK)
        {
                return -1;
        }

        send(fd, pack.data(), pack.size(), 0);
        return 0;
};
#endif

#endif


