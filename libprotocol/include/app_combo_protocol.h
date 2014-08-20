
/********************************
*文件名：app_combo_protocol.h
*功能：应用组合协议解析类
*作者：钟何明
*版本：1.0
*最后更新时间：2010.06.21
**********************************/

#ifndef _APP_COMBO_PROTOCOL_H_
#define _APP_COMBO_PROTOCOL_H_

#include <stdint.h>
#include <stdlib.h>
#include <string>
#include <vector>

using std::string;
using std::vector;

#include "protocol_pack.h"
#include "protocol_err.h"
#include "data_platform_protocol.h"

namespace protocol
{

//COMBO服务器子命令字
const unsigned short COMBO_REQUEST_SUB_CMD = 0x0001;

//组合请求类
class AppComboReq
{
public:
    AppComboReq()
    {
        m_Timeout = 0;
        m_Header.main_cmd = COMBO_REQUEST_MAIN_CMD;
        m_Header.sub_cmd = COMBO_REQUEST_SUB_CMD;
    }
    ~AppComboReq(){}
    
    void SetTimeout(unsigned short t){m_Timeout = t;}

    /*
    *功能：生成数据包
    *OUT req:生成的数据包
    *返回值：生成的数据包的大小。<=0出错
    */
   int MakeReq(string &req)
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
    *返回值：生成的数据包的大小。<=0出错
    */
   int MakeReq(char *buf, unsigned int buf_len)
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
    *功能：添加一个子请求包
    *IN server_type:要发送到哪个server
    *IN route_key:路由Key
    *IN p:请求包
    *IN len:请求包长度
    *返回值：无
    */
    void Add(unsigned int server_type,unsigned int route_key, char * p,unsigned int len)
    {
        uint64_t val = server_type ;
        val = (val << 32) | route_key;
        m_ComboReq.push_back(std::make_pair<string,uint64_t>(string(p,len),val));
    }

    void Add(unsigned int server_type,unsigned int route_key,const string & pk)
    {
        uint64_t val = server_type ;
        val = (val << 32) | route_key;
        m_ComboReq.push_back(std::make_pair<string,uint64_t>(pk,val));
    }

     //设置Combo包的Seq
    void SetSeq(unsigned int seq){m_Header.seq = seq;}

    /*
    *功能：读或者写cookie
    *返回值：cookie引用
    */
    string &Cookie(){return m_Cookie;}

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

protected:
    template<class PACKER>
    void MakeReq(PACKER &pk)
    {
        pk.PackWord(m_Timeout);
        unsigned short req_num = m_ComboReq.size();
        pk.PackWord(req_num);     //条件表达式数目

        unsigned int  server_type = 0;
        unsigned int route_key = 0;
        
        for(unsigned short i = 0; i < req_num; i++)
        {
            server_type = (m_ComboReq[i].second>>32);
            route_key = m_ComboReq[i].second;
            pk.PackDWord(server_type);
            pk.PackDWord(route_key);
            string & req = m_ComboReq[i].first;
            unsigned int req_size = req.size();
            pk.PackDWord(req_size);
            pk.PackBinary(req.data(), req_size);
        }

        PackCookie(pk);
    }

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

private:

    static unsigned int m_DefaultSeq;
    string m_Cookie;//Combo自身Cookie
    MsgHeader m_Header;//Combo请求包头
    typedef std::pair<string,uint64_t> item_type;
    std::vector<item_type> m_ComboReq;//Combo子请求包
    unsigned short m_Timeout;//Combo请求超时时间
};

class AppComboRsp
{
public:
    AppComboRsp()
    {
        m_RetCode = 0xffff;
        m_RecordNum = 0;
    }
    
    ~AppComboRsp(){}

    /*
    *功能：解析数据平台的响应
    *IN buf:响应包
    *IN buf_len:响应包的长度
    *返回值：0成功，非0失败
    */
    int ParseRsp(const char *buf, unsigned int buf_len) ;

    /*
    *功能：取返回的错误码
    *返回值：返回的错误码
    */
    unsigned short GetRetCode(){return m_RetCode;}

    /*
    *功能：取返回的记录数
    *返回值：返回的记录数
    */
    unsigned int GetSubRspNum(){return m_RecordNum;}

    /*
    *功能：读或者写cookie
    *返回值：cookie引用
    */
    string &Cookie(){return m_Cookie;}

    /*
    *功能:取得子响应包
    *参数说明
    *OUT buf:响应包
    *IN buf_len:buf长度
    *IN idx:第几个响应包
    *返回值:
    * >0:响应包长度,<=0失败
    */
    int GetSubRsp(char * buf,unsigned int buf_len,unsigned short idx);


    /*
    *功能:取得子响应包
    *参数说明
    *OUT buf:响应包
    *IN idx:第几个响应包
    *返回值:
    * >0:响应包长度,<=0失败
    */
    int GetSubRsp(string& subpack,unsigned short idx);


    /*
    *功能:取得子响应包的命令字
    *参数说明
    *OUT main_cmd:响应包主命令字
    *OUT snb_cmd:响应包子命令字
    *IN idx:第几个响应包
    *返回值:
    * 0:成功,非0失败
    */
    int GetSubRspCmd(unsigned short & main_cmd,unsigned short & sub_cmd,unsigned short idx);


protected:
    void UnPackCookie(UnPacker &upk)
    {
        unsigned short cookie_len = upk.UnPackWord();
        upk.UnPackBinary(m_Cookie, cookie_len);
    }

    int GetRspCmd(unsigned short &main_cmd, unsigned short &sub_cmd, 
                const char *pack, unsigned int pack_len)
    {
        if(MSG_HEADER_LEN > pack_len)
        {
            return ERR_PROTOCOL;
        }
        unsigned short *p = (unsigned short *)(pack + MSG_MAIN_CMD_POS);
        main_cmd = htons(*p);
        sub_cmd = htons(*(p+1));
        return RET_OK;
    }

    
private:

    string m_Cookie;//Combo cookie信息
    unsigned short m_RetCode;//Combo返回码
    unsigned int m_RecordNum;//子请求数

    typedef std::pair<string, unsigned int> item_type;
    vector<item_type> m_ComboRsp;

    
};

}

#endif

