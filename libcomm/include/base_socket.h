 /********************************************
//文件名:base_socket.h
//功能:sockete函数封装，TCP客户类和服务类
//作者:钟何明
//创建时间:2009.06.11
//修改记录:

*********************************************/

#ifndef _SOCKET_INTERFACE_H_
#define _SOCKET_INTERFACE_H_

#include <pthread.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/stropts.h>
#include <netinet/in.h>

#include <errno.h>
#include <string.h>
#include <stdio.h>

#include <string>

using std::string;

namespace comm
{
namespace basesock
{

//无效SOCKET
const int  INVALID_SOCKET = -1;

/***************************************
*功能描述:封装SOCKET操作的基类
****************************************/
class CBaseSocket
{
public:
    /****************************
    //构造函数
    *****************************/
    CBaseSocket();


    /****************************
    //析构函数
    *****************************/
    virtual ~CBaseSocket(){Close();}

    /********************************
    //包长度检查的回调函数类型
    //参数说明:
    //[in] buf:接收数据缓冲
    //[in] len:包头的长度
    //返回值:
    //0还要继续收数据才能确定，
    //-1:数据出错
    //>0:包的全长
    ********************************/
    typedef int (*pfHandleInput)(const char * buf, unsigned int len);
	
private:

    /*拷贝构造函数*/
    CBaseSocket(const CBaseSocket &rhs);

    /*赋值操作*/
    CBaseSocket &operator=(const CBaseSocket &rhs);	

public:

    /**************************************
    //功能：取SOCKET描述符
    //参数说明:无
    //返回值:socket的描述符			
    ****************************************/
    inline int GetFD()const{return m_iSocket;}

    /******************************************
    //功能:将ip地址转换为逗点形式
    //参数说明:
    //[in] in:IP地址
    //[out] buf:逗点格式的IP地址
    //返回值:
    //	buf的地址
    *******************************************/
    static char *ip_ntoa(unsigned int in, char *buf)
    {  
        in_addr _in = { in }; 
        return inet_ntoa_r(_in, buf);
    }

    static char *inet_ntoa_r(unsigned int in, char *buf)
    {
        in_addr _in = {in};
        return inet_ntoa_r(_in, buf);
    }
	
    static char *inet_ntoa_r(struct in_addr in, char *buf)
    {
        register char *p;  
        p = (char *)&in;
        #define UC(b)   (((int)b)&0xff)  
        sprintf(buf, "%d.%d.%d.%d", UC(p[0]), UC(p[1]), UC(p[2]), UC(p[3])); 
        return buf;
    }

    /**************************************
    //功能:重连
    //参数说明:
    //[in] timeout:超时时间
    //返回值:
    //0:成功,-1失败
    ***************************************/
    int Reconnect(int timeout = -1);


    /******************************************
    //功能：判断是否是一个可用的SOCKET描述符
    //返回值：	
    //	true:可用
    //	false:不可用
    ***********************************************/
    inline bool IsValid(){return m_iSocket != INVALID_SOCKET;}

    /************************************
    //功能:取得连接类型
    //参数说明:无
    //返回值:
    //返回连接类型
    **************************************/
    inline int Type()const {return m_iType;}

    /**************************************
    //功能：取上次操作的错误码
    //参数说明:无
    //返回值：	
    //		返回上次操作的错误码
    ***************************************/
    inline int GetLastError()const{return errno;}


    /**************************************
    //功能：取上次操作的错误信息
    //参数说明:	无
    //返回值：	
    //	返回上次操作的错误信息
    ***************************************/
    const char *GetLastErrorMsg()const
    {
        return strerror(errno);
    }



    /**************************************
    //功能：生成新的TCP SOCKET描述符
    //参数说明:无
    //返回值：	
    //		INVALID_SOCKET:失败
    //		非INVALID_SOCKET:生成的SOCKET描述符
    ***************************************/
    int CreateTcp(){return Create(SOCK_STREAM);}

