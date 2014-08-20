/********************************
*文件名：protocol_pack.h
*功能：协议打包解包类
*作者：张荐林
*时间：2009.05.20
**********************************/


#ifndef _PROTOCOL_PACK_H_
#define _PROTOCOL_PACK_H_

#ifdef _WIN32
#define _WIN32_WINNT 0x0400
#include <winsock2.h>
#include <windows.h>
#pragma comment(lib,"wsock32.lib")
typedef unsigned __int64 uint64_t;
#else
#include <arpa/inet.h>
#include <stdint.h>
#endif


//#include <arpa/inet.h>
//#include <stdint.h>

#include <string>
#include <sstream>
using std::string;
using std::ostringstream;

namespace protocol
{
/*消息头标记*/
const unsigned char SOH = 'J';
/*消息尾标记*/
const unsigned char EOT = 'W';

/*协议主版本号*/
const unsigned char MAIN_VER = 1;
/*协议子版本号*/
const unsigned char SUB_VER = 0;

/*加密方式：不加密*/
const unsigned char ENC_TYPE_NONE = 0;

/*加密方式：TEA加密*/
const unsigned char ENC_TYPE_TEA = 1;

/*来源类型定义7bit,高1bit为Server内部使用*/
//========Client 1-31,0暂时保留=============
const unsigned char SOURCE_TYPE_MIN_IM = 1;
const unsigned char SOURCE_TYPE_MAX_IM = 31;

const unsigned char SOURCE_TYPE_IM_CLIENT = 1;//IM

//========Web 32-63=======================
const unsigned char SOURCE_TYPE_MIN_WEB = 32;
const unsigned char SOURCE_TYPE_MAX_WEB = 63;

const unsigned char SOURCE_TYPE_WEB_SNS = 32;//SNS社区
const unsigned char SOURCE_TYPE_WEB_HOME = 33;//788111门户
const unsigned char SOURCE_TYPE_WEB_TAO = 34;//淘股天堂

//=========DYJ Client 64-127==================
const unsigned char SOURCE_TYPE_DYJ_MIN_CLIENT = 64;
const unsigned char SOURCE_TYPE_DYJ_MAX_CLIENT = 127;

const unsigned char SOURCE_TYPE_DYJ_CLIENT_XGW = 64;//选股王
const unsigned char SOURCE_TYPE_DYJ_CLIENT_PCW = 66;//评测王
const unsigned char SOURCE_TYPE_DYJ_CLIENT_GGW = 68;//股票管家
const unsigned char SOURCE_TYPE_DYJ_CLIENT_GBS = 70;//股博士

//====================================

const unsigned int MSG_TOTAL_LEN_POS = 4;
const unsigned int MSG_MAIN_CMD_POS = 7;
const unsigned int MSG_SUB_CMD_POS = 9;
const unsigned int MSG_SEQ_POS = 11;
const unsigned int MSG_FROM_UIN_POS = 15;
const unsigned int MSG_TO_UIN_POS = 19;
const unsigned int MSG_ENC_TYPE_POS = 23;
const unsigned int MSG_SOURCE_TYPE_POS = 24;
const unsigned int MSG_REV_POS = 25;
const unsigned int MSG_HEADER_LEN = 29;


/*
*功能：取数据包头中的长度字段。
            注：数据包是网络序。
            长度要>=7
*IN p:数据包地址
*返回值：包的总长度
*/
inline unsigned int GetPackLen(const unsigned char *p)
{
    unsigned int *q = (unsigned int *)(p+3);
    return ntohl(*q);
}

inline void SetPackLen(unsigned char *p, unsigned int len)
{
    unsigned int *q = (unsigned int *)(p+3);
    *q =  htonl(len);
}

/*
*功能：设置数据包头中的命令字字段。
            注：数据包是网络序。
            长度要>=11
*IN p:数据包地址
*IN main:主命令字
*IN sub:子命令字
*返回值：
*/
inline void SetPackCmd(const unsigned char *p, unsigned short main, unsigned short sub)
{
    unsigned short *q = (unsigned short *)(p+MSG_MAIN_CMD_POS);
    *q = htons(main);
    *(q+1) = htons(sub);
}

inline void GetPackCmd(const unsigned char *p, unsigned short &main, unsigned short &sub)
{
    unsigned short *q = (unsigned short *)(p+MSG_MAIN_CMD_POS);
    main = ntohs(*q);
    sub = ntohs(*(q+1));
}

inline void SetFromUin(const unsigned char *p, unsigned int from_uin)
{
    unsigned int *q = (unsigned int *)(p+MSG_FROM_UIN_POS);
    *q = htonl(from_uin);
}

inline unsigned int GetFromUin(const unsigned char *p)
{
    unsigned int *q = (unsigned int *)(p+MSG_FROM_UIN_POS);
    return ntohl(*q);
}

inline void SetToUin(const unsigned char *p, unsigned int to_uin)
{
    unsigned int *q = (unsigned int *)(p+MSG_TO_UIN_POS);
    *q = htonl(to_uin);
}

inline unsigned int GetToUin(const unsigned char *p)
{
    unsigned int *q = (unsigned int *)(p+MSG_TO_UIN_POS);
    return ntohl(*q);
}

inline void SetSequence(const unsigned char *p, unsigned int seq)
{
    unsigned int *q = (unsigned int *)(p+MSG_SEQ_POS);
    *q = htonl(seq);
}

inline unsigned int GetSequence(const unsigned char *p)
{
    unsigned int *q = (unsigned int *)(p+MSG_SEQ_POS);
    return  ntohl(*q);
}



/*
*功能:设置保留字段
*参数说明:
*   IN p:数据包地址
*   IN rev:保留字段
* 返回值:无
*/
inline void SetReserve(unsigned char *p, unsigned char rev)
{
    unsigned int *q = (unsigned int *)(p+MSG_REV_POS);
    unsigned int reserve = ntohl(*q);
    reserve = (rev<<24) | (reserve & 0x00FFFFFF);
    *q = htonl(reserve);
}

/*
*功能:读取保留字段
*参数说明:
*   IN p:数据包地址
* 返回值:保留字段内容
*/
inline unsigned char GetReserve(const unsigned char *p)
{
    unsigned int *q = (unsigned int *)(p+MSG_REV_POS);
    return  (unsigned char ) (ntohl(*q) >> 24);
}

/*
*功能:设置Cookie的位置
*参数说明:
*   IN p:数据包地址
*   IN offset:cookie的偏移位置
* 返回值:无
*/
inline void SetCookiePos(unsigned char *p, unsigned int pos)
{
    unsigned int *q = (unsigned int *)(p+MSG_REV_POS);
    unsigned int reserve = ntohl(*q);
    reserve = (reserve & 0xFF000000 ) | (pos & 0x00FFFFFF);
    *q = htonl(reserve);
}

/*
*功能:取得Cookie的位置
*参数说明:
*   IN p:数据包地址
* 返回值:
* 0:无cookie
* >0:Cookie的位置
*/
inline unsigned int GetCookiePos(const unsigned char *p)
{
    unsigned int *q = (unsigned int *)(p+MSG_REV_POS);
    return  (ntohl(*q) & 0x00FFFFFF);
}

/*
* 功能:设置cookie信息
* 参数说明:
* IN p:数据包地址
* IN len:数据包buf的长度
* IN ck:cookie信息地址
* IN ckLen:cookie信息长度
* 返回值: 
* 成功:返回消息包总长度,失败<0
*/
inline int SetCookie(unsigned char * p,unsigned int len,const void * ck,unsigned short ckLen )
{        
    unsigned int pack_len = GetPackLen(p);
    unsigned int ck_pos = GetCookiePos(p);
    unsigned short ori_ck_len = 0;//原来cookie信息长度
    unsigned short * l = (unsigned short *)(p + ck_pos);
    if(ck_pos == 0 )
    {
        //原来无cookie信息
        l =(unsigned short *) (p + pack_len - 1);//Pack Tail      
    }
    else
    {
        ori_ck_len = ntohs(*l);//原来cookie的长度
        pack_len -= (2 + ori_ck_len);//修正pack_len
    }

    if( len < (pack_len + 2 + ckLen ) )
        return -1;//buf太小了
    
    *l = htons(ckLen);
    unsigned char * v = (unsigned char *) (l + 1);
    memcpy(v,ck,ckLen);
    *(v+ckLen) = EOT;

    SetCookiePos(p,pack_len - 1);
    SetPackLen(p,pack_len + 2 + ckLen);    
    return (int)(pack_len + 2 + ckLen);
}

inline int SetCookie(unsigned char * p,unsigned int len,const string & cookie)
{
    if(cookie.size() > 0xFFFF )
        return -2;//cookie 太长
        
    return SetCookie(p,len,cookie.data(),(unsigned short )cookie.size());
}

/*
* 功能:取得消息包中的cookie信息
* 参数说明:
* IN p:数据包地址
* OUT ck:cookie信息
* IN OUT ckLen:
    IN 表示cookie接收缓冲区的长度
    OUT表示实际cookie长度
* 返回值: 
*  0:成功
*  <0:失败
*/
inline int GetCookie(const unsigned char *p,void * ck,unsigned short & ckLen)
{
    unsigned int ck_pos = GetCookiePos(p);
    if(ck_pos == 0 )
    {
        //没有cookie信息
        ckLen = 0;
        return 0;
    }
    
    unsigned short * l = (unsigned short * ) (p + ck_pos);
    unsigned short ck_len = ntohs(*l);
    if(ckLen < ck_len )
    {
        //ck缓冲太小,返回实际cookie长度
        ckLen = ck_len;
        return -1;
    }
    
    unsigned char * v = (unsigned char *)( l + 1);
    memcpy(ck,v,ck_len);
    ckLen = ck_len;    
    return 0;
}

/*
* 功能:取得消息包中的cookie信息
* 参数说明:
* IN p:消息包头指针
* OUT ck:cookie信息
* 返回值: 
* 成功:true,失败:false
*/
inline int GetCookie(const unsigned char *p,string & ck)
{
    unsigned int ck_pos = GetCookiePos(p);
    if(ck_pos == 0 )
    {
        //没有cookie信息
        return 0;
    }
    
    unsigned short * l = (unsigned short * ) (p + ck_pos);
    unsigned short ck_len = ntohs(*l);
    if(ck_len == 0 )
    {
        ck = "";
        return 0;//cookie内容为空
    }
        
    unsigned char * v = (unsigned char *)( l + 1);
    ck.assign((const char *)v,ck_len);
    return 0;
}

/*
* 功能:删除消息包中的cookie信息
* 参数说明:
* IN p:消息包头指针
* 返回值: 
* 成功:>0消息包的长度,<=0失败
*/
inline int DelCookie(unsigned char * p)
{
    unsigned  int  pack_len = GetPackLen(p);
    unsigned int ck_pos =GetCookiePos(p);
    if(ck_pos <= MSG_HEADER_LEN || ck_pos >= pack_len )
    {
        //没有cookie信息
        return pack_len;
    }
        
    unsigned short * l = (unsigned short * ) (p + ck_pos);
    unsigned short ck_len = ntohs(*l);
    pack_len -= (2 + ck_len);
    unsigned char * v = (unsigned char * ) l;
    *v = EOT;
    SetCookiePos(p,0);
    SetPackLen(p,pack_len);
    return (int)pack_len;
}


inline unsigned char GetEncType(const unsigned char *p){return p[MSG_ENC_TYPE_POS];}
inline void SetEncType(unsigned char *p, unsigned int type){p[MSG_ENC_TYPE_POS]=type;}


/*消息头定义*/
class MsgHeader
{
public:
	MsgHeader():soh(SOH), main_ver(MAIN_VER), sub_ver(SUB_VER),
		main_cmd(0), sub_cmd(0), seq(0), from_uin(0), to_uin(0),
		enc_type(ENC_TYPE_NONE), source_type(SOURCE_TYPE_IM_CLIENT),rev(0)
	{
	}
	
public:
	unsigned char soh;              /*消息开始标记*/
	unsigned char main_ver;     /*协议主版本号*/
	unsigned char sub_ver;      /*协议子版本号*/
	unsigned int pack_len;    /*整个包的长度，包括包头，包体，包尾*/
	unsigned short main_cmd;    /*主命令字*/
	unsigned short sub_cmd;     /*子命令字*/
	unsigned int seq;                   /*序列号*/
	unsigned int from_uin;          /*发送方的uin*/
	unsigned int to_uin;              /*接收方的uin,没有就填0*/
	unsigned char enc_type;     /*加密方式*/
	unsigned char source_type;  /*消息来源*/
	unsigned int rev;                   /*保留字段*/
};




/*
打包类
*/
class Packer
{
public:
	Packer();
	virtual ~Packer();
        /*
        *       功能：重置打包器
        */
	void Reset();
        /*
        *       功能：打包8bit整数
        *       IN v:要打包的数据
        *       返回值：*this对象
        */
	Packer &PackByte(unsigned char v);
        /*
        *       功能：打包16bit整数
        *       IN v:要打包的数据
        *       返回值：*this对象
        */
	Packer &PackWord(unsigned short v);

