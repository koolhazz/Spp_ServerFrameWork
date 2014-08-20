#include <stdio.h>
#include "sockcommu.h"
#include "socket.h"
#include "connset.h"
#include "epollflow.h"
#include "mempool.h"
#include "misc.h"
#include "likelydef.h"
#include "commumng.h"

#ifdef LOAD_CHECK_ENABLE
#define LOAD_DETECT_CMD_LEN		(2*sizeof(int))
#define LOAD_DETECT_CMD_1		0x11223344
#define LOAD_DETECT_CMD_2		0x55667788
#endif

static char EXT_SBUFF[MAX_BLOB_EXTDATA_LEN];

using namespace comm::base;
using namespace comm::sockcommu;
using namespace comm::commu;

/////////////////TRouteTable////////////////////////////////////
int tagTRouteTable::GetNodeInfo(TNodeInfo & info,unsigned int node_id)
{
    NodeIter itr = map_node_.find(node_id);
    if(itr != map_node_.end() )
    {
        info = itr->second;
        return 0;
    }
    return -1;	
}

unsigned int tagTRouteTable::GetNodeID(unsigned int node_key)
{
    NodeIter itr = map_node_.begin(),last = map_node_.end();
    for(;itr != last;++itr)
    {
        if(itr->second.begin_ <= node_key && node_key <= itr->second.end_ )
        {
            return itr->second.node_id_;
        }
    }

    return 0;
}

void tagTRouteTable::Show()
{
#ifdef OPEN_PRINT
    printf("serv_type=%u,version=%s, route_str=%s,route_val=%d\n",serv_type_,version_.c_str(),route_.c_str(),route_val_);
    NodeIter itr = map_node_.begin();
    for(;itr != map_node_.end();++itr)
    {
        itr->second.Show();
    }
#endif
}

/////////////////
int tagTRouteConfig::GetNodeInfo(TNodeInfo & node,unsigned int serv_type,unsigned int node_id)
{
    TRouteTable tbl;
    if( 0 == this->GetRouteTable(tbl,serv_type) )
    {
        return tbl.GetNodeInfo(node,node_id);
    }
    return -1; 
}

int tagTRouteConfig::GetRouteTable(TRouteTable & tbl,unsigned int serv_type)
{
    RouteTableIter itr = map_route_table_.find(serv_type);
    if(itr != map_route_table_.end() )
    {
        tbl = itr->second;
        return 0;
    }

    return -1; 
}

/////////////////////////////////////////////////////////////

struct CTSockCommu::TInternal
{
    CMemPool* mempool_;
    CConnSet* connset_;
    CEPollFlow* epollflow_;
};
CTSockCommu::CTSockCommu():maxconn_(0), expiretime_(0), check_expire_interval_(60),lastchecktime_(0), maxbindfd_(0), flow_(0)
{
    buff_blob_.len = 0;
    buff_blob_.data = NULL; 
    buff_blob_.owner = NULL; 
    buff_blob_.ext_type = EXT_TYPE_NONE;
    buff_blob_.ext_len = 0;
    buff_blob_.extdata = EXT_SBUFF;
    memset(&extinfo_, 0x0, sizeof(TConnExtInfo));

    ix_ = new TInternal;
    ix_->mempool_ = NULL; 
    ix_->epollflow_ = NULL; 
    ix_->connset_ = NULL; 
}
CTSockCommu::~CTSockCommu()
{
    fini();
    if(ix_ != NULL)
        delete ix_;
}
void CTSockCommu::fini()
{
    if(ix_)
    {	
        if(ix_->connset_)
        {
            delete ix_->connset_;
            ix_->connset_ = NULL;
        }
        
        if(ix_->mempool_)
        {
            delete ix_->mempool_;
            ix_->mempool_ = NULL;
        }
        
        if(ix_->epollflow_)
        {
            delete ix_->epollflow_;
            ix_->epollflow_ = NULL;
        }
    }


    for(int i = 0; i <= maxbindfd_; ++i)
    {
        if(sockbind_[i].type_ != 0)
            close(i);
    }

}
void CTSockCommu::check_expire()
{
    //0表示不检查超时
    if(check_expire_interval_ == 0)
        return ;

    //check_expire_interval_ 秒检查一次
    int now = get_timebytask(NULL);
    if(unlikely(now - lastchecktime_ > check_expire_interval_))  
    {
        list<unsigned> timeout_list;
        ix_->connset_->check_expire(now - expiretime_, timeout_list);
        //有超时回调
        if(unlikely(func_list_[CB_TIMEOUT] != NULL))
        {	
            buff_blob_.len = 0;
            buff_blob_.data = NULL;
            buff_blob_.ext_type = EXT_TYPE_NONE;
            buff_blob_.ext_len = 0;
            buff_blob_.extdata = EXT_SBUFF;
            unsigned flow;
            for(list<unsigned>::iterator it = timeout_list.begin(); it != timeout_list.end(); it++)
            {
                flow = *it;
                ix_->connset_->closeconn(flow);
                func_list_[CB_TIMEOUT](flow, &buff_blob_, func_args_[CB_TIMEOUT]);
            }
        }
        //无超时回调
        else
        {
            for(list<unsigned>::iterator it = timeout_list.begin(); it != timeout_list.end(); it++)
            {
                ix_->connset_->closeconn(*it);
            }
        }

        lastchecktime_ = now;

    }
}
int CTSockCommu::create_sock(const TSockBind* s)
{
    //建立tcp\udp\unixsocket侦听fd
    int fd = CSocket::create(s->type_);
    if(unlikely(fd < 0)) 
        return -1;

    CSocket::set_reuseaddr(fd);

    int ret = CSocket::set_nonblock(fd);
    if(unlikely(ret)) 
        return ret;

    if(s->type_ != SOCK_TYPE_UNIX)
        ret = CSocket::bind(fd, s->ipport_.ip_, s->ipport_.port_);	
    else
        ret = CSocket::bind(fd, s->path_);

    if(unlikely(ret)) 
        return ret;

    if(s->type_ == SOCK_TYPE_UDP)
        return fd;

    ret = CSocket::listen(fd, 1024);
    
    if(likely(!ret))
        return fd;
    else
        return -1;
}