    /**************************************
    //功能：生成新的UDP SOCKET描述符
    //参数说明:	无
    //返回值：	
    //		INVALID_SOCKET:失败
    //		非INVALID_SOCKET:生成的SOCKET描述符
    *****************************************/
    int CreateUdp(){return Create(SOCK_DGRAM);}


    /**************************************
    //功能：依附到新的SOCKET描述符
    //参数说明：
    //[in] iSocket:新的SOCKET描述符
    //[in] iType:SOCKET的类型
    //返回值：	
    //	无
    *****************************************/
    void Attach(int iSocket, int iType);


    /**************************************
    //功能：取消依附
    //参数说明：
    //		无	
    //返回值：	
    //		无
    *****************************************/
    inline void Dettach()
    {
        m_iSocket = INVALID_SOCKET;
        m_iType = -1;
        memset(&m_PeerAddr, 0, sizeof(m_PeerAddr));
        memset(&m_SockAddr,0, sizeof(m_SockAddr));
    }

    /**************************************
    //功能:保存对端地址
    //参数说明:无
    //返回值:
    //成功:0,失败-1,具体错误可以通过GetLastError()获取
    *****************************************/
    int SavePeerAddr();

    /******************************************
    //功能:保存本端地址
    //参数说明:无
    //返回值:
    //成功:0,失败-1,具体错误可以通过GetLastError()获取
    *******************************************/
    int SaveSockAddr();

    /*****************************************
    //功能:保存两端地址
    //参数说明:无
    //返回值:
    //成功:0,失败-1,具体错误可以通过GetLastError()获取
    ******************************************/
    int SaveAddr();


    /*****************************************
    //功能：连接到SERVER
    //参数说明:
    //[in] szServerIP:服务器IP
    //[in] uServerPort：服务器端口
    //[in] timeout_usec: 连接超时的毫秒数(1秒=1000毫秒）
                -1表阻塞式连接
                0:不可连接时立即返回，不等待
                 >0：不可连接时等待至超时

    //返回值：	
    //0:成功,-1：失败，具体错误可以通过GetLastError()获取
    **********************************************/
    int Connect(const char *szServerIP, unsigned short iServerPort, int timeout_usec = -1);



    /**********************************************
    //功能：关闭SOCKET描述符
    //参数说明:	无
    //返回值：	
    //	0:成功,-1：失败，具体错误可以通过GetLastError()获取
    *************************************************/
    int Close();

    /**********************************************
    //功能：关闭SOCKET描述符
    //参数说明:无	
    //返回值：	
    //0:成功
    //-1：失败，具体错误可以通过GetLastError()获取
    *************************************************/
    int Shutdown(int how = SHUT_RDWR);

    /**********************************************
    //功能：接收数据
    //参数说明:
    //[out] chBuffer:接收缓冲区
    //[in] nSize:缓冲区的长度
    //[in] timeout: 超时的毫秒数(1秒=1000毫秒）
            -1表阻塞式接收
            0:不可收时立即返回，不等待
            >0：不可收时等待至超时
    //返回值：	
        -1：失败，具体错误可以通过GetLastError()获取
        0:对方已关闭
        >0:收到的数据长度
    *************************************************/
    int Recv(char *chBuffer, unsigned int nSize, int timeout = -1, int flags = 0);


    /**********************************************
    //功能：发送数据
    //参数说明:
    //[in] chBuff：待发送的数据
    //[in] nSize:缓冲区的长度
    //[in] timeout: 超时的毫秒数(1秒=1000毫秒）
        -1表阻塞式接收
        0:不可发时立即返回，不等待
        >0：不可发时等待至超时
    //[in] flag:发送标志	
    //返回值：	
        -1：失败，具体错误可以通过GetLastError()获取
        >=0:已发送的数据长度
    *************************************************/
    int Send(const void* chBuff, unsigned int nSize, int timeout = -1, int flags = 0);