        /*
        *       功能：打包32bit整数
        *       IN v:要打包的数据
        *       返回值：*this对象
        */
	Packer &PackDWord(unsigned int v);

        /*
        *       功能：打包64bit整数
        *       IN v:要打包的数据
        *       返回值：*this对象
        */
	Packer &PackQWord(uint64_t v);
        /*
        *       功能：打包浮点数
        *       IN v:要打包的数据
        *       返回值：*this对象
        */
	Packer &PackReal(double v);
    
        /*
        *       功能：打包二进制
        *       IN v:要打包的数据
        *       IN len:数据长度
        *       返回值：*this对象
        */
	Packer &PackBinary(const void *v, unsigned int len);
        /*
        *       功能：打包字符串
        *       IN v:要打包的数据
        *       返回值：*this对象
        */
	Packer &PackString(const string &v);

        /*
        *       功能：打包字符串
        *       IN v:要打包的数据
        *       返回值：*this对象
        */
	Packer &PackString(const char *v);

        /*
        *       功能：获取生成的包体
        *       返回值：生成的消息包体
        */
	string GetBody();

        /*
        *       功能：生成完整的消息包
        *       IN h:包头
        *       返回值：生成的消息包
        */
	string GetPack(const MsgHeader &h);

        /*
        *       功能：生成完整的消息包
        *       IN h:包头
        *       IN key:加密的密钥
        *       IN key_len:密钥的长度
        *       返回值：生成的消息包
        */
	string GetPack(const MsgHeader &h, const void *key, unsigned int key_len);