int CTSockCommu::create_sock(int sock_type)
{
    //建立tcp\udp\unixsocket
    int fd = CSocket::create(sock_type);
    if(unlikely(fd < 0)) 
        return -1;

    CSocket::set_reuseaddr(fd);

    int ret = CSocket::set_nonblock(fd);
    if(unlikely(ret)) 
        return ret;

    return fd;
}

void CTSockCommu::handle_accept(int fd)
{
    int ret = 0;

    //接受socket
    int clifd = 0;
    while(true)
    {
        ret = CSocket::accept(fd);
        if(likely(ret > 0))
            break;
        else if(errno == EINTR || errno == EAGAIN)
            continue;
        else
        {
#ifdef OPEN_PRINT	
            printf("accept error ret=%d,error=%m\n", ret);
#endif	
            return;			
        }
    }

    clifd = ret;

    if(unlikely(++flow_ == 0))//这里flow开始自增
    {
        flow_ = 1;
    }

    //连接建立回调
    if(func_list_[CB_CONNECTED] != NULL)
    {
        buff_blob_.len = 0;
        buff_blob_.data = NULL;

        if(likely(sockbind_[fd].type_ == SOCK_TYPE_TCP))	//tcp客户端取其信息
        {	
            extinfo_.fd_ = clifd;
            extinfo_.type_ = sockbind_[fd].type_;
            extinfo_.localip_ = sockbind_[fd].ipport_.ip_;
            extinfo_.localport_ = sockbind_[fd].ipport_.port_;
            CSocket::get_peer_name(clifd, extinfo_.remoteip_, extinfo_.remoteport_);
            buff_blob_.ext_type = EXT_TYPE_CONNEXTINFO;
            buff_blob_.extdata = EXT_SBUFF;
            memcpy(buff_blob_.extdata,&extinfo_,sizeof(TConnExtInfo) );
            buff_blob_.ext_len = sizeof(extinfo_);
        }
        else												//unix socket客户端
        {
            buff_blob_.ext_type = EXT_TYPE_NONE;
            buff_blob_.ext_len = 0;
            buff_blob_.extdata = EXT_SBUFF;
        }	

		//调用连接成功后回调
        if(func_list_[CB_CONNECTED](flow_, &buff_blob_, func_args_[CB_CONNECTED]) != 0)
        {
#ifdef OPEN_PRINT	
            printf("deny!!! client fd=%d\n", clifd);
#endif	
            close(clifd);
            return;
        }
    }

    ret = CSocket::set_nonblock(clifd);
    if(unlikely(ret))
    {
        close(clifd);
        return;
    }

    //加入连接池

    ret = ix_->connset_->addconn(flow_, clifd, sockbind_[fd].type_);
    if(unlikely(ret < 0))
    {
        close(clifd);
        if(func_list_[CB_OVERLOAD] != NULL)
        {
            buff_blob_.len = 0;
            buff_blob_.data = (char*)COMMU_ERR_OVERLOAD_CONN; 
            buff_blob_.ext_type = EXT_TYPE_NONE;
            buff_blob_.ext_len = 0;
            buff_blob_.extdata = EXT_SBUFF;

            func_list_[CB_OVERLOAD](0, &buff_blob_, func_args_[CB_OVERLOAD]);
        }
#ifdef OPEN_PRINT
        printf("%s:%d:%s overload\n", __FILE__, __LINE__, __FUNCTION__);
#endif
        return;
    }
	
#ifdef OPEN_PRINT	
    printf("add conn, flow=%d,client fd=%d,sock type=%d\n", flow_, clifd, sockbind_[fd].type_);
#endif	

    //加入epoll池
    ret = ix_->epollflow_->add(clifd, flow_, EPOLLET | EPOLLIN);
    if(unlikely(ret < 0))
    {
        close(clifd);
        printf("epollflow add error, ret =%d\n", ret);
        return;
    }

#ifdef OPEN_PRINT	
    printf("add epoll flow, flow=%d,client fd =%d\n", flow_, clifd);
#endif	
}
void CTSockCommu::handle_accept_udp(int fd)
{
    //加入连接池
    if(unlikely(++flow_ == 0))
    {
        flow_ = 1;
    }	
    int ret = ix_->connset_->addconn(flow_, fd, UDP_SOCKET);
    if(unlikely(ret < 0))
    {
        if(func_list_[CB_OVERLOAD] != NULL)
        {
            buff_blob_.len = 0;
            buff_blob_.data = (char*)COMMU_ERR_OVERLOAD_CONN;
            buff_blob_.ext_type = EXT_TYPE_NONE;
            buff_blob_.ext_len = 0;
            buff_blob_.extdata = EXT_SBUFF;
            func_list_[CB_OVERLOAD](0, &buff_blob_, func_args_[CB_OVERLOAD]);
        }

#ifdef OPEN_PRINT		
        printf("%s:%d:%s overload\n", __FILE__, __LINE__, __FUNCTION__);
#endif
        return;
    }

    //收数据
    ret = ix_->connset_->recv(flow_);
    if(likely(ret > 0))
    {	
        ConnCache* cc = ix_->connset_->getcc(flow_);	
        buff_blob_.len = cc->_r.data_len(); 
        buff_blob_.data = cc->_r.data();

        extinfo_.fd_ = fd;
        extinfo_.type_ = SOCK_TYPE_UDP;
        extinfo_.localip_ = sockbind_[fd].ipport_.ip_;
        extinfo_.localport_ = sockbind_[fd].ipport_.port_;
        extinfo_.remoteip_ = cc->_addr.get_numeric_ipv4();
        extinfo_.remoteport_ = cc->_addr.get_port();	

        buff_blob_.ext_type = EXT_TYPE_NONE;
        buff_blob_.extdata = EXT_SBUFF;
        memcpy(buff_blob_.extdata,&extinfo_,sizeof(TConnExtInfo));
        buff_blob_.ext_len = sizeof(TConnExtInfo);

        if(func_list_[CB_CONNECTED] != NULL)
        {
            if(func_list_[CB_CONNECTED](flow_, &buff_blob_, func_args_[CB_CONNECTED]) != 0)
            {
#ifdef OPEN_PRINT	
                printf("deny udp flow=%d\n", flow_);
#endif	
                ix_->connset_->closeconn(flow_);
                return;
            }
        }
#ifdef LOAD_CHECK_ENABLE			
        //
        // 负载监控协议，取当前负载，just hack!!!
        //
        if(buff_blob_.len == LOAD_DETECT_CMD_LEN && 
            *((int*)&buff_blob_.data[0]) == LOAD_DETECT_CMD_1 && 
            *((int*)&buff_blob_.data[sizeof(int)]) == LOAD_DETECT_CMD_2)
        {
            int curload = myload_.peek_load();	
            ix_->connset_->send(flow_, (char*)&curload, sizeof(int));
            ix_->connset_->closeconn(flow_);
            return;
        }

        //
        // 检查是否过载
        //
        if(unlikely(myload_.check_load()))
        {
            ix_->connset_->closeconn(flow_);

            if(func_list_[CB_OVERLOAD] != NULL)
            {
                buff_blob_.len = 0;
                buff_blob_.data = (char*)COMMU_ERR_OVERLOAD_PKG; 
                buff_blob_.ext_type = EXT_TYPE_NONE;
                buff_blob_.ext_len = 0;
                buff_blob_.extdata = EXT_SBUFF;
                func_list_[CB_OVERLOAD](0, &buff_blob_, func_args_[CB_OVERLOAD]);
            }
            printf("%s:%d:%s overload\n", __FILE__, __LINE__, __FUNCTION__);

            return;
        }
        myload_.grow_load(1);
#endif

        buff_blob_.ext_type = EXT_TYPE_CONNEXTINFO;//附带的连接信息
        buff_blob_.extdata = EXT_SBUFF;
        memcpy(buff_blob_.extdata,&extinfo_,sizeof(TConnExtInfo));
        buff_blob_.ext_len = sizeof(TConnExtInfo);
        //udp数据一次就可以收完整
        func_list_[CB_RECVDATA](flow_, &buff_blob_, func_args_[CB_RECVDATA]);
    }
    else
    {
        ix_->connset_->closeconn(flow_);
    }
}
int CTSockCommu::init(const void* config)
{
    fini();

    TSockCommuConf* conf = (TSockCommuConf*)config;

    //printf("max_conn=%d,maxpkg=%d,expiretime=%d\n",conf->maxconn_,conf->maxpkg_,conf->expiretime_);
    assert(conf->maxconn_ > 0 && conf->maxpkg_ >= 0 && conf->expiretime_ > 0);

    //建立侦听fd
    memset(sockbind_, 0x0, sizeof(TSockBind) * SOCK_MAX_BIND);
    int fd = 0;
    TSockBind* s = NULL;	
    maxbindfd_ = 0;


    for(int i = 0; i < SOCK_MAX_BIND; ++i)
    {
        s = &conf->sockbind_[i];
        if(s && s->type_ != 0)
        {
            fd = create_sock(s);
            assert(fd >= 0 && fd < SOCK_MAX_BIND);
            memcpy(&sockbind_[fd], s, sizeof(TSockBind));

            if(fd > maxbindfd_)
            {
                maxbindfd_ = fd;
            }
        }
        else
        {
            break;
        }
    }



    //最大连接数
    maxconn_ = conf->maxconn_;	

    //创建内存池	
    ix_->mempool_ = new CMemPool();
    //创建连接集合
    ix_->connset_ = new CConnSet(*ix_->mempool_, maxconn_);
    //创建epoll对象
    ix_->epollflow_ = new CEPollFlow();
    ix_->epollflow_->create(100000); //目前的epoll实现下，此参数无关紧要


    for(int i = 0; i <= maxbindfd_; ++i)
    {
        if(sockbind_[i].type_ == SOCK_TYPE_TCP || sockbind_[i].type_ == SOCK_TYPE_UNIX)
        {
            //ix_->epollflow_->add(i, 0, EPOLLET | EPOLLIN);
            ix_->epollflow_->add(i, 0, EPOLLIN);
        }
        else if(sockbind_[i].type_ == SOCK_TYPE_UDP)
        {
            //ix_->epollflow_->add(i, 0 , EPOLLET | EPOLLIN | EPOLLOUT);
            ix_->epollflow_->add(i, 0 , EPOLLIN);
        }
        else	//unused	
        {

        } 
    }	
	

    //其他初始化
    expiretime_ = conf->expiretime_;
    check_expire_interval_ = conf->check_expire_interval_;
    buff_blob_.owner = this;
    buff_blob_.ext_type = EXT_TYPE_NONE;
    buff_blob_.ext_len = 0;
    buff_blob_.extdata = EXT_SBUFF;
    memset(&extinfo_, 0x0, sizeof(TConnExtInfo)); //初始化扩展信息
    lastchecktime_ = get_timebytask(NULL);//设置上次超时检查时间为当前时间
    flow_ = 0; //序列号重置
#ifdef LOAD_CHECK_ENABLE	
    myload_.maxload(conf->maxpkg_); //设置每秒最大包量
#endif		
    return 0;
}