    /**********************************************
    //功能：收数据(一直收满nBytes字节为止，或者出错为止，或者超时为止）
    //参数说明:
    //[out] nread:返回已收到的字节数
    //[out] chBuffer:缓冲区中包含已收到的数据
    //[out] nBytes:缓冲区的长度
    //[in] timeout: 超时的毫秒数(1秒=1000毫秒）
            -1表阻塞式接收
            0:不可收时立即返回，不等待
            >0：不可收时等待至超时
    //[in] flags:标志
    //返回值：	
        -1：失败，具体错误可以通过GetLastError()获取
        0:对方已关闭
        >0:收到的数据长度
    *************************************************/
    int RecvN(unsigned int &nread, void *pBuffer, unsigned int nBytes,int timeout = -1, int flags = 0);

    int ReadN(unsigned int &nread, void *vptr, unsigned int n);

    /**********************************************
    //功能：收一个包 
    //参数说明:
    //[in]  pf:包长度检查的函数
    			(返回值:0还要继续收数据才能确定，
    				-1:数据出错
    				>0:包的全长)
    		unsigned int nBytes:缓冲区的长度		
    		int timeout: 超时的毫秒数(1秒=1000毫秒）
    		 -1表阻塞式接收
    		 0:不可收时立即返回，不等待
    		 >0：不可收时等待至超时
    		int flags:标志
    *输出参数：
    		unsigned int &nRead:返回已收到的字节数
    		char *pBuffer:缓冲区中包含已收到的数据

    返回值: >0:read ok, pack size is return, actually read size is nread, nread>=返回值
    0:closed by peer
    -1:error
    -2:timeout
    -3:data invalid
    *************************************************/
    int RecvPack(pfHandleInput pf, unsigned int &nread, void *pBuffer, unsigned int nBytes,int timeout = -1, int flags = 0);


    /*****************************************************************
    //功能：收一个包 
    //参数说明:
    //[in] pf:包长度检查的函数
    			(返回值:0还要继续收数据才能确定，
    				-1:数据出错
    				>0:包的全长)
    //[in] hlen:包头的长度
    //[out] nRead:返回已收到的字节数
    //[out] pBuffer:缓冲区中包含已收到的数据
    //[in] nBytes:缓冲区的长度		
    //[in] timeout: 超时的毫秒数(1秒=1000毫秒）
    		 -1表阻塞式接收
    		 0:不可收时立即返回，不等待
    		 >0：不可收时等待至超时
    //[in] flags:标志

    返回值: >0:read ok, pack size is return, actually read size is nread, nread>=返回值
    0:closed by peer
    -1:error
    -2:timeout
    -3:data invalid
    ***********************************************************************/
    int RecvPack(pfHandleInput pf, unsigned int hlen, unsigned int &nread, void *pBuffer, unsigned int nBytes,int timeout = -1, int flags = 0);


    /*****************************************************************
    //功能：发数据(一直发完nBytes字节为止，或者出错为止，或者超时为止）
    //输入参数：
    //[out] nwrite:返回已发送的字节数
    //[in] pBuffer:待发送的缓冲区
    //[in] nBytes:缓冲区的长度
    //[in] timeout: 超时的毫秒数(1秒=1000毫秒）
    		 -1表阻塞式接收
    		 0:不可发时立即返回，不等待
    		 >0：不可发时等待至超时
    //[in] flags:标志
    //返回值：	
    		-1：失败，具体错误可以通过GetLastError()获取
    		0:对方已关闭
    		>0:收到的数据长度
    ***********************************************************************/
    int SendN(unsigned int &nwrite, const void *pBuffer, unsigned int nBytes,int timeout = -1, int flags = 0);

    int WriteN(unsigned int &nwrite, const void *pBuffer, unsigned int nBytes);



    /*****************************************************************
    //功能：绑定本地地址
    //参数说明:
    //[in] pszBindIP ：绑定的IP
    //[in] iBindPort：绑定的端口
    //返回值：	
    		-1：失败，具体错误可以通过GetLastError()获取
    		0:成功
    ***********************************************************************/
    int Bind(const char *pszBindIP, const unsigned short iBindPort);