        /*
        *       生成完整的消息包
        *       buf为输出缓冲区，生成的包放在buf里面。
        *       len为输出缓冲区长度
        *       返回值：<=0:buf长度不够, >0，消息包的长度
        */
	//int GetPack(char *buf, unsigned int len, const MsgHeader &h);
	//int GetPack(char *buf, unsigned int len, const MsgHeader &h, const void *key, unsigned int key_len);


        /*
        *   功能:生成带cookie的完整消息包
        *   IN h:包头
        *   IN ck:cookie信息
        *   IN ck_len:cookie信息长度
        *   返回值:生成的消息包
        */
        string GetPack(const MsgHeader & h,unsigned short ck_len,const char * ck);

        
        /*
        *hbuf:生成的header。
        *body_size:包体的长度，不包括包头和包尾
        */
        static void PackHeader(char *hbuf, const MsgHeader &h, unsigned int body_size);

        /*
        pack即是输入参数，也是输出参数。
            加密消息包，pack是一个完整的消息包。
            调用后，pack中已经是加密后的数据。
            加密后数据长度不变
        */
	static void EncryptPack(void *pack, unsigned int len, const void *key, unsigned int key_len=16);
protected:
	ostringstream m_oss;
};




class FixedPacker
{
public:
	FixedPacker(char *buf = NULL, unsigned int len = 0){Init(buf, len);}
	virtual ~FixedPacker(){}
    unsigned int GetErr(){return m_Err;}
    int Init(char *buf, unsigned int len);
    int LeftSpace(){ return m_BufLen - (m_pTail - m_pBuf);}
	FixedPacker &PackByte(unsigned char v);
	FixedPacker &PackWord(unsigned short v);
	FixedPacker &PackDWord(unsigned int v);
	FixedPacker &PackQWord(uint64_t v);
	FixedPacker &PackReal(double v);
    
