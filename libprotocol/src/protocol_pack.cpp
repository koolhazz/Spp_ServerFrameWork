/********************************
*文件名：protocol_pack.cpp
*功能：协议打包解包类
*作者：张荐林
*版本：1.0
*最后更新时间：2009.05.20
**********************************/

//#include <arpa/inet.h>

#include "protocol_err.h"
#include "protocol_pack.h"

namespace protocol
{

//ErrInfoMng s_InitErrInfoMng;
const unsigned char STR_END = '\0';

//__gnu_cxx::hash_map<unsigned int, string> ErrInfoMng::m_ErrInfo;
hash_map<unsigned int, string> ErrInfoMng::m_ErrInfo;





void xxtea_long_encrypt(unsigned int *v, unsigned int len, unsigned int *k);
void xxtea_long_decrypt(unsigned int *v, unsigned int len, unsigned int *k);




#define XXTEA_MX (z >> 5 ^ y << 2) + (y >> 3 ^ z << 4) ^ (sum ^ y) + (k[p & 3 ^ e] ^ z)


void xxtea_long_encrypt(unsigned int *v, unsigned int len, unsigned int *k) 
{
	const unsigned int XXTEA_DELTA = 0x9e3779b9;
 
	unsigned int n = len - 1;
    unsigned int z = v[n], y = v[0], p, q = 6 + 52 / (n + 1), sum = 0, e;
    if (n < 1) 
	{
        return;
    }
    while (0 < q--) 
	{
        sum += XXTEA_DELTA;
        e = sum >> 2 & 3;
        for (p = 0; p < n; p++) 
		{
            y = v[p + 1];
            z = v[p] += XXTEA_MX;
        }
        y = v[0];
        z = v[n] += XXTEA_MX;
    }
}

void xxtea_long_decrypt(unsigned int *v, unsigned int len, unsigned int *k) 
{
	 const unsigned int XXTEA_DELTA = 0x9e3779b9;

    unsigned int n = len - 1;
    unsigned int z = v[n], y = v[0], p, q = 6 + 52 / (n + 1), sum = q * XXTEA_DELTA, e;
    if (n < 1) 
	{
        return;
    }
    while (sum != 0) 
	{
        e = sum >> 2 & 3;
        for (p = n; p > 0; p--) 
		{
            z = v[p - 1];
            y = v[p] -= XXTEA_MX;
        }
        z = v[n];
        y = v[0] -= XXTEA_MX;
        sum -= XXTEA_DELTA;
    }
}


void xxtea_encrypt(char *v, unsigned int len, unsigned int *k) 
{
        if(len < 4)
        {
            return;
        }
	xxtea_long_encrypt((unsigned int *)v, len / 4, k);
}



void xxtea_decrypt(char *v, unsigned int len, unsigned int *k) 
{
        if(len < 4)
        {
            return;
        }
	xxtea_long_decrypt((unsigned int *)v, len / 4, k);
}




uint64_t myntohll(uint64_t n)
{
    #if __BYTE_ORDER == __BIG_ENDIAN
        return n;
    #else

        return (((uint64_t)ntohl(n)) << 32) + ntohl(n >> 32);    
    #endif
}

uint64_t myhtonll(uint64_t n)
{
    #if __BYTE_ORDER == __BIG_ENDIAN
        return n;
    #else
        return (((uint64_t)htonl(n)) << 32) + htonl(n >> 32);
    #endif
}



Packer::Packer()
{

}

Packer::~Packer()
{

}

void Packer::Reset()
{
	m_oss.str("");
}

Packer & Packer::PackByte(unsigned char v)
{
	m_oss<<v;
	return *this;
}

Packer & Packer::PackWord(unsigned short v)
{
	unsigned short v2 = htons(v);
	m_oss.write((const char *) &v2,2);
	return *this;
}

Packer & Packer::PackDWord(unsigned int v)
{
	unsigned int v2 = htonl(v);
	m_oss.write((const char *) &v2,4);
	return *this;
}


Packer &Packer::PackQWord(uint64_t v)
{
    uint64_t v2 = myhtonll(v);
    m_oss.write((const char *) &v2,8);
    return *this;    
}

/*
*  实数会转成字符串
*/
Packer &Packer::PackReal(double v)
{
    char data[64] = {0};
    sprintf(data, "%f", v);
    return PackString(data);
}

Packer & Packer::PackBinary(const void *v, unsigned int len)
{
        if(len < 1)
        {
            return *this;
        }
	m_oss.write((const char *) v,len);
	return *this;
}

Packer & Packer::PackString(const string &v)
{
	m_oss<<v<<STR_END;
	return *this;
}

Packer & Packer::PackString(const char *v)
{
    if(v == NULL)
    {
        return *this;
    }
    m_oss<<v<<STR_END;
    return *this;
}

string Packer::GetBody()
{
	return m_oss.str();
}

void Packer::PackHeader(char *hbuf, const MsgHeader &h, unsigned int body_size)
{
	unsigned char offset = 0;
	hbuf[offset] = h.soh;
	++offset;
	hbuf[offset] = h.main_ver;
	++offset;
	hbuf[offset] = h.sub_ver;
	++offset;
	unsigned int *p = (unsigned int *)(hbuf + offset);
	*p = htonl(MSG_HEADER_LEN + body_size + 1);
	offset += 4;
	
	unsigned short *ps = (unsigned short *)(hbuf + offset);
	*ps = htons(h.main_cmd);
	offset += 2;
	++ps;
	*ps = htons(h.sub_cmd);
	offset += 2;
	unsigned int *q = (unsigned int *)(hbuf + offset);
	*q = htonl(h.seq);
	offset += 4;
	++q;
	*q = htonl(h.from_uin);
	offset += 4;
	++q;
	*q = htonl(h.to_uin);
	offset += 4;
	hbuf[offset] = h.enc_type;
	++offset;
	hbuf[offset] = h.source_type;
	++offset;
	q = (unsigned int *)(hbuf + offset);
	*q = htonl(h.rev);

}

string Packer::GetPack(const MsgHeader &h)
{
    #ifndef _WIN32
        unsigned int body_size = m_oss.str().size();
        unsigned int pack_len = MSG_HEADER_LEN+ body_size + 1;
        char hbuf[pack_len];
        PackHeader( hbuf, h, body_size);
        memcpy(hbuf + MSG_HEADER_LEN, m_oss.str().data(), body_size);
        hbuf[pack_len - 1] = EOT;
        return string(hbuf, pack_len);
    #else
        unsigned int body_size = m_oss.str().size();
        unsigned int pack_len = MSG_HEADER_LEN+ body_size + 1;
        char * hbuf = new char[pack_len];
        PackHeader( hbuf, h, body_size);
        memcpy(hbuf + MSG_HEADER_LEN, m_oss.str().data(), body_size);
        hbuf[pack_len - 1] = EOT;
        string strPack(hbuf, pack_len);
        delete hbuf;
        return strPack;
    #endif
}


string Packer::GetPack(const MsgHeader &h, const void *key, unsigned int key_len)
{
	if(h.enc_type != ENC_TYPE_TEA)
	{
		return GetPack(h);
	}
	string body(m_oss.str());
	unsigned int pack_len = MSG_HEADER_LEN+body.size() + 1;

#ifndef _WIN32
	char hbuf[pack_len];
	PackHeader(hbuf, h, body.size());
    memcpy(hbuf + MSG_HEADER_LEN, body.data(), body.size()); 
    xxtea_encrypt(hbuf + MSG_HEADER_LEN, body.size(), (unsigned int *)key);
	hbuf[pack_len - 1] = EOT;
	return string(hbuf, pack_len);
#else
    char *hbuf = new char[pack_len];
    PackHeader(hbuf, h, body.size());
    memcpy(hbuf + MSG_HEADER_LEN, body.data(), body.size()); 
    xxtea_encrypt(hbuf + MSG_HEADER_LEN, body.size(), (unsigned int *)key);
    hbuf[pack_len - 1] = EOT;
    string strPack(hbuf, pack_len);    
    delete hbuf;
    return strPack;
#endif
}


string Packer::GetPack(const MsgHeader & h, unsigned short ck_len,const char * ck)
{            
    #ifndef _WIN32
        unsigned int body_size = m_oss.str().size();
        unsigned int pack_len = MSG_HEADER_LEN+ body_size + 2 + ck_len + 1;//header + body + cklenSize + ckData + tail
        char hbuf[pack_len];
        PackHeader( hbuf, h, body_size);
        memcpy(hbuf + MSG_HEADER_LEN, m_oss.str().data(), body_size);

        unsigned short * l = (unsigned short *) (hbuf + MSG_HEADER_LEN + body_size);
        *l = htons(ck_len);
        unsigned char * v = (unsigned char *) (l + 1);
        memcpy(v,ck,ck_len);
        *(v + ck_len) = EOT;

        SetCookiePos((unsigned char *) hbuf,MSG_HEADER_LEN + body_size);
        SetPackLen((unsigned char * )hbuf,pack_len);        
        return string(hbuf, pack_len);

    #else
        unsigned int body_size = m_oss.str().size();
        unsigned int pack_len = MSG_HEADER_LEN+ body_size + 2 + ck_len + 1;//header + body + cklenSize + ckData + tail
        char *hbuf = new char[pack_len];
        PackHeader( hbuf, h, body_size);
        memcpy(hbuf + MSG_HEADER_LEN, m_oss.str().data(), body_size);

        unsigned short * l = (unsigned short *) (hbuf + MSG_HEADER_LEN + body_size);
        *l = htons(ck_len);
        unsigned char * v = (unsigned char *) (l + 1);
        memcpy(v,ck,ck_len);
        *(v + ck_len) = EOT;
        SetCookiePos((unsigned char *) hbuf,MSG_HEADER_LEN + body_size);
        SetPackLen((unsigned char * )hbuf,pack_len);        
        string strPack(hbuf, pack_len);
        delete hbuf;
        return strPack;
    #endif
}

    



void Packer::EncryptPack(void *pack, unsigned int len, const void *key, unsigned int key_len)
{
    if(len <= MSG_HEADER_LEN + 1)
    {
        return;
    }
    if(GetEncType((const unsigned char * )pack) != ENC_TYPE_TEA)
    {
        return;
    }
    char *p = (char *)pack;
    xxtea_encrypt(p+MSG_HEADER_LEN, len - MSG_HEADER_LEN - 1, (unsigned int *)key);    
}



FixedPacker & FixedPacker::PackByte(unsigned char v)
{
        if(LeftSpace() < 1)
        {
            m_Err = ERR_PROTOCOL;
            return *this;
        }
        *m_pTail = v;
        ++m_pTail;
	return *this;
}

FixedPacker & FixedPacker::PackWord(unsigned short v)
{
        if(LeftSpace() < 2)
        {
            m_Err = ERR_PROTOCOL;
            return *this;
        }

	unsigned short v2 = htons(v);
        unsigned short *p = (unsigned short *)m_pTail;
        *p = v2;
        m_pTail += 2; 
	return *this;
}

FixedPacker & FixedPacker::PackDWord(unsigned int v)
{
        if(LeftSpace() < 4)
        {
            m_Err = ERR_PROTOCOL;
            return *this;
        }

	unsigned int v2 = htonl(v);
        unsigned int *p = (unsigned int *)m_pTail;
        *p = v2;
        m_pTail += 4; 
	return *this;
}


FixedPacker &FixedPacker::PackQWord(uint64_t v)
{
    if(LeftSpace() < 8)
    {
        m_Err = ERR_PROTOCOL;
        return *this;
    }
    uint64_t v2 = myhtonll(v);
    memcpy(m_pTail, &v2, 8);
    m_pTail += 8;
    return *this;    
}

/*
*  实数会转成字符串
*/
FixedPacker &FixedPacker::PackReal(double v)
{
    char data[64] = {0};
    sprintf(data, "%f", v);
    return PackString(data);
}

FixedPacker & FixedPacker::PackBinary(const void *v, unsigned int len)
{
    if(LeftSpace() < (int)len)
    {
        m_Err = ERR_PROTOCOL;
        return *this;
    }
    if(len < 1)
    {
        return *this;
    }
    memcpy(m_pTail, v, len);
    m_pTail += len;
    return *this;    
}

FixedPacker & FixedPacker::PackString(const string &v)
{
    if(LeftSpace() < (int)(v.size()+1))
    {
        m_Err = ERR_PROTOCOL;
        return *this;
    }
    if(!v.empty())
    {
        memcpy(m_pTail, v.data(), v.size());
        m_pTail += v.size();
    }
    *m_pTail = STR_END;
    ++m_pTail;
    return *this;
}

FixedPacker & FixedPacker::PackString(const char *v)
{
    if(NULL == v)
    {
        return *this;
    }
    int vlen = strlen(v);
    if(LeftSpace() < vlen+1)
    {
        m_Err = ERR_PROTOCOL;
        return *this;
    }
    memcpy(m_pTail, v, vlen+1);
    m_pTail += vlen+1;
    return *this;    
}


int FixedPacker::Init(char *buf, unsigned int len)
{
    if(len < MSG_HEADER_LEN + 1)
    {
        m_Err = ERR_PROTOCOL;
        return -1;
    }
    m_Err = 0;
    m_pBuf = buf; 
    m_pTail = m_pBuf + MSG_HEADER_LEN;
    m_BufLen = len;
    return 0;
}

string FixedPacker::GetBody()
{
	string strBody;
	strBody.assign(m_pBuf + MSG_HEADER_LEN,m_pTail - m_pBuf - MSG_HEADER_LEN);
	return strBody;
}

/*
*len:返回生成包的长度，<=0失败，>0成功
*返回值：包的指针
*/
char *FixedPacker::GetPack(int &len, const MsgHeader &h)
{
        if(m_Err != 0)
        {
                len = 0;
                return m_pBuf;
        }
        unsigned int pack_len = m_pTail - m_pBuf+1;
        Packer::PackHeader( m_pBuf, h, pack_len - MSG_HEADER_LEN - 1);
        m_pBuf[pack_len - 1] = EOT;
        len =  pack_len;
        return m_pBuf;
}


char *FixedPacker::GetPack(int &len, const MsgHeader &h, const void *key, unsigned int key_len)
{
        if(h.enc_type != ENC_TYPE_TEA)
        {
            return GetPack(len, h);
        }

        if(m_Err != 0)
        {
            len = 0;
            return m_pBuf;
        }
        
        unsigned int pack_len = m_pTail - m_pBuf+1;
        Packer::PackHeader( m_pBuf, h, pack_len - MSG_HEADER_LEN - 1);
        xxtea_encrypt(m_pBuf + MSG_HEADER_LEN, pack_len - MSG_HEADER_LEN-1, (unsigned int *)key);
        m_pBuf[pack_len - 1] = EOT;
        len =  pack_len;
        return m_pBuf;
}


char * FixedPacker::GetPack(int & len, const MsgHeader & h, unsigned short ck_len, const char * ck)
{
    if(LeftSpace() < (2 + ck_len ) )
    {
        m_Err = ERR_PROTOCOL;
    }
    
    if(m_Err != 0)
    {
        len = 0;
        return m_pBuf;
    }
    
    unsigned int pack_len = m_pTail - m_pBuf+1;
    Packer::PackHeader( m_pBuf, h, pack_len - MSG_HEADER_LEN - 1);

    unsigned int total_pack_len = pack_len + 2 + ck_len;

    unsigned short * l = (unsigned short *) (m_pTail);
    *l = htons(ck_len);
    unsigned char * v = (unsigned char *)(l + 1);
    memcpy(v,ck,ck_len);
    m_pBuf[total_pack_len - 1] = EOT;

    SetCookiePos((unsigned char *) m_pBuf, pack_len - 1);
    SetPackLen((unsigned char *) m_pBuf,total_pack_len);
        
    len =  total_pack_len;
    return m_pBuf;
}




UnPacker::UnPacker():m_Err(RET_OK)
{

}

UnPacker::~UnPacker()
{
}

int UnPacker::Init(const void *data, unsigned int len)
{
	m_pData =(const char *) data;
	m_Len = len;
	m_pBody = m_pData + MSG_HEADER_LEN;
	m_BodyLen = len - MSG_HEADER_LEN - 1;
	m_Offset = 0;
	return CheckPack(m_pData, m_Len);
}

int UnPacker::CheckPack(const void *data, unsigned int len)
{
	const char * pData = (const char *) data;
	if( (len < MSG_HEADER_LEN + 1) || (pData == NULL))
	{
		return ERR_PROTOCOL;
	}

	
	MsgHeader h;
	GetHeader(h);
	if( h.soh != SOH)
	{
		return ERR_PROTOCOL;
	}

	if(h.main_ver != MAIN_VER)
	{
		return ERR_PROTOCOL_VER;
	}
	if(h.pack_len > len)
	{
		return ERR_PROTOCOL;
	}
        
	if(pData[h.pack_len-1] != EOT)
	{
		return ERR_PROTOCOL;
	}   
	return RET_OK;
}

void UnPacker::GetHeader(MsgHeader &h)
{
	unsigned char offset = 0;
	h.soh = m_pData[offset];
	++offset;
	h.main_ver = m_pData[offset];
	++offset;
	h.sub_ver = m_pData[offset];
	++offset;
	unsigned int *p = (unsigned int *)(m_pData + offset);
	h.pack_len =  htonl(*p);
	offset += 4;
	unsigned short *ps = (unsigned short *)(m_pData + offset);
	h.main_cmd = ntohs(*ps);
	offset += 2;
	++ps;
	h.sub_cmd = ntohs(*ps);
	offset += 2;
	unsigned int *q = (unsigned int *)(m_pData + offset);
	h.seq = ntohl(*q);
	offset += 4;
	++q;
	h.from_uin = ntohl(*q);
	offset += 4;
	++q;
	h.to_uin = ntohl(*q);
	offset += 4;
	h.enc_type = m_pData[offset];
	++offset;
	h.source_type = m_pData[offset];
	++offset;
	q = (unsigned int *)(m_pData + offset);
	h.rev = ntohl(*q);
}



int UnPacker::Decrypt(const void *key, unsigned int len)
{
        unsigned char enc_type = GetEncType((const unsigned char *)m_pData);
	if(enc_type == ENC_TYPE_NONE)
	{
		return RET_OK;
	}
	if( enc_type != ENC_TYPE_TEA)
	{
		return ERR_ENC_TYPE;
	}	
        xxtea_decrypt((char *)m_pBody,m_BodyLen,(unsigned int *)key);
	return RET_OK;
}

int UnPacker::DumpDecrypt(const void *key, unsigned int len)
{
        unsigned char enc_type = GetEncType((const unsigned char *)m_pData);    
	if(enc_type == ENC_TYPE_NONE)
	{
		return RET_OK;
	}
	if( enc_type != ENC_TYPE_TEA)
	{
		return ERR_ENC_TYPE;
	}	

        m_Body.assign(m_pBody, m_BodyLen);
        m_pBody = m_Body.data();
        xxtea_decrypt((char *)m_pBody,m_BodyLen,(unsigned int *)key);
	return RET_OK;
}


void UnPacker::DecryptPack(void *pack, unsigned int len, const void *key, unsigned int key_len)
{
    if(len <= MSG_HEADER_LEN + 1)
    {
        return;
    }
    if(GetEncType((const unsigned char * )pack) != ENC_TYPE_TEA)
    {
        return;
    }
    char *p = (char *)pack;
    xxtea_decrypt(p+MSG_HEADER_LEN, len - MSG_HEADER_LEN - 1, (unsigned int *)key);    
}


void UnPacker::Reset()
{
	m_pData = NULL;
	m_Len = 0;
	m_Body = "";
	m_pBody = NULL;
	m_BodyLen = 0;
	m_Err = RET_OK;
}

unsigned char UnPacker::UnPackByte()
{
	if(m_Offset > m_BodyLen)
	{
		m_Err =  ERR_PROTOCOL;
		return 0;
	}
		
	return m_pBody[m_Offset++];
}

unsigned short UnPacker::UnPackWord()
{
	if(m_Offset + 2 > m_BodyLen)
	{
		m_Err =  ERR_PROTOCOL;
		return 0;
	}
	m_Offset += 2;
	return ntohs(*((unsigned short *)(m_pBody + m_Offset - 2)));
}

unsigned int UnPacker::UnPackDWord()
{
	if(m_Offset + 4 > m_BodyLen)
	{
		m_Err =  ERR_PROTOCOL;
		return 0;
	}
	m_Offset += 4;
	return ntohl(*((unsigned int *)(m_pBody + m_Offset - 4)));
}

uint64_t UnPacker::UnPackQWord()
{
	if(m_Offset + 8 > m_BodyLen)
	{
		m_Err =  ERR_PROTOCOL;
		return 0;
	}
	m_Offset += 8;
	return myntohll(*((uint64_t *)(m_pBody + m_Offset - 8)));
}

double UnPacker::UnPackReal()
{
    unsigned int len = 0;
    const char *p = this->UnPackString(len);
    if(len > 0)
    {
        return atof(p);
    }
    return 0.0;
}
    
unsigned int UnPacker::UnPackBinary(void * v, unsigned int len)
{
	if(m_Offset + len > m_BodyLen)
	{
		m_Err =  ERR_PROTOCOL;
		return m_Err;
	}
	memcpy(v, m_pBody + m_Offset, len);
	m_Offset += len;
	return RET_OK;
}

const char *UnPacker::UnPackBinary(unsigned int len)
{
    if(m_Offset + len > m_BodyLen)
    {
        m_Err =  ERR_PROTOCOL;
        return NULL;
    }
    const char *p = m_pBody + m_Offset;
    m_Offset += len;
    return p;
}

unsigned int UnPacker::UnPackBinary(string &data, unsigned int len)
{
	if(m_Offset + len > m_BodyLen)
	{
		m_Err =  ERR_PROTOCOL;
		return m_Err;
	}
        if(len == 0)
        {
            return RET_OK;
        }
        data.assign(m_pBody + m_Offset, len);
	m_Offset += len;
	return RET_OK;
}


string UnPacker::UnPackString()
{
	const char *p = m_pBody + m_Offset;
	const char *q = p;
	for(; *q && (q < m_pBody + m_BodyLen); ++q);
	if(q 	>= m_pBody + m_BodyLen)
	{
		return string();
	}
	m_Offset += q -p + 1;
	return string(p, q-p);
}

const char *UnPacker::UnPackString(unsigned int &len)
{
	const char *p = m_pBody + m_Offset;
	const char *q = p;
	for(; *q && (q < m_pBody + m_BodyLen); ++q);
	if(q 	>= m_pBody + m_BodyLen)
	{
		return NULL;
	}
	len = q - p;
	m_Offset += len + 1;
	return p;
}
	

unsigned int UnPacker::GetErr()
{
	return m_Err;
}


}