    /*****************************************************************
    //功能：取SOCKET选项
    //参数说明:
    //[in] level:级别
    //[in] optname:选项名称
    //[out] optval:选项值
    //[out] optlen:选项值的长度
    //返回值：	
    		-1：失败，具体错误可以通过GetLastError()获取
    		0:成功
    ***********************************************************************/
    int GetSockOpt(int level, int optname, void *optval, socklen_t *optlen);

    /*****************************************************************
    //功能：设置SOCKET选项
    //参数说明:
    //	[in]  level:级别
    //	[in] optname:选项名称
    //	[in] optval:选项值
    //	[in] optlen:选项值的长度
    //返回值：	
    		-1：失败，具体错误可以通过GetLastError()获取
    		0:成功
    ***********************************************************************/
    int SetSockOpt(int level, int optname, const void *optval, socklen_t optlen);



    /*****************************************************************
    *功能：
    		取SOCKET的本端地址
    *输入参数：
    		socklen_t *namelen:初始化为name结构体的大小
    *输出参数：
    		struct sockaddr *name：返回的本端地址
    *返回值：	
    		-1：失败，具体错误可以通过GetLastError()获取
    		0:成功
    ***********************************************************************/
    int GetSockName(struct sockaddr *name, socklen_t *namelen);


    /*****************************************************************
    *功能：
    		取本端的IP地址
    *输入参数：
    		无
    *输出参数：
    		sring &strHost：返回的本端IP地址
    *返回值：	
    		-1：失败，具体错误可以通过GetLastError()获取
    		0:成功
    ***********************************************************************/
    int GetSockIP(string &strHost)
    {
        if(SaveSockAddr() != 0)
        {
            return -1;
        }
        char buf[32] = {0};
        strHost = inet_ntoa_r(m_SockAddr.sin_addr, buf);
        return 0;
    }
    
    /***********************************************************************
    //功能:获取本端IP地址
    //参数说明:
    //[in] ip:接收IP地址的缓冲区
    //[in] len:缓冲区的长度
    //返回值:0,成功，-1失败，具体错误可以通过GetLastError()获取
    ***********************************************************************/
    int GetSockIP(char * ip, int len)
    {
        string s;

        GetSockIP(s);

        strncpy(ip, s.c_str(), len);

        return 0;
    }

    /***********************************************************************
    *功能：取本端的端 口
    *输入参数：
        无
    *输出参数：
        无
    *返回值：	
        -1：失败，具体错误可以通过GetLastError()获取
        >=0:本端的端口号
    ***********************************************************************/
    int GetSockPort()
    {
        if( 0 != SaveSockAddr())
        {
            return -1;
        }
        return ntohs(m_SockAddr.sin_port);
    }



    /***********************************************************************
    *功能：
    		取SOCKET的对端地址
    *输入参数：
    		socklen_t *namelen:初始化为name结构体的大小
    *输出参数：
    		struct sockaddr *name：返回的本端地址
    *返回值：	
    		-1：失败，具体错误可以通过GetLastError()获取
    		0:成功
    ***********************************************************************/
    int GetPeerName(struct sockaddr *name, socklen_t *namelen);

    /***********************************************************************
    *功能：
    		取对端的IP地址
    *输入参数：
    		无
    *输出参数：
    		sring &strHost：返回的对端IP地址
    *返回值：	
    		-1：失败，具体错误可以通过GetLastError()获取
    		0:成功
    ***********************************************************************/
    int GetPeerIP(string &strHost)
    {
        if( SavePeerAddr() != 0)
        {
            return -1;
        }
        char buf[32] = {0};
        strHost = inet_ntoa_r(m_PeerAddr.sin_addr, buf);
        return 0;
    }

    int GetPeerIP(char * ip, int len)
    {
        string s;

        GetPeerIP(s);

        strncpy(ip, s.c_str(), len);

        return 0;
    }