	FixedPacker &PackBinary(const void *v, unsigned int len);
	FixedPacker &PackString(const string &v);
	FixedPacker &PackString(const char *v);
	string GetBody();

        /*
        *len:返回生成包的长度，<=0失败，>0成功
        *返回值：包的指针
        */
	char *GetPack(int &len, const MsgHeader &h);
	char *GetPack(int &len, const MsgHeader &h, const void *key, unsigned int key_len);


        /*
        *   功能:生成带cookie的完整消息包
        *   OUT len:返回生成包的长度，<=0失败，>0成功
        *   IN h:包头
        *   IN ck:cookie信息
        *   IN ck_len:cookie信息长度
        *   返回值:包的指针
        */
        char * GetPack(int & len,const MsgHeader & h,unsigned short ck_len,const char * ck);

    
        /*
        *       生成完整的消息包
        *       buf为输出缓冲区，生成的包放在buf里面。
        *       len为输出缓冲区长度
        *       返回值：<=0:buf长度不够, >0，消息包的长度
        */
	//int GetPack(char *buf, unsigned int len, const MsgHeader &h);
	//int GetPack(char *buf, unsigned int len, const MsgHeader &h, const void *key, unsigned int key_len);

        /*
            pack即是输入参数，也是输出参数。
            加密消息包，pack是一个完整的消息包。
            调用后，pack中已经是加密后的数据。
            加密后数据长度不变
        */
	static void EncryptPack(void *pack, unsigned int len, const void *key, unsigned int key_len=16);
protected:
        char *m_pBuf;
        char *m_pTail;
        int m_BufLen;
	unsigned int m_Err;
        
};









/*
解包类
*/
class UnPacker
{
public:
	UnPacker();
	virtual ~UnPacker();

