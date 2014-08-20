

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <netdb.h>
#include <stdio.h>
#include <errno.h>

#include "base_socket.h"

namespace comm
{
namespace basesock
{

inline int DiffMillSec(const struct timeval &t1, const struct timeval &t2)
{
    return (t1.tv_sec * 1000 - t2.tv_sec * 1000) + (t1.tv_usec / 1000 - t2.tv_usec/1000); 
}

inline int DiffMillSec(const struct timeval &t)
{
    struct timeval tv;
    struct timezone tz;
    gettimeofday(&tv, &tz);
    return DiffMillSec(tv, t);
}

inline void MkTimeval(struct timeval &tv, int millisec)
{
    if(millisec > 0)
    {
        tv.tv_sec = millisec / 1000;
        tv.tv_usec = (millisec % 1000)*1000;
    }
    else
    {
        tv.tv_sec = 0;
        tv.tv_usec = 0;
    }
}

/*
*封闭SOCKET操作的基类
*/

CBaseSocket::CBaseSocket()
{
    m_iSocket = 	INVALID_SOCKET;
    m_iType = -1;
    memset(&m_PeerAddr,0,sizeof(m_PeerAddr));
    memset(&m_SockAddr,0,sizeof(m_SockAddr));
}
	





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
int CBaseSocket::Create(int nSocketType)
{
    m_iType = nSocketType;
    if( (m_iType != SOCK_STREAM) && (m_iType != SOCK_DGRAM))
    {
        return INVALID_SOCKET;
    }
    m_iSocket = socket(AF_INET, m_iType, 0);
    return m_iSocket;
}



/*
*功能：
		依附到新的SOCKET描述符
*输入参数：
		int iSocket:新的SOCKET描述符
*输出参数：
		无
*返回值：	
		无
*/
void CBaseSocket::Attach(int iSocket, int iType)
{
    if(IsValid())
    {
        Close();
    }
    m_iSocket = iSocket;	
    m_iType = iType;
    memset(&m_PeerAddr, 0, sizeof(m_PeerAddr));
    memset(&m_SockAddr, 0, sizeof(m_SockAddr));
    SaveAddr();
}

/*
*保存对端地址
*/
int CBaseSocket::SavePeerAddr()
{
    socklen_t len = sizeof(m_PeerAddr);
    int n =  getpeername(m_iSocket, (struct sockaddr *)&m_PeerAddr, &len);
    char buf[32] = {0};
    inet_ntoa_r(m_PeerAddr.sin_addr, buf);
    return n;
}

/*
*保存本端地址
*/
int CBaseSocket::SaveSockAddr()
{
    socklen_t len = sizeof(m_SockAddr);
    int n =  getsockname(m_iSocket, (struct sockaddr *)&m_SockAddr, &len);
    char buf[32] = {0};
    inet_ntoa_r(m_SockAddr.sin_addr, buf);
    return n;
}

/*
*保存两端地址
*/
int CBaseSocket::SaveAddr()
{
    SaveSockAddr();
    return SavePeerAddr();
}





/*
*功能：
		关闭SOCKET描述符
*输入参数：
		无
*输出参数：
		无
*返回值：	
		0:成功
		-1：失败，具体错误可以通过GetLastError()获取
*/
int CBaseSocket::Close()
{
    if(IsValid())
    {
        close(m_iSocket);
        m_iSocket = INVALID_SOCKET;
    }
    return 0;
}

/*
*功能：
		关闭SOCKET描述符
*输入参数：
		无
*输出参数：
		无
*返回值：	
		0:成功
		-1：失败，具体错误可以通过GetLastError()获取
*/
int CBaseSocket::Shutdown(int how /* = SHUT_RDWR */)
{
    if(IsValid())
    {
        shutdown(m_iSocket, how);
    }
    return 0;
}


/*
*功能：
		收数据
*输入参数：
		unsigned int nSize:缓冲区的长度
		int timeout: 超时的毫秒数(1秒=1000毫秒）
		 -1表阻塞式接收
		 0:不可收时立即返回，不等待
		 >0：不可收时等待至超时
*输出参数：
		char *chBuffer:缓冲区
*返回值：	
		-1：失败，具体错误可以通过GetLastError()获取
		0:对方已关闭
		>0:收到的数据长度
*/
int CBaseSocket::Recv(char *chBuffer, unsigned int nSize, int timeout /*= -1*/, int flags /*= 0*/)
{
    if(timeout < 0)
    {
        return recv(m_iSocket, chBuffer, nSize, flags);
    }
    int n = WaitRead( timeout );
    if(n != 0)
    {
        return n;
    }
    return recv(m_iSocket, chBuffer, nSize, flags);
}


/*
*功能：
		发数据
*输入参数：
		const void *chBuff：待发送的数据
		unsigned int nSize:缓冲区的长度
		int timeout: 超时的毫秒数(1秒=1000毫秒）
		 -1表阻塞式接收
		 0:不可发时立即返回，不等待
		 >0：不可发时等待至超时
		int flag:发送标志
*输出参数：
		无
*返回值：	
		-1：失败，具体错误可以通过GetLastError()获取
		>=0:已发送的数据长度
*/
int CBaseSocket::Send(const void* chBuff, unsigned int nSize, int timeout /*= -1*/, int flags /*= 0*/)
{
    if(timeout < 0)
    {
        return send(m_iSocket, chBuff, nSize, flags);
    }
    int n = WaitWrite( timeout );
    if(n != 0)
    {
        return n;
    }
    return send(m_iSocket, chBuff, nSize, flags);
}


/*
*功能：
		绑定本地地址
*输入参数：
		const char *pszBindIP, const ：绑定的IP
		unsigned short iBindPort	：绑定的端口
*输出参数：
		无
*返回值：	
		-1：失败，具体错误可以通过GetLastError()获取
		0:成功
*/
int CBaseSocket::Bind(const char *pszBindIP, const unsigned short iBindPort)
{
    struct sockaddr_in inaddr;	
    bzero (&inaddr, sizeof (struct sockaddr_in));
    inaddr.sin_family = AF_INET;		//ipv4协议族	
    inaddr.sin_port = htons (iBindPort);	
    if(pszBindIP== 0 || pszBindIP[0] == 0 || strcmp(pszBindIP, "0.0.0.0") == 0)
    {
        inaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    }
    else
    {
        if (inet_pton(AF_INET, pszBindIP, &inaddr.sin_addr) <= 0) //点分十进制ip地址转为二进制地址	
        {		
            return -1;	
        }
    }
    int n = bind(m_iSocket, (struct sockaddr*)&inaddr, sizeof(struct sockaddr));
    if( n == 0)
    {
        SaveSockAddr();
    }
    return n;
}


/*
*功能：
		取SOCKET选项
*输入参数：
		int level:级别
		int optname:选项名称
*输出参数：
		void *optval:选项值
		socklen_t *optlen:选项值的长度
*返回值：	
		-1：失败，具体错误可以通过GetLastError()获取
		0:成功
*/
int CBaseSocket::GetSockOpt(int level, int optname, void *optval, socklen_t *optlen)
{
    return getsockopt(m_iSocket, level, optname, optval, optlen);
}
/*
*功能：
		设置SOCKET选项
*输入参数：
		int level:级别
		int optname:选项名称
		void *optval:选项值
		socklen_t *optlen:选项值的长度

*输出参数：
		无
*返回值：	
		-1：失败，具体错误可以通过GetLastError()获取
		0:成功
*/
int CBaseSocket::SetSockOpt(int level, int optname, const void *optval, socklen_t optlen)
{
    return setsockopt(m_iSocket, level, optname, optval, optlen);
}



/*
*功能：
		取SOCKET的本端地址
*输入参数：
		socklen_t *namelen:初始化为name结构体的大小
*输出参数：
		struct sockaddr *name：返回的本端地址
*返回值：	
		-1：失败，具体错误可以通过GetLastError()获取
		0:成功
*/
int CBaseSocket::GetSockName(struct sockaddr *name, socklen_t *namelen)
{
    if(0 != SaveSockAddr())
    {
        return -1;
    }
    memcpy(name, &m_SockAddr, sizeof(m_SockAddr));
    *namelen = sizeof(m_SockAddr); 
    return 0;
}



/*
*功能：
		取SOCKET的对端地址
*输入参数：
		socklen_t *namelen:初始化为name结构体的大小
*输出参数：
		struct sockaddr *name：返回的本端地址
*返回值：	
		-1：失败，具体错误可以通过GetLastError()获取
		0:成功
*/
int CBaseSocket::GetPeerName(struct sockaddr *name, socklen_t *namelen)
{
    if(0 != SavePeerAddr())
    {
        return -1;
    }
    memcpy(name, &m_PeerAddr, sizeof(m_PeerAddr));
    *namelen = sizeof(m_PeerAddr); 
    return 0;

}

/*
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
*/
int CBaseSocket::SetNonBlockOption(bool flag /*= true*/)
{
    int save_mode;
    save_mode = fcntl( m_iSocket, F_GETFL, 0 );
    if (flag)
    { // set nonblock
        save_mode |= O_NONBLOCK;
    }
    else 
    { // set block
        save_mode &= (~O_NONBLOCK);
    }
    
    fcntl( m_iSocket, F_SETFL, save_mode );
    return 0;
}


int CBaseSocket::Reconnect(int timeout /*=-1*/)
{
    Close();
    if(m_iType == SOCK_STREAM)
    {
        CreateTcp();
    }
    else
    {
        CreateUdp();
    }
    if(!IsValid())
    {
        return -1;
    }

    if( 0 != Connect(&m_PeerAddr, timeout))
    {
        //Close();
        return -1;
    }
    return 0;
}






/*
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
            -2:timeout
		-1：失败，具体错误可以通过GetLastError()获取
		0:可读
*/
int CBaseSocket::WaitRead(int timeout /*= -1*/)
{
    if(m_iSocket < 0)
    {
        return -1;
    }
    struct timeval tv;
    struct timeval*ptv = &tv;
    if(timeout < 0)
    {
        ptv = NULL;
    }
    else
    {
        MkTimeval(tv, timeout);
    }
    
    fd_set recv_fds;
    int iNum= 0;

    FD_ZERO( &recv_fds );
    FD_SET( m_iSocket, &recv_fds );

    iNum= select( m_iSocket+1, &recv_fds, NULL, NULL, ptv );
    if(iNum == 1)
    {
        return 0;
    }
    if( iNum == 0)
    {
        return -2;
    }
    return -1;
}


/*
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
            -2:timeout
		-1：失败，具体错误可以通过GetLastError()获取
		0:可写
*/
int CBaseSocket::WaitWrite(int timeout /*= -1*/)
{
    if(m_iSocket < 0)
    {
        return -1;
    }
    struct timeval tv;
    struct timeval*ptv = &tv;
    if(timeout < 0)
    {
        ptv = NULL;
    }
    else
    {
        MkTimeval(tv, timeout);
    }

    fd_set wfds;
    int iNum= 0;

    FD_ZERO( &wfds );
    FD_SET( m_iSocket, &wfds );
    iNum= select( m_iSocket+1, NULL, &wfds, NULL, ptv );
    if(iNum == 1)
    {
        return 0;
    }
    
    if( iNum == 0)
    {
        return -2;
    }
    return -1;
}



/*
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
            -2:timeout
		-1：失败，具体错误可以通过GetLastError()获取
		1:可写
		2:可读
		3:即可写，又可读
*/
int CBaseSocket::WaitRdWr(int timeout /*= -1*/)
{
    if(m_iSocket < 0)
    {
        return -1;
    }
    struct timeval tv;
    struct timeval *ptv = &tv;
    if(timeout < 0)
    {
        ptv = NULL;
    }
    else
    {
        MkTimeval(tv, timeout);
    }

    fd_set recv_fds;
    int iNum= 0;
    fd_set wfds;

    FD_ZERO( &recv_fds );
    FD_SET( m_iSocket, &recv_fds );
    FD_ZERO( &wfds );
    FD_SET( m_iSocket, &wfds );

    iNum= select( m_iSocket+1, &recv_fds, &wfds, NULL, ptv );
    if(iNum == 1)
    {
        if(FD_ISSET(m_iSocket, &recv_fds))
        {
            return 2;
        }
        return 1;
    }
    
    if(iNum == 2)
    {
        return 3;
    }
    
    if(iNum == 0)
    {
        return -2;
    }
    return -1;
}





/*
*功能：
		发数据(一直发完nBytes字节为止，或者出错为止，或者超时为止）
*输入参数：
		const void *pBuffer:待发送的缓冲区
		unsigned int &nBytes:缓冲区的长度
		int timeout: 超时的毫秒数(1秒=1000毫秒）
		 -1表阻塞式接收
		 0:不可发时立即返回，不等待
		 >0：不可发时等待至超时
		int flags:标志
*输出参数：
		unsigned int &nBytes:返回已发送的字节数
*返回值：
            -2:timeout
		-1：失败，具体错误可以通过GetLastError()获取
		0:对方已关闭
		>0:收到的数据长度
*/
int CBaseSocket::SendN(unsigned int &nwrite, const void *pBuffer, unsigned int nBytes,int timeout /*= -1*/, int flags /*= 0*/)
{
    if(m_iSocket < 0)
    {
        return -1;
    }
    nwrite = 0;
    if(timeout < 0)
    {
        return WriteN(nwrite, pBuffer, nBytes);
    }

    struct timeval start;
    struct timezone tz;
    gettimeofday(&start, &tz);
    unsigned int left = nBytes;
    int n = 0;
    char *ptr = (char *)pBuffer;
    while(1)
    {
        n = WaitWrite(timeout);
        if(n < 0)	//error
        {
            return n;
        }
        n = send(m_iSocket, ptr, left, flags);
        if( n < 0)
        {
            if(errno == EINTR)
            {
                continue;
            }
            return n;
        }
        
        if(n == 0)
        {
            return 0;
        }
        
        left -= n;
        ptr += n;
        nwrite += n;
        if(left <= 0)
        {
            break;
        }

        timeout -= DiffMillSec(start);
        if(timeout <= 0)	//timeout
        {
            return -2;
        }
    }
    return nwrite;
}


int CBaseSocket::WriteN(unsigned int &nwrite, const void *pBuffer, unsigned int n)
{
    if(m_iSocket < 0)
    {
        return -1;
    }
    size_t	nleft;
    char	*ptr;

    ptr = (char *)pBuffer;
    nleft = n;
    int ret = 0;
    nwrite = 0;
    while (nleft > 0) 
    {
        if ( (ret = write(m_iSocket, ptr, nleft)) < 0) 
        {
            if (errno == EINTR)
                ret = 0;		/* and call read() again */
            else
                return(-1);
        } 
        else if (ret == 0)
        {
            return 0;				/* EOF */
        }
        
        nleft -= ret;
        ptr   += ret;
        nwrite += ret;
    }
    return nwrite;	
}


/*
*功能：
		收数据(一直收满nBytes字节为止，或者出错为止，或者超时为止）
*输入参数：
		unsigned int &nBytes:缓冲区的长度
		int timeout: 超时的毫秒数(1秒=1000毫秒）
		 -1表阻塞式接收
		 0:不可收时立即返回，不等待
		 >0：不可收时等待至超时
		int flags:标志
*输出参数：
		char *chBuffer:缓冲区中包含已收到的数据
*返回值：	
		-2:timeout
		-1：失败，具体错误可以通过GetLastError()获取
		0:对方已关闭
		>0:收完了所有数据
*/
int CBaseSocket::RecvN(unsigned int &nread, void *pBuffer, unsigned int nBytes,int timeout /*= -1*/, int flags /*= 0*/)
{
    if(m_iSocket < 0)
    {
        return -1;
    }
    nread = 0;
    if( timeout < 0)
    {
        return ReadN(nread, pBuffer, nBytes);		
    }
    struct timeval start;
    struct timezone tz;
    gettimeofday(&start, &tz);
    unsigned int left = nBytes;
    int n = 0;
    char *ptr = (char *)pBuffer;
    while(1)
    {
        n = WaitRead(timeout);
        if(n < 0)	//error
        {
            //if(errno == ETIMEO)
            return n;	//timeout
        }

        n = recv(m_iSocket, ptr, left, flags);
        if( n < 0)
        {
            if(errno == EINTR)
            {
                continue;
            }
            return -1;		//error
        }
        
        if(n == 0)
        {
            return 0;		//closed by peer
        }
        
        left -= n;
        ptr += n;
        nread += n;
        if(left <= 0)
        {
            break;	//ok
        }

        timeout -= DiffMillSec(start);
        if(timeout <= 0)	//timeout
        {
            return -2;
        }
    }
    
    if(nread == nBytes)
    {
        return nread;
    }
    return -2;	//timeout
}

int CBaseSocket::ReadN(unsigned int &nread,void *vptr, unsigned int n)
{
    if(m_iSocket < 0)
    {
        return -1;
    }
    size_t	nleft;
    char	*ptr;

    ptr = (char *)vptr;
    nleft = n;
    int ret = 0;
    nread = 0;
    while (nleft > 0) 
    {
        if ( (ret = read(m_iSocket, ptr, nleft)) < 0) 
        {
            if (errno == EINTR)
                ret = 0;		/* and call read() again */
            else
                return(-1);
        } 
        else if (ret == 0)
        {
            return 0;				/* EOF */
        }
        nleft -= ret;
        ptr   += ret;
        nread += ret;
    }
    return nread;	
}



/*
*功能：
		收一个包 
*输入参数：
		pfHandleInput pf:包长度检查的函数
			(返回值:0还要继续收数据才能确定，
				-1:数据出错
				>0:包的全长)
		unsigned int hlen:包头的长度
		unsigned int nBytes:缓冲区的长度		
		int timeout: 超时的毫秒数(1秒=1000毫秒）
		 -1表阻塞式接收
		 0:不可收时立即返回，不等待
		 >0：不可收时等待至超时
		int flags:标志
*输出参数：
		unsigned int &nRead:返回已收到的字节数
		char *pBuffer:缓冲区中包含已收到的数据

返回值: >0:read ok, pack size is return, actually read size is nread, nread=返回值
0:closed by peer
-1:error
-2:timeout
-3:data invalid
*/
int CBaseSocket::RecvPack(pfHandleInput pf, unsigned int hlen, unsigned int &nread, void *pBuffer, unsigned int nBytes,int timeout , int flags )
{
    nread = 0;
    struct timeval start;
    struct timezone tz;
    if(timeout > 0)
    {
        gettimeofday(&start, &tz);
    }

    char *ptr = (char *)pBuffer;
    int ret = RecvN(nread, ptr, hlen, timeout, flags);
    if(nread != hlen)
    {
	return ret;
    }

    if(timeout > 0)
    {
	timeout -= DiffMillSec(start);
	if(timeout < 0)
	{
		return -2;	//timeout
	}
    }

   int pack_len = pf(ptr, hlen);
   if(pack_len <= 0)
   {
   	return -3;	//data error.
   }

    if((unsigned int )pack_len == hlen)
    {
        return pack_len;	//包体为空
    }

    if((unsigned int)pack_len > nBytes)
    {
        return -4;	//too big.
    }
	
    unsigned int nbody = 0;
    ret = RecvN(nbody, ptr + hlen, pack_len - hlen, timeout, flags); 
    nread += nbody;
    if(nread == (unsigned int)pack_len)
    {
	return nread;
    }
    return ret;
}


int CBaseSocket::Connect(struct sockaddr_in *addr, int timeout_usec /*=-1*/)
{
    char buf[32] = {0};
    inet_ntoa_r(addr->sin_addr, buf);
    socklen_t addr_len = sizeof(struct sockaddr_in);
    int retval = 0;
    if (timeout_usec < 0) // 阻塞
    {
        int n = connect(m_iSocket, (struct sockaddr *)addr, addr_len);
        if(n != 0)
        {
            perror("connect:");
            return -1;
        }

        //...........
        SaveAddr();
        return 0;
    }
    SetNonBlockOption(true);
    retval= connect(m_iSocket, (struct sockaddr *) addr, addr_len);
    if(retval < 0 && errno != EINPROGRESS)
    {
        Close();
        return -1;
    }


    if(retval == 0)
    {
    	SaveAddr();
        SetNonBlockOption(false);
        return 0;
    }

    if(timeout_usec == 0)   //timeout
    {
    	Close();
        return -1;
    }
    fd_set rset, wset;
    FD_ZERO(&rset);
    FD_SET(m_iSocket, &rset);
    FD_ZERO(&wset);
    FD_SET(m_iSocket, &wset);
    struct timeval timeout;
    MkTimeval(timeout, timeout_usec);
    int n = select(m_iSocket + 1, &rset, &wset, NULL, &timeout);
    if( n == 0) //timeout
    {
    	Close();
        return -1;
    }

    int error = 0;
    if(FD_ISSET(m_iSocket, &rset) || FD_ISSET(m_iSocket, &wset))
    {
        socklen_t len = sizeof(error);
        if(getsockopt(m_iSocket, SOL_SOCKET, SO_ERROR, &error, &len) < 0)
        {
            Close();
            return -1;
        }

        SetNonBlockOption(false);

        if(error)
        {
        	    Close();
            return -1;
        }
        SaveAddr();
        return 0;
    }
        //not ready...
    Close();
    return -1;
}

/*
*功能：
		连接到SERVER
*输入参数：
*		const char *szServerIP:服务器IP
		unsigned short uServerPort：服务器端口
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
int CBaseSocket::Connect(const char *pszHost, unsigned short nPort, int timeout_usec /*= -1*/)
{
    sockaddr_in	addr = { 0 };
    addr.sin_family      = AF_INET;
    addr.sin_addr.s_addr = inet_addr(pszHost);
    addr.sin_port        = htons((ushort)nPort);
    // Host name isn't an IP Address?
    if (addr.sin_addr.s_addr == INADDR_NONE)
    {
        // Resolve host name.
        hostent* pHost = gethostbyname(pszHost);

        if (pHost == NULL)
        {
            return -1;
        }

        memcpy(&addr.sin_addr, pHost->h_addr_list[0], pHost->h_length);
    }

    return Connect(&addr, timeout_usec);
}


/*
*功能：
		连接到SERVER
*输入参数：
*		const char uServerIP:服务器IP(网络字节序)
		unsigned short uServerPort：服务器端口(网络字节序)
		int timeout_usec: 连接超时的毫秒数(1秒=1000毫秒）
			   -1表阻塞式连接
			   0:不可连接时立即返回，不等待
			   >0：不可连接时等待至超时

*输出参数：
		无
*返回值：	
		0:成功
		-1:error, -2:timeout：失败，具体错误可以通过GetLastError()获取
*/
int CBaseSocket::Connect(unsigned int uServerIP, unsigned short iServerPort, int timeout_usec /*= -1*/)
{
    struct sockaddr_in addr;
    addr.sin_family      = AF_INET;
    addr.sin_addr.s_addr = uServerIP;
    addr.sin_port = htons(iServerPort);
    return Connect(&addr, timeout_usec);
}




/*
*	-1:recv error
	-2:recv timeout
	-3:recv invalid data
	-4:send error
	-5:send timeout		
	0:closed
	>0:ok,the pack len is returned.
*/
int CBaseSocket::SendRecv(
		const char *req,
		unsigned int req_len, 
		pfHandleInput pf,
		unsigned int hlen,    		
		char *rsp, 
		unsigned int rsp_len,  
		int timeout,
		int flags)
{
    unsigned int nwrite = 0;
    int ret = SendN(nwrite, req, req_len, timeout);

    if(nwrite != req_len)
    {
        if(ret == 0)
        {
            return 0;
        }
        
        if(ret == -2)
        {
            return -5;
        }
        return -4;
    }
	
    return RecvPack(pf, hlen, rsp_len, rsp, rsp_len, timeout);
}



//example:
#ifdef _TEST_SOCKET_

int HandleInput(const char *buf, unsigned int len)
{
    return *buf;
}

void Client(const char *host, unsigned int port)
{
	CTcpClient client(host, port);
	if(client.IsValid())
	{
		dbg_out("client is ok,connect server ok.\n");
	}
	else
	{
		dbg_out("connect server error.\n");
		return;
	}

	int ret = 0;
	unsigned int req_len = 6;
	char req[32] = {0};
	req[0] = req_len;
	strcpy(req + 1,"12345");
	char rsp[32];
	unsigned int rsp_len = 31;
	while(1)
	{
		ret = client.SendRecvWithRetry(req, req_len, HandleInput, 
			1, rsp, rsp_len);
		if(ret <= 0)
		{
			printf("send recv error,ret = %d\n",ret);
			break;
		}
		rsp[ret] = 0;
		printf("recv packet, len = %d, content = %s\n", rsp[0], rsp + 1);
		sleep(10);
	}
}

int main(int argc, char **argv)
{
	if(argc < 3)
	{
		printf("usage: %s host port\n", argv[0]);
		return 0;
	}
	signal(SIGPIPE, SIG_IGN);

	Client(argv[1], atoi(argv[2]));
	
	return 0;
}

#endif


}
}