    /***********************************************************************
    *功能：
    		取对端的端 口
    *输入参数：
    		无
    *输出参数：
    		无
    *返回值：	
    		-1：失败，具体错误可以通过GetLastError()获取
    		>＝0:对端的端口号
    ***********************************************************************/
    int GetPeerPort()
    {
        if( 0 != SaveSockAddr())
        {
            return -1;
        }
        return ntohs(m_PeerAddr.sin_port);
    }

    /***********************************************************************
    *功能：
    		设置非阻塞选项
    *输入参数：
    		bool flag:是否为非阻塞
    			true:是
    			false:不是
    *输出参数：
    		无
    *返回值：	
    		-1：失败，具体错误可以通过GetLastError()获取
    		0:成功
    ***********************************************************************/
    int SetNonBlockOption(bool flag = true);


    bool SetReuseAddr(int on = 1)
    {
        return setsockopt(m_iSocket, SOL_SOCKET, SO_REUSEADDR, (void *)&on, sizeof(int)) != -1;
    }


    /***********************************************************************
    *功能：
    		等待可读
    *输入参数：
    		int timeout: 超时的毫秒数(1秒=1000毫秒）
    		 -1:一直等待
    		 0:不等待
    		 >0：最多等待timeout毫秒
    *输出参数：
    		无
    *返回值：	
    		-1：失败，具体错误可以通过GetLastError()获取
    		0:可读
    ***********************************************************************/
    int WaitRead(int timeout = -1);

    /***********************************************************************
    *功能：
    		等待可写
    *输入参数：
    		int timeout: 超时的毫秒数(1秒=1000毫秒）
    		 -1:一直等待
    		 0:不等待
    		 >0：最多等待timeout毫秒
    *输出参数：
    		无
    *返回值：	
    		-1：失败，具体错误可以通过GetLastError()获取
    		0:可写
    ***********************************************************************/
    int WaitWrite(int timeout = -1);
	
    /***********************************************************************
    *功能：
    		等待可写或可读
    *输入参数：
    		int timeout: 超时的毫秒数(1秒=1000毫秒）
    		 -1:一直等待
    		 0:不等待
    		 >0：最多等待timeout毫秒
    *输出参数：
    		无
    *返回值：	
    		-1：失败，具体错误可以通过GetLastError()获取
    		1:可写
    		2:可读
    		3:即可写，又可读
    ***********************************************************************/
    int WaitRdWr(int timeout = -1);	

    int SendRecvWithRetry(
        const char *req,
        unsigned int req_len, 
        pfHandleInput pf,
        unsigned int hlen,    		
        char *rsp, 
        unsigned int rsp_len,  
        int timeout = -1,
        int flags = 0)
    {
        int n = SendRecv(req, req_len, pf, hlen, rsp, rsp_len, timeout, flags) ;
        if( n <= 0)
        {
            if( Reconnect(timeout) != 0)
            {
                return n;
            }
            n = SendRecv(req, req_len, pf, hlen, rsp, rsp_len, timeout, flags); 
        }
        return n;
    }

    /*
    *	-1:recv error
    	-2:recv timeout
    	-3:recv invalid data
    	-4:send error
    	-5:send timeout		
    	0:closed
    	>0:ok, ret the pack len.
    */
    int SendRecv(
    	const char *req,
    		unsigned int req_len, 
    	pfHandleInput pf,
    	unsigned int hlen,
    		char *rsp, 
    		unsigned int rsp_len,
    		int timeout = -1,
    		int flags = 0);
	
	
protected:
    /*
    *功能：
        生成新的SOCKET描述符
    *输入参数：
        int nSocketType:SOCKET类型（SOCK_DGRAM:UDP,SOCK_STREAM:TCP)
    *输出参数：
    		无
    *返回值：	
            INVALID_SOCKET:失败
            非INVALID_SOCKET:生成的SOCKET描述符
    */
    int Create(int nSocketType);
    int Connect(struct sockaddr_in *addr, int timeout_usec =-1);