        /*
        初始化，返回0成功，其它代表包格式错误
        */
	int Init(const void *data, unsigned int len);

	/*检查包格式是否正确*/
	int CheckPack(const void *data, unsigned int len);
	
	void GetHeader(MsgHeader &h);

        /*
        解密包，原来的包的数据会被修改为解密后的数据。
        */
	int Decrypt(const void *key, unsigned int len=16);

        /*
        解密包，原来的包的数据不会被修改
        */
	int DumpDecrypt(const void *key, unsigned int len=16);


        /*
        pack即是输入参数，也是输出参数。
            解密消息包，pack是一个完整的消息包。
            调用后，pack中已经是解密后的数据。
            解密后数据长度不变
        */
        static void DecryptPack(void *pack, unsigned int len, const void *key, unsigned int key_len=16);

	
	void Reset();
	unsigned char UnPackByte();
	unsigned short UnPackWord();
	unsigned int UnPackDWord();
	uint64_t UnPackQWord();
        double UnPackReal();

        /*
        * v:解析出的二进制数据,内存要先分配好，足够len长度
        *len:二进制数据的长度
        *返回0代表成功
        */
	unsigned int UnPackBinary(void * v, unsigned int len);

        /*
        *len:二进制数据的长度
        *返回数据的指针,返回NULL代表出错
        */
	const char *UnPackBinary(unsigned int len);

        /*
        * data:解析出的二进制数据
        *len:二进制数据的长度
        *返回0代表成功
        */
        unsigned int UnPackBinary(string &data, unsigned int len);

    
	string UnPackString();
	const char *UnPackString(unsigned int &len);
	
	unsigned int GetErr();
protected:
	const char *m_pData;
	unsigned int m_Len;
	unsigned int m_Err;
	string m_Body;
	const char *m_pBody;
	unsigned int m_BodyLen;
	unsigned int m_Offset;
};

}

#endif


