
/********************************
*文件名：protocol_pack.cpp
*功能：数据平台协议解析类
*作者：张荐林
*版本：1.0
*最后更新时间：2009.06.09
**********************************/

#include <ext/hash_map>
#include <utility>

using namespace __gnu_cxx;
using std::pair;
    
#include <arpa/inet.h>

#include "protocol_err.h"
#include "protocol_pack.h"
#include "data_platform_protocol.h"

namespace protocol
{

static int PackTLV(Packer &pk, const TLV &tlv)
{
    pk.PackByte(tlv.type);
    pk.PackDWord(tlv.length);
    switch(tlv.type)
    {
    case TLV_BYTE:
        pk.PackByte((unsigned char )(tlv.int_value));
        break;
    case TLV_WORD:
        pk.PackWord((unsigned short)(tlv.int_value));
        break;
    case TLV_DWORD:
        pk.PackDWord((unsigned int )(tlv.int_value));
        break;
    case TLV_QWORD:
        pk.PackQWord(tlv.int_value);
        break;
     case TLV_REAL:
        pk.PackReal(tlv.real_value);
        break;
     case TLV_STRING:
     case TLV_BINARY:
        pk.PackBinary((const void *)(tlv.string_value.data()), tlv.string_value.size());
        break;
    default:
        return ERR_PROTOCOL;
    }
    return RET_OK;
}


static int UnPackTLV(UnPacker &upk, TLV &tlv)
{
    tlv.type = upk.UnPackByte();
    tlv.length = upk.UnPackDWord();
    switch(tlv.type)
    {
    case TLV_BYTE:
        tlv.int_value = upk.UnPackByte();
        break;
    case TLV_WORD:
        tlv.int_value = upk.UnPackWord();
        break;
    case TLV_DWORD:
        tlv.int_value = upk.UnPackDWord();
        break;
    case TLV_QWORD:
        tlv.int_value = upk.UnPackQWord();
        break;
     case TLV_REAL:
        tlv.real_value = upk.UnPackReal();
        break;
     case TLV_STRING:
     case TLV_BINARY:
        upk.UnPackBinary(tlv.string_value, tlv.length);
        break;
    default:
        return ERR_PROTOCOL;
    }
    return RET_OK;
}


static int PackTLV(FixedPacker &pk, const TLV &tlv)
{
    pk.PackByte(tlv.type);
    pk.PackDWord(tlv.length);
    switch(tlv.type)
    {
    case TLV_BYTE:
        pk.PackByte((unsigned char )(tlv.int_value));
        break;
    case TLV_WORD:
        pk.PackWord((unsigned short)(tlv.int_value));
        break;
    case TLV_DWORD:
        pk.PackDWord((unsigned int )(tlv.int_value));
        break;
    case TLV_QWORD:
        pk.PackQWord(tlv.int_value);
        break;
     case TLV_REAL:
        pk.PackReal(tlv.real_value);
        break;
     case TLV_STRING:
     case TLV_BINARY:
        pk.PackBinary((const void *)(tlv.string_value.data()), tlv.string_value.size());
        break;
    default:
        return ERR_PROTOCOL;
    }
    return RET_OK;
}



#if 0
int DataPlatformProtocol::SetRouteKey(string &pack, unsigned int key, bool unique)
{
    if(pack.size() < MSG_HEADER_LEN)
    {
        return -1;
    }
    unsigned int *p = (unsigned int *)(pack.data() + MSG_TO_UIN_POS);
    *p = htonl(key);
    char *q = (char *)(pack.data() + MSG_SOURCE_TYPE_POS);
    *q = (unique ? 0 : 1);
    return 0;
}


int DataPlatformProtocol::MakeQueryReq(string &pack, bool is_primary_key, const ConditionExpression &cond, 
                                                    unsigned short ret_field_num,unsigned int *ret_filedid_set, unsigned int seq)
{
    return DataPlatformProtocol::MakeQueryReq(pack,is_primary_key, 1, &cond, NULL, ret_field_num, ret_filedid_set, seq);
 }

int DataPlatformProtocol::MakeQueryReq(string &pack,bool is_primary_key, const ConditionExpression &cond, 
                                                unsigned short ret_field_num,unsigned int *ret_filedid_set, unsigned int seq,
                                                unsigned short cookie_len, const void *cookie)
{
    return DataPlatformProtocol::MakeQueryReq(pack,is_primary_key, 1, &cond, NULL, ret_field_num, 
                                        ret_filedid_set,  seq, cookie_len, cookie);
}



int DataPlatformProtocol::MakeQueryReq(string &pack, bool is_primary_key, 
                    unsigned short cond_num, const ConditionExpression *condtions,  
                    const char *logic_expression, unsigned short ret_field_num,
                    unsigned int *ret_filedid_set, unsigned int seq)
{
    return DataPlatformProtocol::MakeQueryReq(pack,is_primary_key, cond_num,
        condtions, logic_expression, ret_field_num, ret_filedid_set,  seq, 0, 0);
}

int DataPlatformProtocol::MakeQueryReq(string &pack, bool is_primary_key, 
                    unsigned short cond_num, const ConditionExpression *condtions,  
                    const char *logic_expression, unsigned short ret_field_num,
                    unsigned int *ret_filedid_set, unsigned int seq, unsigned short cookie_len, const void *cookie)
{
    Packer pk;
    unsigned short flags = 0;
    if(is_primary_key)
    {
        flags |= PRIMARY_KEY_FLAG_MASK;
    }
    pk.PackWord(flags);
    pk.PackWord(cond_num);     //条件表达式数目

    for(unsigned short i = 0; i < cond_num; i++)
    {
        pk.PackDWord(condtions[i].field_id);
        pk.PackByte(condtions[i].op_code);
        int ret = PackTLV(pk, condtions[i].data);
        if((unsigned int )ret != RET_OK)
        {
            return ret;
        }
    }

    unsigned short logic_exp_len = 0;   
    if(logic_expression)
    {
        logic_exp_len = strlen(logic_expression);
    }
    pk.PackWord(logic_exp_len); //逻辑表达式长度
    if(logic_exp_len > 0)
    {
        pk.PackBinary(logic_expression, logic_exp_len);
    }

    pk.PackWord(ret_field_num);
    for(unsigned short i = 0; i < ret_field_num; i++)
    {
        pk.PackDWord(ret_filedid_set[i]);
    }
    pk.PackWord(cookie_len);
    if(cookie_len > 0)
    {
        pk.PackBinary(cookie, cookie_len);
    }
    MsgHeader h;
    h.main_cmd = QUERY_MAIN_CMD;
    h.sub_cmd = 0;
    h.seq = seq;
    pack = pk.GetPack(h);
    return RET_OK;

}


int DataPlatformProtocol::GetRspCmd(unsigned short &main_cmd, unsigned short &sub_cmd, 
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


int DataPlatformProtocol::ParseQueryRsp(unsigned short &status, std::vector<unsigned int> &fields,
        std::vector<TLV> &data, const char *pack, unsigned int pack_len,
        MsgHeader *h)
{
    string cookie;
    return ParseQueryRsp(cookie, status, fields, data, pack, pack_len, h);
}

int DataPlatformProtocol::ParseQueryRsp(string &cookie, unsigned short &status, std::vector<unsigned int> &fields,
        std::vector<TLV> &data, const char *pack, unsigned int pack_len,
        MsgHeader *h)
{

    UnPacker upk;
    int ret = upk.Init(pack, pack_len);
    if(RET_OK != (unsigned int )ret)
    {
        return ret;
    }
    status = upk.UnPackWord();
    unsigned short field_num = upk.UnPackWord();
    unsigned int field_id = 0;
    for(unsigned short i = 0; i < field_num; i++)
    {
        field_id = upk.UnPackDWord();
        fields.push_back(field_id);
    }
    unsigned int record_num = upk.UnPackDWord();
    unsigned int tlv_num = record_num * field_num;
    for(unsigned int i = 0; i < tlv_num; i++)
    {
        TLV tlv;
        ret = UnPackTLV(upk, tlv);
        if((unsigned int)ret != RET_OK)
        {
            return ret;
        }
        data.push_back(tlv);
    }
    unsigned short cookie_len = upk.UnPackWord();
    if(cookie_len != 0)
    {
        upk.UnPackBinary(cookie, cookie_len);
    }
    
    if(h)
    {
        upk.GetHeader(*h);
    }    
    return RET_OK;
}



int DataPlatformProtocol::MakeCountReq(string &pack, 
                    unsigned short cond_num, const ConditionExpression *condtions,  
                    const char *logic_expression, unsigned int seq)
{
    return MakeCountReq(pack, cond_num, condtions, logic_expression,
        seq, 0,0);
}

int DataPlatformProtocol::MakeCountReq(string &pack, 
                    unsigned short cond_num, const ConditionExpression *condtions,  
                    const char *logic_expression, unsigned int seq, 
                    unsigned short cookie_len, const void *cookie)
{
    Packer pk;
    unsigned short flags = 0;
    pk.PackWord(flags);
    pk.PackWord(cond_num);     //条件表达式数目

    for(unsigned short i = 0; i < cond_num; i++)
    {
        pk.PackDWord(condtions[i].field_id);
        pk.PackByte(condtions[i].op_code);
        int ret = PackTLV(pk, condtions[i].data);
        if((unsigned int )ret != RET_OK)
        {
            return ret;
        }
    }

    unsigned short logic_exp_len = 0;   
    if(logic_expression)
    {
        logic_exp_len = strlen(logic_expression);
    }
    pk.PackWord(logic_exp_len); //逻辑表达式长度
    if(logic_exp_len > 0)
    {
        pk.PackBinary(logic_expression, logic_exp_len);
    }
    pk.PackWord(cookie_len);
    if(cookie_len >0)
    {
        pk.PackBinary(cookie, cookie_len);
    }
    MsgHeader h;
    h.main_cmd = COUNT_MAIN_CMD;
    h.sub_cmd = 0;
    h.seq = seq;
    pack = pk.GetPack(h);
    return RET_OK;
}


int DataPlatformProtocol::ParseCountRsp(unsigned short &status, unsigned int &record_num, 
        const char *pack, unsigned int pack_len,MsgHeader *h)
{
    string cookie;
    return ParseCountRsp(cookie, status, record_num, pack, pack_len, h);
}

int DataPlatformProtocol::ParseCountRsp(string &cookie, unsigned short &status, unsigned int &record_num, 
        const char *pack, unsigned int pack_len,MsgHeader *h)
{
    UnPacker upk;
    int ret = upk.Init(pack, pack_len);
    if(RET_OK != (unsigned int )ret)
    {
        return ret;
    }
    status = upk.UnPackWord();
    record_num = upk.UnPackWord();
    unsigned short cookie_len = upk.UnPackWord();
    if(cookie_len != 0)
    {
        upk.UnPackBinary(cookie, cookie_len);
    }
    if(h)
    {
        upk.GetHeader(*h);
    }    
    return RET_OK;
}


int DataPlatformProtocol::MakeDeleteReq(string &pack, bool is_async,
                    unsigned short cond_num, const ConditionExpression *condtions,  
                    unsigned int seq)
{
    return MakeDeleteReq(pack, is_async, cond_num, condtions, seq, 0, 0);
}

int DataPlatformProtocol::MakeDeleteReq(string &pack, bool is_async,
                    unsigned short cond_num, const ConditionExpression *condtions,  
                    unsigned int seq, unsigned short cookie_len, const void *cookie)
{
    Packer pk;
    unsigned short flags = 0;
    if(!is_async)
    {
        flags |= SYNC_FLAG_MASK;
    }
    pk.PackWord(flags);
    pk.PackWord(cond_num);     //条件表达式数目

    for(unsigned short i = 0; i < cond_num; i++)
    {
        pk.PackDWord(condtions[i].field_id);
        pk.PackByte(condtions[i].op_code);
        int ret = PackTLV(pk, condtions[i].data);
        if((unsigned int )ret != RET_OK)
        {
            return ret;
        }
    }

    unsigned short logic_exp_len = 0;   
    pk.PackWord(logic_exp_len); //逻辑表达式长度
    pk.PackWord(cookie_len);
    if(cookie_len > 0)
    {
        pk.PackBinary(cookie, cookie_len);
    }

    MsgHeader h;
    h.main_cmd = DELETE_MAIN_CMD;
    h.sub_cmd = 0;
    h.seq = seq;
    pack = pk.GetPack(h);
    return RET_OK;

}


int DataPlatformProtocol::ParseDeleteRsp(unsigned short &status, unsigned int &record_num, 
        const char *pack, unsigned int pack_len, MsgHeader *h)
{
    string cookie;
    return ParseDeleteRsp(cookie, status, record_num, pack, pack_len, h);
}
int DataPlatformProtocol::ParseDeleteRsp(string &cookie, unsigned short &status, unsigned int &record_num, 
        const char *pack, unsigned int pack_len, MsgHeader *h)
{
    UnPacker upk;
    int ret = upk.Init(pack, pack_len);
    if(RET_OK != (unsigned int )ret)
    {
        return ret;
    }
    status = upk.UnPackWord();
    record_num = upk.UnPackDWord();
    unsigned short cookie_len = upk.UnPackWord();
    if(cookie_len > 0)
    {
        upk.UnPackBinary(cookie, cookie_len);
    }
    if(h)
    {
        upk.GetHeader(*h);
    }
    return RET_OK;

}


int DataPlatformProtocol::MakeUpdateReq(string &pack,  bool is_async,
                    unsigned short cond_num, const ConditionExpression *condtions,  
                    const unsigned int update_field_num, const unsigned int *update_field_set,
                    const TLV *update_value_set,  unsigned int seq)
{
    return MakeUpdateReq(pack, is_async, cond_num, condtions,
            update_field_num, update_field_set, update_value_set, seq, 0, 0);
}

int DataPlatformProtocol::MakeUpdateReq(string &pack, bool is_async,
                unsigned short cond_num, const ConditionExpression *condtions,  
                const unsigned int update_field_num, const unsigned int *update_field_set,
                const TLV *update_value_set,  unsigned int seq,
                unsigned short cookie_len, const void *cookie)
{
    Packer pk;
    unsigned short flags = 0;
    if(!is_async)
    {
        flags |= SYNC_FLAG_MASK;
    }
    pk.PackWord(flags);
    pk.PackWord(cond_num);     //条件表达式数目

    for(unsigned short i = 0; i < cond_num; i++)
    {
        pk.PackDWord(condtions[i].field_id);
        pk.PackByte(condtions[i].op_code);
        int ret = PackTLV(pk, condtions[i].data);
        if((unsigned int )ret != RET_OK)
        {
            return ret;
        }
    }

    unsigned short logic_exp_len = 0;   
    pk.PackWord(logic_exp_len); //逻辑表达式长度
    
    pk.PackWord(update_field_num);
    for(unsigned int i = 0; i < update_field_num; i++)
    {
        pk.PackDWord(update_field_set[i]);
        PackTLV(pk, update_value_set[i]);
    }
    pk.PackWord(cookie_len);
    if(cookie_len > 0)
    {
        pk.PackBinary(cookie, cookie_len);
    }
    MsgHeader h;
    h.main_cmd = UPDATE_MAIN_CMD;
    h.sub_cmd = 0;
    h.seq = seq;
    pack = pk.GetPack(h);
    return RET_OK;

}


int DataPlatformProtocol::ParseUpdateRsp(unsigned short &status, unsigned int &record_num, 
        const char *pack, unsigned int pack_len, MsgHeader *h)
{
    string cookie;
    return ParseUpdateRsp(cookie, status, record_num, pack, pack_len, h);
}

int DataPlatformProtocol::ParseUpdateRsp(string &cookie, unsigned short &status, unsigned int &record_num, 
        const char *pack, unsigned int pack_len, MsgHeader *h)
{
    UnPacker upk;
    int ret = upk.Init(pack, pack_len);
    if(RET_OK != (unsigned int )ret)
    {
        return ret;
    }
    status = upk.UnPackWord();
    //modi by zhm
    //record_num = upk.UnPackWord();
    record_num = upk.UnPackDWord();

    //add by zhm
    unsigned short cookie_len = upk.UnPackWord();
    if(cookie_len > 0)
    {
        upk.UnPackBinary(cookie, cookie_len);
    }
    //end add by zhm
    
    if(h)
    {
        upk.GetHeader(*h);
    }

    
    return RET_OK;
}


int DataPlatformProtocol::MakeInsertReq(string &pack, bool is_async,
                    const unsigned int ins_field_num, const unsigned int *ins_field_set,
                    const TLV *ins_value_set,  unsigned int seq)
{
    return MakeInsertReq(pack, is_async,ins_field_num, ins_field_set, 
            ins_value_set, seq, 0, 0);
}

int DataPlatformProtocol::MakeInsertReq(string &pack, bool is_async,
            const unsigned int ins_field_num, const unsigned int *ins_field_set,
            const TLV *ins_value_set,  unsigned int seq, 
            unsigned short cookie_len, const void *cookie)
{
    Packer pk;
    unsigned short flags = 0;
    if(!is_async)
    {
        flags |= SYNC_FLAG_MASK;
    }
    pk.PackWord(flags);
    
    pk.PackWord(ins_field_num);
    for(unsigned int i = 0; i < ins_field_num; i++)
    {
        pk.PackDWord(ins_field_set[i]);
        PackTLV(pk, ins_value_set[i]);
    }
    pk.PackWord(cookie_len);
    pk.PackBinary(cookie, cookie_len);
    MsgHeader h;
    h.main_cmd = INSERT_MAIN_CMD;
    h.sub_cmd = 0;
    h.seq = seq;
    pack = pk.GetPack(h);
    return RET_OK;

}


int DataPlatformProtocol::ParseInsertRsp(unsigned short &status, unsigned int &record_num, 
        const char *pack, unsigned int pack_len, MsgHeader *h)
{
    string cookie;
    return ParseInsertRsp(cookie, status, record_num, pack, pack_len,h);
}

int DataPlatformProtocol::ParseInsertRsp(string &cookie, unsigned short &status, unsigned int &record_num, 
        const char *pack, unsigned int pack_len, MsgHeader *h)
{
    UnPacker upk;
    int ret = upk.Init(pack, pack_len);
    if(RET_OK != (unsigned int )ret)
    {
        return ret;
    }
    status = upk.UnPackWord();
    record_num = upk.UnPackDWord();
    unsigned short cookie_len = upk.UnPackWord();
    if(cookie_len > 0)
    {
        upk.UnPackBinary(cookie, cookie_len);
    }
    if(h)
    {
        upk.GetHeader(*h);
    }
    return RET_OK;
}

#endif




//New API....

int CommCookie::Pack(string &ck) 
{
    int ck_len =  8 + m_strExtInfo.size() + m_ReqPackLen;
    if(ck_len>0xFFFF)
    {
        return -1;
    }
    char cookie[ck_len];
    Pack(cookie, ck_len);
    ck.assign(cookie, ck_len);
    return ck_len;
}

int CommCookie::Pack(char *buf, unsigned int len)
{
    unsigned int ck_len =  8 + m_strExtInfo.size() + m_ReqPackLen;
    if(ck_len >0xFFFF)
    {
        return -1;
    }
    if(len < ck_len)
    {
        return -1;
    }
    char *cookie = buf;
    unsigned short *p2 = (unsigned short *)cookie;
    *p2 = htons(m_Source);
    unsigned int *p4 = (unsigned int *)(cookie+2);
    *p4 = htonl(m_Flow);
    p2 = (unsigned short *)(cookie+6);
    *p2 = htons(m_strExtInfo.size());
    char *p = cookie+8; //+other.size();
    if(m_strExtInfo.size() > 0)
    {
        memcpy(p, m_strExtInfo.data(), m_strExtInfo.size());
    }
    p+= m_strExtInfo.size();
    if(m_ReqPackLen > 0)
    {
        memcpy(p, m_pReqPack , m_ReqPackLen);
    }
    return ck_len;
}

int CommCookie::UnPack(const char *buf, unsigned int len)
{
    const char *pBuf = buf;
    unsigned int ck_len = len;
    unsigned int left = ck_len;
    if(left < 8)
    {
        return -1;
    }
    unsigned short *p2 = (unsigned short *)pBuf;
    m_Source = ntohs(*p2);
    pBuf += 2;
    unsigned int *p4 = (unsigned int *)pBuf;
    m_Flow = ntohl(*p4);
    pBuf += 4;
    p2 = (unsigned short *)pBuf;
    unsigned short other_size = ntohs(*p2);
    pBuf += 2;
    left -= 8;
    if(other_size > 0)
    {
        if(left < other_size)
        {
            return -2;
        }
        m_strExtInfo.assign(pBuf, other_size);
        pBuf+= other_size;
        left -= other_size;
    }
    if(left > 0)
    {
        m_ReqPackLen = left;
        m_pReqPack=pBuf;    
    }
    else
    {
        m_ReqPackLen = 0;
        m_pReqPack=0;    
    }
    return 0;
}

       
unsigned int BaseDpReq::m_DefaultSeq = 0;

void BaseDpReq::SetRouteKey(const char *rk, bool isUnique)
{
            __gnu_cxx::hash<const char *> hasher;
            unsigned int route_key = hasher(rk);
            SetRouteKey(route_key, isUnique);
}

unsigned int BaseDpReq::MakeRouteKey(const char *rk)
{
    __gnu_cxx::hash<const char *> hasher;
    unsigned int route_key = hasher(rk);
    return route_key;
}


int BaseDpRsp::GetRspCmd(unsigned short &main_cmd, unsigned short &sub_cmd, 
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



int BaseDpRsp::ParseRsp(const char *buf, unsigned int buf_len)
{
    UnPacker upk;
    int ret = upk.Init(buf, buf_len);
    if(RET_OK != (unsigned int )ret)
    {
        return ret;
    }
    m_RetCode = upk.UnPackWord();
    m_RecordNum = upk.UnPackDWord();
    UnPackCookie(upk);
    return RET_OK;
}


int DpQueryReq::MakeReq(string &req)
{
    Packer pk;
    unsigned short flags = 0;
    if(m_bPrimaryKey)
    {
        flags |= PRIMARY_KEY_FLAG_MASK;
    }
    pk.PackWord(flags);
    unsigned int cond_num =m_CondSet.size(); 
    pk.PackWord(cond_num);     //条件表达式数目

    for(unsigned short i = 0; i < cond_num; i++)
    {
        pk.PackDWord(m_CondSet[i].field_id);
        pk.PackByte(m_CondSet[i].op_code);
        int ret = PackTLV(pk, m_CondSet[i].data);
        if((unsigned int )ret != RET_OK)
        {
            return ret;
        }
    }

    unsigned short logic_exp_len = m_LogicExp.size();   
    pk.PackWord(logic_exp_len); //逻辑表达式长度
    if(logic_exp_len > 0)
    {
        pk.PackBinary(m_LogicExp.data(), logic_exp_len);
    }

    unsigned short ret_field_num = m_RetFieldSet.size();
    pk.PackWord(ret_field_num);
    for(unsigned short i = 0; i < ret_field_num; i++)
    {
        pk.PackDWord(m_RetFieldSet[i]);
    }
    
    PackCookie(pk);
    req = pk.GetPack(m_Header);
    return req.size();
}


int DpQueryReq::MakeReq(char *buf, unsigned int buf_len)
{
    FixedPacker pk;
    int ret = pk.Init(buf, buf_len);
    if(ret != 0)
    {
        return -1;
    }
    unsigned short flags = 0;
    if(m_bPrimaryKey)
    {
        flags |= PRIMARY_KEY_FLAG_MASK;
    }
    pk.PackWord(flags);
    unsigned int cond_num =m_CondSet.size(); 
    pk.PackWord(cond_num);     //条件表达式数目

    for(unsigned short i = 0; i < cond_num; i++)
    {
        pk.PackDWord(m_CondSet[i].field_id);
        pk.PackByte(m_CondSet[i].op_code);
        int ret = PackTLV(pk, m_CondSet[i].data);
        if((unsigned int )ret != RET_OK)
        {
            return ret;
        }
    }

    unsigned short logic_exp_len = m_LogicExp.size();   
    pk.PackWord(logic_exp_len); //逻辑表达式长度
    if(logic_exp_len > 0)
    {
        pk.PackBinary(m_LogicExp.data(), logic_exp_len);
    }

    unsigned short ret_field_num = m_RetFieldSet.size();
    pk.PackWord(ret_field_num);
    for(unsigned short i = 0; i < ret_field_num; i++)
    {
        pk.PackDWord(m_RetFieldSet[i]);
    }
    
    PackCookie(pk);
    if(pk.GetErr() != 0)
    {
        return -1;
    }

    int pack_len = 0;
    pk.GetPack(pack_len, m_Header);
    return pack_len;
}



int DpUpdateReq::MakeReq(string &req)
{
    Packer pk;
    unsigned short flags = 0;
    if(!m_bAsyn)
    {
        flags |= SYNC_FLAG_MASK;
    }
    pk.PackWord(flags);
    unsigned short cond_num = m_CondFieldSet.size();
    pk.PackWord(cond_num);     //条件表达式数目

    for(unsigned short i = 0; i < cond_num; i++)
    {
        pk.PackDWord(m_CondFieldSet[i].field_id);
        pk.PackByte(m_CondFieldSet[i].op_code);
        int ret = PackTLV(pk, m_CondFieldSet[i].data);
        if((unsigned int )ret != RET_OK)
        {
            return ret;
        }
    }

    unsigned short logic_exp_len = m_LogicExp.size();   
    pk.PackWord(logic_exp_len); //逻辑表达式长度
    pk.PackBinary(m_LogicExp.data(),logic_exp_len); //逻辑表达式长度

    unsigned short update_field_num = m_UpdFieldSet.size();
    pk.PackWord(update_field_num);
    for(unsigned int i = 0; i < update_field_num; i++)
    {
        pk.PackDWord(m_UpdFieldSet[i].field_id);
        PackTLV(pk, m_UpdFieldSet[i].data);
    }
    PackCookie(pk);
    req = pk.GetPack(m_Header);
    return req.size();
}

int DpUpdateReq::MakeReq(char *buf, unsigned int buf_len)
{
    FixedPacker pk;
    int ret = pk.Init(buf, buf_len);
    if(ret != 0)
    {
        return -1;
    }
    
    unsigned short flags = 0;
    if(!m_bAsyn)
    {
        flags |= SYNC_FLAG_MASK;
    }
    pk.PackWord(flags);
    unsigned short cond_num = m_CondFieldSet.size();
    pk.PackWord(cond_num);     //条件表达式数目

    for(unsigned short i = 0; i < cond_num; i++)
    {
        pk.PackDWord(m_CondFieldSet[i].field_id);
        pk.PackByte(m_CondFieldSet[i].op_code);
        int ret = PackTLV(pk, m_CondFieldSet[i].data);
        if((unsigned int )ret != RET_OK)
        {
            return ret;
        }
    }

    unsigned short logic_exp_len = m_LogicExp.size();   
    pk.PackWord(logic_exp_len); //逻辑表达式长度
    pk.PackBinary(m_LogicExp.data(),logic_exp_len); //逻辑表达式长度

    unsigned short update_field_num = m_UpdFieldSet.size();
    pk.PackWord(update_field_num);
    for(unsigned int i = 0; i < update_field_num; i++)
    {
        pk.PackDWord(m_UpdFieldSet[i].field_id);
        PackTLV(pk, m_UpdFieldSet[i].data);
    }
    PackCookie(pk);
    if(pk.GetErr() != 0)
    {
        return -1;
    }
    
    int pack_len = 0;
    pk.GetPack(pack_len, m_Header);
    return pack_len;
}





int DpInsertReq::MakeReq(string &req)
{
    Packer pk;
    unsigned short flags = 0;
    if(!m_bAsyn)
    {
        flags |= SYNC_FLAG_MASK;
    }
    pk.PackWord(flags);

    unsigned short ins_field_num = m_InsFieldSet.size();
    pk.PackWord(ins_field_num);
    for(unsigned int i = 0; i < ins_field_num; i++)
    {
        pk.PackDWord(m_InsFieldSet[i].field_id);
        PackTLV(pk, m_InsFieldSet[i].data);
    }
    PackCookie(pk);
    req = pk.GetPack(m_Header);
    return req.size();
    
}

int DpInsertReq::MakeReq(char *buf, unsigned int buf_len)
{
    FixedPacker pk;
    int ret = pk.Init(buf, buf_len);
    if(ret != 0)
    {
        return -1;
    }
    
    unsigned short flags = 0;
    if(!m_bAsyn)
    {
        flags |= SYNC_FLAG_MASK;
    }
    pk.PackWord(flags);

    unsigned short ins_field_num = m_InsFieldSet.size();
    pk.PackWord(ins_field_num);
    for(unsigned int i = 0; i < ins_field_num; i++)
    {
        pk.PackDWord(m_InsFieldSet[i].field_id);
        PackTLV(pk, m_InsFieldSet[i].data);
    }
    PackCookie(pk);
    if(pk.GetErr() != 0)
    {
        return -1;
    }
    
    int len = 0;
    pk.GetPack(len ,m_Header);
    return len;
}




int DpDeleteReq::MakeReq(string &req)
{
    Packer pk;
    unsigned short flags = 0;
    if(!m_bAsyn)
    {
        flags |= SYNC_FLAG_MASK;
    }
    pk.PackWord(flags);
    unsigned short cond_num = m_CondFieldSet.size();
    pk.PackWord(cond_num);     //条件表达式数目

    for(unsigned short i = 0; i < cond_num; i++)
    {
        pk.PackDWord(m_CondFieldSet[i].field_id);
        pk.PackByte(m_CondFieldSet[i].op_code);
        int ret = PackTLV(pk, m_CondFieldSet[i].data);
        if((unsigned int )ret != RET_OK)
        {
            return ret;
        }
    }

    unsigned short logic_exp_len = m_LogicExp.size();   
    pk.PackWord(logic_exp_len); //逻辑表达式长度
    pk.PackBinary(m_LogicExp.data(),logic_exp_len); //逻辑表达式长度

    PackCookie(pk);

    req = pk.GetPack(m_Header);
    return req.size();
}

int DpDeleteReq::MakeReq(char *buf, unsigned int buf_len)
{
    FixedPacker pk;
    int ret = pk.Init(buf, buf_len);
    if(ret != 0)
    {
        return -1;
    }
    unsigned short flags = 0;
    if(!m_bAsyn)
    {
        flags |= SYNC_FLAG_MASK;
    }
    pk.PackWord(flags);
    unsigned short cond_num = m_CondFieldSet.size();
    pk.PackWord(cond_num);     //条件表达式数目

    for(unsigned short i = 0; i < cond_num; i++)
    {
        pk.PackDWord(m_CondFieldSet[i].field_id);
        pk.PackByte(m_CondFieldSet[i].op_code);
        int ret = PackTLV(pk, m_CondFieldSet[i].data);
        if((unsigned int )ret != RET_OK)
        {
            return ret;
        }
    }

    unsigned short logic_exp_len = m_LogicExp.size();   
    pk.PackWord(logic_exp_len); //逻辑表达式长度
    pk.PackBinary(m_LogicExp.data(),logic_exp_len); //逻辑表达式长度

    PackCookie(pk);
    if(pk.GetErr() != 0)
    {
        return -1;
    }

    int len = 0;
    pk.GetPack(len, m_Header);
    return len;
}



int DpCountReq::MakeReq(string &req)
{
    Packer pk;
    unsigned short flags = 0;
    pk.PackWord(flags);
    unsigned short cond_num = m_CondFieldSet.size();
    pk.PackWord(cond_num);     //条件表达式数目

    for(unsigned short i = 0; i < cond_num; i++)
    {
        pk.PackDWord(m_CondFieldSet[i].field_id);
        pk.PackByte(m_CondFieldSet[i].op_code);
        int ret = PackTLV(pk, m_CondFieldSet[i].data);
        if((unsigned int )ret != RET_OK)
        {
            return ret;
        }
    }

    unsigned short logic_exp_len = m_LogicExp.size();   
    pk.PackWord(logic_exp_len); //逻辑表达式长度
    if(logic_exp_len > 0)
    {
        pk.PackBinary(m_LogicExp.data(), logic_exp_len);
    }
    PackCookie(pk);
    req = pk.GetPack(m_Header);
    return req.size();

}

int DpCountReq::MakeReq(char *buf, unsigned int buf_len)
{
    FixedPacker pk;
    int ret = pk.Init(buf, buf_len);
    if(ret != 0)
    {
        return -1;
    }
    unsigned short flags = 0;
    pk.PackWord(flags);
    unsigned short cond_num = m_CondFieldSet.size();
    pk.PackWord(cond_num);     //条件表达式数目

    for(unsigned short i = 0; i < cond_num; i++)
    {
        pk.PackDWord(m_CondFieldSet[i].field_id);
        pk.PackByte(m_CondFieldSet[i].op_code);
        int ret = PackTLV(pk, m_CondFieldSet[i].data);
        if((unsigned int )ret != RET_OK)
        {
            return ret;
        }
    }

    unsigned short logic_exp_len = m_LogicExp.size();   
    pk.PackWord(logic_exp_len); //逻辑表达式长度
    if(logic_exp_len > 0)
    {
        pk.PackBinary(m_LogicExp.data(), logic_exp_len);
    }
    PackCookie(pk);
    if(pk.GetErr() != 0)
    {
        return -1;
    }
    int len = 0;
    pk.GetPack(len, m_Header);
    return len;
    
}


int DpQueryRsp::ParseRsp(const char *buf, unsigned int buf_len )
{
    UnPacker upk;
    int ret = upk.Init(buf, buf_len);
    if(RET_OK != (unsigned int )ret)
    {
        return ret;
    }
    m_RetCode = upk.UnPackWord();
    unsigned short field_num = upk.UnPackWord();
    unsigned int field_id = 0;
    for(unsigned short i = 0; i < field_num; i++)
    {
        field_id = upk.UnPackDWord();
        m_RetFields.push_back(field_id);
    }
    unsigned int record_num = upk.UnPackDWord();
    unsigned int tlv_num = record_num * field_num;
    for(unsigned int i = 0; i < tlv_num; i++)
    {
        TLV tlv;
        ret = UnPackTLV(upk, tlv);
        if((unsigned int)ret != RET_OK)
        {
            return ret;
        }
        m_RetData.push_back(tlv);
    }
    if(field_num > 0)
    {
        m_RecordNum = tlv_num / field_num;
    }
    UnPackCookie(upk);

    return RET_OK;
}


DpComboRsp::~DpComboRsp()
{
    unsigned int n = m_ComboRsp.size();
    for(unsigned int i = 0; i < n; i++)
    {
        delete m_ComboRsp[i].first;
    }
}

int DpComboRsp::ParseRsp(const char *buf, unsigned int buf_len)
{
    UnPacker upk;
    int ret = upk.Init(buf, buf_len);
    if(RET_OK != (unsigned int )ret)
    {
        return ret;
    }
    unsigned short retcode = upk.UnPackWord();
    if(retcode != 0)
    {    
        m_RetCode = DP_ERR_GENERAL;
        return -1;
    }
    m_RetCode = 0;
    m_RecordNum = upk.UnPackWord();
    unsigned int len = 0;
    const char *pRsp = 0; 
    for(unsigned int i = 0; i < m_RecordNum; i++)
    {
        len = upk.UnPackDWord();
        pRsp = upk.UnPackBinary(len);
        if(!pRsp)
        {
            m_RetCode = DP_ERR_GENERAL;
            m_RecordNum = m_ComboRsp.size();
            return -1;
        }
        unsigned short main_cmd = 0;
        unsigned short sub_cmd = 0;
        int r = BaseDpRsp::GetRspCmd(main_cmd, sub_cmd, pRsp, len);
        if(r != 0)
        {
            m_RetCode = DP_ERR_GENERAL;
            m_RecordNum = m_ComboRsp.size();
            return -1;
        }
        
        BaseDpRsp *pDpRsp = 0;
        switch(main_cmd)
        {
            case INSERT_MAIN_CMD:
                pDpRsp = new DpInsertRsp();
                break;
                
            case UPDATE_MAIN_CMD:
                pDpRsp = new DpUpdateRsp();
                break;                    

            case DELETE_MAIN_CMD:
                pDpRsp = new DpDeleteRsp();
                break;
                
            case DELTA_MAIN_CMD:
                pDpRsp = new DpDeltaRsp();
                break;
                
            case QUERY_MAIN_CMD:
                pDpRsp = new DpQueryRsp();
                break;
                
            case COUNT_MAIN_CMD:
                pDpRsp = new DpCountRsp();
                break;
            default:
                break;
        }
        if(!pDpRsp)
        {
            //delete pDpRsp;
            m_RetCode = DP_ERR_GENERAL;
            m_RecordNum = m_ComboRsp.size();
            return -1;
        }
        r = pDpRsp->ParseRsp(pRsp, len);
        if(r != 0)
        {
            delete pDpRsp;
            m_RetCode = DP_ERR_GENERAL;
            m_RecordNum = m_ComboRsp.size();
            return -1;
        }
        m_ComboRsp.push_back(std::make_pair<BaseDpRsp *, unsigned short>(pDpRsp, main_cmd));
    }

    m_RecordNum = m_ComboRsp.size();
    UnPackCookie(upk);
    return RET_OK;
}


int DpCondFindReq::MakeReq(string & req)
{
    Packer pk;
    
    pk.PackWord(m_nLimitNumber);
    pk.PackWord(m_nStatus);
    unsigned int cond_num =m_CondSet.size(); 
    pk.PackWord(cond_num);     //条件表达式个数

    for(unsigned short i = 0; i < cond_num; i++)
    {
        pk.PackDWord(m_CondSet[i].field_id);
        //pk.PackByte(m_CondSet[i].op_code);
        pk.PackWord(m_CondSet[i].op_code);
        int ret = PackTLV(pk, m_CondSet[i].data);
        if((unsigned int )ret != RET_OK)
        {
            return ret;
        }
    }
    
    PackCookie(pk);
    req = pk.GetPack(m_Header);
    return req.size();
}

int DpCondFindReq::MakeReq(char * buf, unsigned int buf_len)
{
    FixedPacker pk;
    int ret = pk.Init(buf, buf_len);
    if(ret != 0)
    {
        return -1;
    }

    pk.PackWord(m_nLimitNumber);
    pk.PackWord(m_nStatus);
    
    unsigned int cond_num =m_CondSet.size(); 
    pk.PackWord(cond_num);     //条件表达式个数

    for(unsigned short i = 0; i < cond_num; i++)
    {
        pk.PackDWord(m_CondSet[i].field_id);
        //pk.PackByte(m_CondSet[i].op_code);
        pk.PackWord(m_CondSet[i].op_code);
        int ret = PackTLV(pk, m_CondSet[i].data);
        if((unsigned int )ret != RET_OK)
        {
            return ret;
        }
    }   
    
    PackCookie(pk);
    if(pk.GetErr() != 0)
    {
        return -1;
    }

    int pack_len = 0;
    pk.GetPack(pack_len, m_Header);
    return pack_len;
}

int DpCondFindRsp::ParseRsp(const char * buf, unsigned int buf_len)
{
    UnPacker upk;
    int ret = upk.Init(buf, buf_len);
    if(RET_OK != (unsigned int )ret)
    {
        return ret;
    }
    m_RetCode = upk.UnPackWord();

    unsigned short status_num = upk.UnPackWord();
    unsigned short total_num = upk.UnPackWord();
    m_RecordNum = total_num;
    
    for(unsigned short i = 0;i<status_num;++i)
    {
        unsigned short status = upk.UnPackWord();//状态
        unsigned short count = upk.UnPackWord();//数量

        if( status == 1 ) //离线
        {
            for(unsigned short idx = 0;idx< count;++idx)
            {
                m_RetOffLineUIN.push_back(upk.UnPackDWord());
            }
        }
        else  //在线
        {
            for(unsigned short idx = 0;idx< count;++idx)
            {
                m_RetOnLineUIN.push_back(upk.UnPackDWord());
            }
        }
    }

    //m_RecordNum = m_RetOnLineUIN.size() + m_RetOffLineUIN.size();
    
    UnPackCookie(upk);

    return RET_OK;
    
}

#if 0
void Test()
{
    const unsigned int FRIENDS_UIN = 1;
    const unsigned int FRIENDS_FUIN = 2;
    const unsigned int FRIENDS_FLAG = 3;
    
    DpInsertReq req1,req2;
    unsigned int uin = 123456;
    unsigned int f_uin = 123457;
    unsigned int flag = 0;
    req1.AddInsData(FRIENDS_UIN, TLV(uin));
    req1.AddInsData(FRIENDS_UIN, TLV(f_uin));
    req1.AddInsData(FRIENDS_UIN, TLV(flag));
    req1.SetRouteKey(uin,true);

    req2.AddInsData(FRIENDS_UIN, TLV(f_uin));
    req2.AddInsData(FRIENDS_UIN, TLV(uin));
    req2.AddInsData(FRIENDS_UIN, TLV(flag));
    req2.SetRouteKey(f_uin,true);

    unsigned int table_friend = 1;
    DpComboReq creq;
    creq.Add(table_friend, &req1);
    creq.Add(table_friend, &req2);
    string req;
    creq.MakeReq(req);    
    //spp_sendto(
}
#endif




}