    /*
    *功能：
        连接到SERVER
    *输入参数：
    *   const struct sockaddr *serv_addr:服务器地址
        socklen_t addrlen：地址结构的长度
        int timeout_usec: 连接超时的毫秒数(1秒=1000毫秒）
            -1表阻塞式连接
            0:不可连接时立即返回，不等待
            >0：不可连接时等待至超时

    *输出参数：
            无
    *返回值：	
        0:成功
        -1：失败，具体错误可以通过GetLastError()获取
    */

    int  Connect(unsigned int ip, unsigned short port,int timeout_usec = -1);


    int m_iSocket;	//SOCKET描述符
    int m_iType; 	//SOCKET类型
    string m_strPeerAddr;//对端地址 
    int m_iPeerPort;//对端端口

    struct sockaddr_in m_SockAddr;	//本端地址
    struct sockaddr_in m_PeerAddr;	//对端地址

};


class CUdpSocket : public CBaseSocket
{
public:
        CUdpSocket()
        {
            CreateUdp();            
        }

        CUdpSocket(const char *host, unsigned short port, int timeout = -1)
        {
            if( INVALID_SOCKET != CreateUdp())
            {
                if(0 != Connect(host, port, timeout))
                {
                    Close();
                }
            }
        }

        int SendPack(void *pBuffer, unsigned int nBytes,int iFlags /*= 0*/)
        {
            socklen_t addr_len = sizeof(m_PeerAddr);
            int bytes = sendto(m_iSocket, pBuffer, nBytes,iFlags, (struct sockaddr *) &m_PeerAddr, addr_len );
            if(bytes < 0 )
            {
                return errno ? -errno:bytes;
            }
            else
            {
                return bytes;
            }
        }

        int RecvPack(void * pBuffer,unsigned int nBytes,int iFlags /*= 0*/)
        {            
            socklen_t addr_len = sizeof(m_PeerAddr);
            int bytes =::recvfrom(m_iSocket, pBuffer, nBytes, iFlags, (struct sockaddr *) &m_PeerAddr, &addr_len ) ;
            if(bytes < 0)
            {
                return errno ? -errno : bytes;
            }
            else
            {
                return bytes;
            }
        }
    
    /*
    *功能：
        收数据（主要用于UDP）
    *输入参数：
        void *pBuffer:接收缓冲区
        unsigned int nBytes:接收缓冲区的大小
        int iFlags:标志
    *输出参数：
        void *pBuffer:接收缓冲区
        struct sockaddr *pFromAddr:对方的地址 
        socklen_t* iAddrLen：地址的长度
    *返回值：	
        -1：失败，具体错误可以通过GetLastError()获取
        0:对方已关闭
        >0:收到的数据长度
    */
    int Recvfrom(void *pBuffer, unsigned int nBytes, struct sockaddr *pFromAddr, socklen_t* iAddrLen, int iFlags /*= 0*/)
    {
        return ::recvfrom(m_iSocket, pBuffer, nBytes,iFlags, pFromAddr, iAddrLen);
    }

    /*
    *功能：
        发数据（主要用于UDP）
    *输入参数：
        void *pBuffer：发送缓冲区
        unsigned int nBytes:发送缓冲区的大小
        int iFlags:标志
        struct sockaddr *pFromAddr:对方的地址 
        socklen_t iAddrLen：地址的长度

    *输出参数：
        无
    *返回值：	
        -1：失败，具体错误可以通过GetLastError()获取
        >=0:已发送的数据长度
    */
    int Sendto(void *pBuffer, unsigned int nBytes, struct sockaddr *pToAddr, socklen_t iAddrLen,int iFlags /*= 0*/ )
    {
        return sendto(m_iSocket, pBuffer, nBytes,iFlags, pToAddr, iAddrLen);
    }


};


class CTcpClient : public CBaseSocket
{
public:
    //client side
    CTcpClient(const char *host, unsigned short port, int timeout = -1)
    {
        if( INVALID_SOCKET != CreateTcp())
        {
            if(0 != Connect(host, port, timeout))
            {
                Close();
            }
        }

    }

    //server side
    CTcpClient(int fd) 
    {
        Attach(fd, SOCK_STREAM);
    }


};

}
}


#endif // _CSOCKET_H_