int CTSockCommu::InitExt(const void * config)
{
    fini();	

    TRouteConfig * conf = (TRouteConfig *) config;

    assert(conf != NULL );

    assert(conf->max_conn_ > 0 && conf->maxpkg_ >= 0 && conf->expiretime_ > 0 && conf->check_expire_interval_ >= 0 );

    //最大连接数
    maxconn_ = conf->max_conn_;

    flow_ = 0;					//序列号重置

    //创建内存池	
    ix_->mempool_ = new CMemPool();
    //创建连接集合
    ix_->connset_ = new CConnSet(*ix_->mempool_, maxconn_);
    //创建epoll对象
    ix_->epollflow_ = new CEPollFlow();
    ix_->epollflow_->create(100000); //目前的epoll实现下，此参数无关紧要
    maxbindfd_ = 0;

    //其他初始化
    expiretime_ = conf->expiretime_;
    check_expire_interval_ = conf->check_expire_interval_;
    buff_blob_.owner = this;	
    buff_blob_.ext_type = EXT_TYPE_NONE;
    buff_blob_.ext_len = 0;
    buff_blob_.extdata = EXT_SBUFF;
    lastchecktime_ = get_timebytask(NULL);//设置上次超时检查时间为当前时间

#ifdef LOAD_CHECK_ENABLE	
    myload_.maxload(conf->maxpkg_); //设置每秒最大包量
#endif

    return 0;
}



