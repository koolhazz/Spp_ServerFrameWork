#include <assert.h>
#include <errno.h>
#include "connset.h"
#include "socket.h"
#include <stdio.h>
#include <unistd.h>
#include "misc.h"

using namespace comm::sockcommu;
using namespace comm::base;

static const unsigned C_READ_BUFFER_SIZE = 64*1024;
static char RecvBuffer[C_READ_BUFFER_SIZE];

CConnSet::CConnSet(CMemPool& mp, int maxconn):maxconn_(maxconn),curconn_(0)
{
    ccs_ = new ConnCache*[maxconn_];
    for(int i = 0; i < maxconn_; ++i)
    {
        ccs_[i] = new ConnCache(mp); 
    }    
}
CConnSet::~CConnSet()
{
    ConnCache* cc = NULL;
    for(int i = 0; i < maxconn_; ++i)
    {
        cc = ccs_[i];
        if(cc->_fd > 0 && cc->_type != UDP_SOCKET)
            close(cc->_fd);

        cc->_r.skip(cc->_r.data_len());
        cc->_w.skip(cc->_w.data_len());

        delete cc;
    }
    delete [] ccs_;
}
int CConnSet::addconn(unsigned& flow, int fd, int type)
{
    ConnCache* cc = ccs_[flow % maxconn_];

    //当前flow不可用，寻找下一个可用的flow
    if(cc->_flow > 0)
    {
        unsigned endflow = flow + maxconn_;
        while(++flow != endflow) //就算endflow或者flow溢出也没关系
        {
            cc = ccs_[flow % maxconn_];
            if(cc->_flow == 0)
            {
                break;
            }    
        }

        if(flow == endflow)
        {
            return -1;
        }    
    }

    cc->_flow = flow;
    cc->_fd = fd;
    cc->_access = get_timebytask(NULL);
    cc->_type = type;

    ++curconn_;//连接数加1

    return 0;
}
int CConnSet::fd(unsigned flow)
{
    ConnCache* cc = ccs_[flow % maxconn_];
    if(flow && cc->_flow == flow)
    {    
        return cc->_fd;
    }    
    else
    {
        return -1;
    }
}
//  ret
// -E_NOT_FINDFD
// -E_NEED_CLOSE
// -EAGAIN
// recvd_len > 0, recv data length
int CConnSet::recv(unsigned flow)
{
    //
    //	first, find the fd
    //
    ConnCache* cc = ccs_[flow % maxconn_];
    if(cc->_flow != flow || flow == 0)
    {
#ifdef OPEN_PRINT
        printf("recv but not found, %d,%d\n", flow, cc->_flow);
#endif	
        return -E_NOT_FINDFD;
    }
    
    cc->_access = get_timebytask(NULL);
    
    //
    //	second, read from socket
    //
    int fd = cc->_fd;
    unsigned recvd_len = 0;
    int ret = 0;
    if(cc->_type != UDP_SOCKET)
    {    
        ret = CSocket::receive(fd, RecvBuffer, C_READ_BUFFER_SIZE, recvd_len);
        //
        //	third, on data received
        //
        if (ret == 0)
        {
            if (recvd_len > 0)
            {
                cc->_r.append(RecvBuffer, recvd_len);
                return recvd_len;
            }
            else	//	recvd_len == 0
            {
            #ifdef OPEN_PRINT
                printf("%s,recv_len == 0\n",__FUNCTION__);
            #endif

                return -E_NEED_CLOSE;            
            }
        }

        //
        //	fourth, on error occur
        //
        else //	ret < 0
        {
            if (ret != -EAGAIN)
            {
#ifdef OPEN_PRINT
            printf("%s,recv_len == 0\n",__FUNCTION__);
#endif
                return -E_NEED_CLOSE;
            }
            
            return -EAGAIN;
        }
    }
    else	            
    {
        ret = CSocket::receive(fd, RecvBuffer, C_READ_BUFFER_SIZE, recvd_len, cc->_addr);    
        if(!ret && recvd_len > 0)
        {
            cc->_r.append(RecvBuffer, recvd_len);
            return recvd_len;
        }
        else
        {
            return -E_NEED_CLOSE;
        } 
    }
}
// ret
// 0 new data not send, only send cache data
// -E_NEED_CLOSE
// send_len >= 0, if send_len > 0 send new data length, send_len = 0 new data not send
// -E_NOT_FINDFD
int CConnSet::send(unsigned flow, const char* data, size_t data_len)
{
    //
    //  first, find the fd
    //
    ConnCache* cc = ccs_[flow % maxconn_];
    if(cc->_flow != flow || flow == 0)
    {
#ifdef OPEN_PRINT	    
        printf("send but not found, %d,%d\n", flow, cc->_flow);
#endif	    
        return -E_NOT_FINDFD;
    }    
    cc->_access = get_timebytask(NULL);

    //
    //	second, if data in cache, send it first
    //
    int fd = cc->_fd;
    unsigned sent_len = 0;
    int ret = 0;
    if (cc->_w.data_len() != 0)//继续发送上次没有发送完毕的内容
    {
        if(cc->_type != UDP_SOCKET)
        {
            ret = CSocket::send(fd, cc->_w.data(), cc->_w.data_len(), sent_len);
        }
        else
        {
            ret = CSocket::send(fd, cc->_w.data(), cc->_w.data_len(), sent_len, cc->_addr);
        }    
        //
        //	third, if cache not sent all, append data into w cache, return
        //
        if (ret == -EAGAIN ||ret == -EINPROGRESS ||  ret == -EINTR  || (ret == 0 && sent_len < cc->_w.data_len()))
        {
            cc->_w.skip(sent_len);//设置_data_head为新值，下次继续发送
            cc->_w.append(data, data_len);
            return 0;    //	nothing sent
        }
        else if (ret < 0)
        {
            return -E_NEED_CLOSE;
        }
    }

    //
    //	fourth, if cache sent all, send new data
    //
    cc->_w.skip(cc->_w.data_len());
    sent_len = 0;
    if(cc->_type != UDP_SOCKET)
    {
        ret = CSocket::send(fd, data, data_len, sent_len);
    }
    else
    {
        ret = CSocket::send(fd, data, data_len, sent_len, cc->_addr);
    }

    if (ret <0 &&  ret != -EAGAIN && ret != -EINPROGRESS && ret != -EINTR )
    {
        return -E_NEED_CLOSE;
    }

    //
    //	fifth, if new data still remain, append into w cache, return
    //
    if (sent_len < data_len )
    {
        cc->_w.append(data + sent_len, data_len - sent_len);
    }

    return sent_len;
}
// ret
// -E_NOT_FINDFD
// 0, send complete
// send_len > 0, send continue
// C_NEED_SEND > 0, send continue
// -E_NEED_CLOSE
int CConnSet::sendfromcache(unsigned flow)
{
    //
    //  first, find the fd
    //
    ConnCache* cc = ccs_[flow % maxconn_];
    if(cc->_flow != flow || flow == 0)
    {
        return -E_NOT_FINDFD;
    }
    cc->_access = get_timebytask(NULL);

    //no cache data to send, send completely
    if (cc->_w.data_len() == 0)
    {
        return 0;
    }
    //
    //	second, if data in cache, send it first
    //
    int fd = cc->_fd;
    unsigned sent_len = 0;
    int ret = 0;
    if(cc->_type != UDP_SOCKET)
    {
        ret = CSocket::send(fd, cc->_w.data(), cc->_w.data_len(), sent_len);
    }
    else
    {
        ret = CSocket::send(fd, cc->_w.data(), cc->_w.data_len(), sent_len, cc->_addr);
    }
    //
    //	third, if cache not sent all, append data into w cache, return
    //
    if(ret == 0)
    {
        if(sent_len == 0)
        {
            return C_NEED_SEND;
        }
        else
        {
            if(sent_len == cc->_w.data_len())    //send completely
            {
                cc->_w.skip(sent_len);
                return 0;
            }
            else
            {
                cc->_w.skip(sent_len);
                return sent_len;        //send partly	
            }
        }
    }
    else if (ret == -EAGAIN)
    {
        return C_NEED_SEND;
    }
    else
    {
        return -E_NEED_CLOSE;
    }
}
int CConnSet::closeconn(unsigned flow)
{
    ConnCache* cc = ccs_[flow % maxconn_];
    if(cc->_flow == flow && flow)
    {
        cc->_flow = 0;
        if(cc->_type != UDP_SOCKET)
        {
            close(cc->_fd);
        }
        cc->_access = 0;
        cc->_type = 0;
        cc->_r.skip(cc->_r.data_len());
        cc->_w.skip(cc->_w.data_len());

        --curconn_;//连接数减1

#ifdef OPEN_PRINT	    
        printf("close %d\n", flow);
#endif
        return 0;
    }
    else
    {
#ifdef OPEN_PRINT
        printf("close but not found, %d\n", flow);
#endif
        return -E_NOT_FINDFD;
    }
}
void CConnSet::check_expire(time_t access_deadline, std::list<unsigned>& timeout_flow)
{
    timeout_flow.clear();
    unsigned flow;
    for(int i = 0; i < maxconn_; ++i)
    {
        if(((flow = ccs_[i]->_flow) > 0) && (ccs_[i]->_access < access_deadline))
        {
            timeout_flow.push_back(flow);
        }
    }
}
int CConnSet::canclose(unsigned flow)
{
    ConnCache* cc = ccs_[flow % maxconn_];
    if(cc->_flow == 0)
    {    
        return 1;
    }    
    else if(cc->_flow == flow)
    {    
        if(cc->_w.data_len() > 0) 
        {
        	    return 0;
        }    
        else
        {
        	    return 1;
        }    
    }
    else
    {
        return 0;
    }        
}