int CTSockCommu::connect(const TNodeInfo * pNode)
{
    assert(pNode != NULL );

#ifdef OPEN_PRINT
    pNode->Show();
#endif

    //内存池，连接池，EPOLL池都已经创建好
    assert( ix_->mempool_ != NULL && ix_->connset_ != NULL && ix_->epollflow_ != NULL );

    int fd = 0;
    if(pNode->bind_.type_ == 0 )
    {
#ifdef OPEN_PRINT	
        printf("invalid sock type,type= %d\n",pNode->bind_.type_);
#endif			
        return -1;//Invalid Type
    }

    fd = create_sock(pNode->bind_.type_);
    assert(fd >= 0);

    if(unlikely(++flow_ == 0))
    {
        flow_ = 1;
    }

    //连接到服务器
    int ret = CSocket::connect(fd, pNode->bind_.ipport_.ip_,pNode->bind_.ipport_.port_);

    //加入连接池	
    ret = ix_->connset_->addconn(flow_, fd, pNode->bind_.type_);
    if(unlikely(ret < 0))
    {
        close(fd);
        if(func_list_[CB_OVERLOAD] != NULL)
        {
            buff_blob_.len = 0;
            buff_blob_.data = (char*)COMMU_ERR_OVERLOAD_CONN; 
            buff_blob_.ext_type = EXT_TYPE_NONE;
            buff_blob_.ext_len = 0;
            buff_blob_.extdata = EXT_SBUFF;
            func_list_[CB_OVERLOAD](0, &buff_blob_, func_args_[CB_OVERLOAD]);
        }
#ifdef OPEN_PRINT
        printf("file:%s:%d:%s overload\n", __FILE__, __LINE__, __FUNCTION__);
#endif
        return -1;
    }
	
#ifdef OPEN_PRINT	
    printf("add conn, flow=%d,fd=%d,type=%d\n", flow_, fd, pNode->bind_.type_);
#endif	

    //加入epoll池
    ret = ix_->epollflow_->add(fd, flow_,   EPOLLET | EPOLLIN );
    if(unlikely(ret < 0))
    {
        close(fd);
#ifdef OPEN_PRINT	
        printf("epollflow add error, %d\n", ret);
#endif
        return -2;
    }

    //连接建立回调
    if(func_list_[CB_CONNECT] != NULL)
    {
        buff_blob_.len = 0;
        buff_blob_.data = NULL;		
        buff_blob_.ext_type = EXT_TYPE_CONNNODEINFO;
        buff_blob_.extdata = EXT_SBUFF;
        memcpy(buff_blob_.extdata,pNode,sizeof(TNodeInfo));
        buff_blob_.ext_len = sizeof(TNodeInfo);


        if(func_list_[CB_CONNECT](flow_, &buff_blob_, func_args_[CB_CONNECT]) != 0)
        {
#ifdef OPEN_PRINT	
            printf("deny %d\n", fd);
#endif	
            close(fd);
            return -3;
        }
    }


    if(func_list_[CB_CONNECTED] != NULL )
    {
        buff_blob_.len = 0;
        buff_blob_.data = NULL;
        buff_blob_.ext_type = EXT_TYPE_CONNECTED;
        buff_blob_.extdata = EXT_SBUFF;
        memcpy(buff_blob_.extdata,&extinfo_,sizeof(TConnExtInfo));
        buff_blob_.ext_len = sizeof(TConnExtInfo);

        if(func_list_[CB_CONNECTED](flow_,&buff_blob_,func_args_[CB_CONNECTED] ) != 0 )
        {
#ifdef OPEN_PRINT
            printf("deny %u\n",fd);
#endif
            close(fd);
            return -3;
        }
    }
    
    return 0;	

}


int CTSockCommu::poll(bool block)
{
    int timeout = 0;//不超时
    if(block)
    {	
        timeout = -1;
    }

    bool is_idle = false;

    CEPollFlowResult result = ix_->epollflow_->wait(timeout);
    int fd = 0;
    unsigned flow = 0;
    int ret = 0;
    if(result.size() == 0)
    {
        is_idle = true;
    }
    else
    {
        is_idle = false;
        //printf("epollresult,size=%lu\n",result.size());
    }

    for(CEPollFlowResult::iterator it = result.begin(); it != result.end(); it++)
    {
        fd = it.fd();
        flow = it.flow();
		
#ifdef OPEN_PRINT
        printf("epoll event, %d,%d\n", flow, fd);
#endif
        //判断是否侦听fd
        if(fd <= maxbindfd_ && sockbind_[fd].type_ != 0)
        {
            //新连接到达(tcp or unix)或者新数据到达(udp)
            if(likely(it->events & EPOLLIN))
            {
                if(sockbind_[fd].type_ != SOCK_TYPE_UDP)
                {
                    handle_accept(fd);
                }
                else
                {
                    handle_accept_udp(fd);
                }	
            }
            continue;	
        }

        //非正常事件，关闭连接
        if(unlikely(!(it->events & (EPOLLIN | EPOLLOUT)))) 
        {
            ix_->connset_->closeconn(flow);
#ifdef OPEN_PRINT
            printf("unknow event, close %d\n", flow);
#endif
            if(unlikely(func_list_[CB_DISCONNECT] != NULL))
            {
                buff_blob_.len = 0;
                buff_blob_.data = NULL;
                buff_blob_.ext_type = EXT_TYPE_NONE;
                buff_blob_.ext_len = 0;
                buff_blob_.extdata = EXT_SBUFF;
                func_list_[CB_DISCONNECT](flow, &buff_blob_, func_args_[CB_DISCONNECT]);
            }

            continue;
        }

        //发数据，可能有tcp\unix类型的数据
        if(it->events & EPOLLOUT)
        {
            ret = ix_->connset_->sendfromcache(flow);

#ifdef OPEN_PRINT
            printf("send %d,%d,%d\n", flow, fd, ret);
#endif
            //缓存发送完毕
            if(ret == 0)	
            {
                //去除EPOLLOUT事件
                ix_->epollflow_->modify(fd, flow, EPOLLIN | EPOLLET);
#ifdef OPEN_PRINT
                printf("send complete %d,%d,%d\n", flow, fd, ret);
#endif
            }
            //发送失败，关闭连接
            else if(unlikely(ret == -E_NEED_CLOSE)) 
            {
                if(unlikely(func_list_[CB_SENDERROR] != NULL))
                {
                    ConnCache* cc = ix_->connset_->getcc(flow);
                    buff_blob_.len = cc->_w.data_len();
                    buff_blob_.data = cc->_w.data();
                    buff_blob_.ext_type = EXT_TYPE_CONNEXTINFO;
                    buff_blob_.extdata = EXT_SBUFF;
                    				
                    extinfo_.fd_ = fd;
                    extinfo_.type_ = cc->_type;
                    extinfo_.localip_ = 0;
                    extinfo_.localport_ = 0;
                    CSocket::get_peer_name(fd, extinfo_.remoteip_, extinfo_.remoteport_);

                    memcpy(buff_blob_.extdata,&extinfo_,sizeof(TConnExtInfo));
                    buff_blob_.ext_len = sizeof(TConnExtInfo);

                    func_list_[CB_SENDERROR](flow, &buff_blob_, func_args_[CB_SENDERROR]);
                }
                
                ix_->connset_->closeconn(flow);
#ifdef OPEN_PRINT				
                printf("send fail, close %d\n", flow);
#endif
            }
            //缓存发送未完毕,EPOLLOUT继续存在,如果E_NOT_FINDFD，epoll已经自动的将该fd删除了,无需处理	
            else
            {
                //assert(ret > 0 || ret == -E_NOT_FINDFD);
#ifdef OPEN_PRINT
                printf("send not complete %d,%d,%d\n", flow, fd, ret);
#endif
            }	
        }

        //收数据，这里收数据只有tcp和unix类型的
        if(it->events & EPOLLIN)
        {
            while(true)	
            {
                try
                {	
                    ret = ix_->connset_->recv(flow);
                }
                //内存不足
                catch(...)	
                {
                    ret = -E_NEED_CLOSE;
                    printf("out of memory\n");
                }

#ifdef OPEN_PRINT
                printf("recv %d,%d,%d\n", flow, fd, ret);
#endif
                //收到了数据
                if(likely(ret > 0))					
                {
#ifdef OPEN_PRINT
                    printf("recv data %d,%d,%d\n", flow, fd, ret);
#endif
                    ConnCache* cc = ix_->connset_->getcc(flow);	
                    buff_blob_.len = cc->_r.data_len(); 
                    buff_blob_.data = cc->_r.data();					
					
#ifdef LOAD_CHECK_ENABLE					
                    //
                    // 负载监控协议，取当前负载，just hack!!!
                    //
                    if(buff_blob_.len == LOAD_DETECT_CMD_LEN && 
                    *((int*)&buff_blob_.data[0]) == LOAD_DETECT_CMD_1 && 
                    *((int*)&buff_blob_.data[sizeof(int)]) == LOAD_DETECT_CMD_2)
                    {
                        int curload = myload_.peek_load();	
                        ix_->connset_->send(flow, (char*)&curload, sizeof(int));
                        cc->_r.skip(LOAD_DETECT_CMD_LEN);
                        break;
                    }
                    //
                    // 检查是否过载
                    //
                    if(unlikely(myload_.check_load()))
                    {
                        ix_->connset_->closeconn(flow);

                        if(func_list_[CB_OVERLOAD] != NULL)
                        {
                            buff_blob_.len = 0;
                            buff_blob_.data = (char*)COMMU_ERR_OVERLOAD_PKG; 
                            buff_blob_.ext_type = EXT_TYPE_NONE;
                            buff_blob_.ext_len = 0;
                            buff_blob_.extdata = EXT_SBUFF;
                            func_list_[CB_OVERLOAD](0, &buff_blob_, func_args_[CB_OVERLOAD]);
                        }
                        printf("%s:%d:%s overload\n", __FILE__, __LINE__, __FUNCTION__);
                        break;
                    }
                    //---->
#endif				
                    extinfo_.fd_ = fd; 
                    extinfo_.type_ = cc->_type;
                    extinfo_.localip_ = 0;
                    extinfo_.localport_ = 0;
                    CSocket::get_peer_name(fd, extinfo_.remoteip_, extinfo_.remoteport_);

                    buff_blob_.ext_type = EXT_TYPE_CONNEXTINFO;
                    buff_blob_.ext_len = sizeof(TConnExtInfo);
                    buff_blob_.extdata = EXT_SBUFF;
                    memcpy(buff_blob_.extdata,&extinfo_,sizeof(TConnExtInfo));

                    ret = func_list_[CB_RECVDATA](flow, &buff_blob_, func_args_[CB_RECVDATA]);

                    //数据不完整
                    if(ret == 0)		
                    {
                        continue;
                    }	
                    //数据完整, ret是数据长度		
                    else if(ret > 0)	
                    {
                        buff_blob_.len = ret;
                        if(func_list_[CB_RECVDONE] != NULL)
                        {
                            func_list_[CB_RECVDONE](flow, &buff_blob_, func_args_[CB_RECVDONE]);
                        }

                        cc->_r.skip(ret);	
#ifdef LOAD_CHECK_ENABLE						
                        myload_.grow_load(1);
#endif					
                    }
                    //被判定为错误的数据
                    else				
                    {
                        ix_->connset_->closeconn(flow);
#ifdef OPEN_PRINT
                        printf("invalid data, close %d\n", flow);
#endif
                        break;
                    }
                }
                //客户端主动关闭连接或者其他错误
                else if(ret == -E_NEED_CLOSE)	
                {
                    if(unlikely(func_list_[CB_DISCONNECT] != NULL))
                    {
                        buff_blob_.len = 0;
                        buff_blob_.data = NULL;
                        buff_blob_.ext_type = EXT_TYPE_NONE;
                        buff_blob_.ext_len = 0;
                        buff_blob_.extdata = EXT_SBUFF;
                        func_list_[CB_DISCONNECT](flow, &buff_blob_, func_args_[CB_DISCONNECT]);
                    }
                    ix_->connset_->closeconn(flow);
#ifdef OPEN_PRINT					
                    printf("client close, close %d\n", flow);
#endif				
                    break;
                }
                //连接已经不存在或者暂时不可读
                else if(ret == -E_NOT_FINDFD || ret == -EAGAIN)
                {
#ifdef OPEN_PRINT
                    printf("recv not complete %d,%d,%d\n", flow, fd, ret);
#endif
                    break;
                }	
            }
        }
    }

    check_expire();
    return is_idle?-1:0;
}
int CTSockCommu::sendto(unsigned flow, void* arg1, void* arg2)
{
    blob_type* blob = (blob_type*)arg1;

#ifdef OPEN_PRINT	
    printf("%s blob_len=%d,flow=%d\n",__FUNCTION__,blob->len,flow);
#endif

    if(unlikely(blob->len == 0 && flow > 0))  //主动关闭连接
    {
        ix_->connset_->closeconn(flow);
        return 0;
    }

    int ret = 0;
    try
    {
        ret = ix_->connset_->send(flow, blob->data, blob->len);
    }
    //内存不足
    catch(...)  
    {
        ret = -E_NEED_CLOSE;
        printf("out of memory\n");
    }

    //发送了数据
    if(likely(ret >= 0))
    {
        //未发送完
        if(ret < blob->len)		 
        {
            ConnCache* cc = ix_->connset_->getcc(flow);	
            //如果不是udp则需要添加EPOLLOUT事件
            if(cc->_type != UDP_SOCKET)
            {
                ix_->epollflow_->modify(cc->_fd, flow, EPOLLET | EPOLLIN | EPOLLOUT);
            }
            else
            {
                ix_->connset_->closeconn(flow);
            }
        }
        //发送完毕
        else
        {	 					
            assert(ret == blob->len);

            ConnCache* cc = ix_->connset_->getcc(flow);	
            if(unlikely(func_list_[CB_SENDDONE] != NULL))
            {
                buff_blob_.len = blob->len;
                buff_blob_.data = blob->data;				

                extinfo_.fd_ = cc->_fd;
                extinfo_.type_ = cc->_type;
                extinfo_.localip_ = 0;
                extinfo_.localport_ = 0;

                if(cc->_type != UDP_SOCKET)
                {
                    CSocket::get_peer_name(cc->_fd, extinfo_.remoteip_, extinfo_.remoteport_);
                }
                else
                {
                    extinfo_.remoteip_ = cc->_addr.get_numeric_ipv4();
                    extinfo_.remoteip_ = cc->_addr.get_port();	
                }

                buff_blob_.ext_type = EXT_TYPE_CONNEXTINFO;
                buff_blob_.ext_len = sizeof(TConnExtInfo);
                buff_blob_.extdata = EXT_SBUFF;
                memcpy(buff_blob_.extdata,&extinfo_,sizeof(TConnExtInfo));

                func_list_[CB_SENDDONE](flow, &buff_blob_, func_args_[CB_SENDDONE]);
            }
            
            if(cc->_type != UDP_SOCKET) 
            {
                ix_->epollflow_->modify(cc->_fd, flow, EPOLLIN | EPOLLET);
            }
            else //(cc->_type == UDP_SOCKET)
            {
                ix_->connset_->closeconn(flow);
            }
        }
    }
    else
    {
        //发送出错，关闭连接
        if(ret == -E_NEED_CLOSE)		
        {
            ix_->connset_->closeconn(flow);
#ifdef OPEN_PRINT			
            printf("send error, %d\n", flow);
#endif		
        }	
        //没找到连接
        else
        {
            //assert(ret == -E_NOT_FINDFD);
#ifdef OPEN_PRINT	
            printf("not found, %d\n", flow);
#endif		
        }	
    }
    return ret;
}
int CTSockCommu::ctrl(unsigned flow, ctrl_type type, void* arg1, void* arg2)
{
    switch(type)
    {
        case CT_LOAD://请求负载信息
        {
            char * buf = (char *) arg1;
            int * len = (int *) arg2;
            unsigned int curconn = ix_->connset_->getcurconn();//当前连接数
            memcpy(buf,&curconn,sizeof(unsigned int));
            *len =sizeof(unsigned int);
        }
        break;

        case CT_GET_CONN_EXT_INFO:
        {
            ConnCache * cc =  ix_->connset_->getcc(flow);
            if(cc->_flow  != flow )
                return -1;

            char * buf = (char * ) arg1;
            unsigned int * len = (unsigned *) arg2;

            if( sizeof(TConnExtInfo) > (*len) )
                return -2;

            extinfo_.fd_ = cc->_fd; 
            extinfo_.type_ = cc->_type;
            extinfo_.localip_ = 0;
            extinfo_.localport_ = 0;
            CSocket::get_peer_name(cc->_fd, extinfo_.remoteip_, extinfo_.remoteport_);

            memcpy(buf,&extinfo_,sizeof(TConnExtInfo));
            *len = sizeof(TConnExtInfo);

        }
        break;
			
        case CT_STAT:
        case CT_DISCONNECT:
        case CT_CLOSE:
        default:
        {
        }
        break;
    }
    return 0;
}

