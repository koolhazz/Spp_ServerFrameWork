
#include "epoller.h"


using namespace comm::base;

//////////////////////////////////////////////////////////////////////////
//	implementation
//////////////////////////////////////////////////////////////////////////

inline void CEPoller::create(size_t iMaxFD) throw(std::runtime_error)
{
    _maxFD = iMaxFD;
    _fd = epoll_create(1024);
    if(_fd == -1)
        throw std::runtime_error("epoll_create fail [" + std::string(strerror(errno)) + "]");
    _events = new epoll_event[iMaxFD];
}

inline void CEPoller::add(int fd, int flag) throw(std::runtime_error)
{
    ctl(fd, EPOLL_CTL_ADD, flag);
}

inline void CEPoller::modify(int fd, int flag) throw(std::runtime_error)
{
    ctl(fd, EPOLL_CTL_MOD, flag);
}

inline CEPollResult CEPoller::wait(int iTimeout) throw(std::runtime_error)
{
    int nfds = epoll_wait(_fd, _events, _maxFD, iTimeout);
    if (nfds < 0)
    {
        if (errno != EINTR)
        {
            throw std::runtime_error("epoll_wait fail [" + std::string(strerror(errno)) + "]");
        }
        else
        {
            //std::cerr << strerror(errno) << std::endl;
            nfds = 0;
        }
    }
    
    return CEPollResult(_events, nfds);;
}

inline void CEPoller::ctl(int fd, int epollAction, int flag) throw(std::runtime_error)
{
    assert(_fd != -1);
    epoll_event ev;
    ev.data.fd = fd;
    ev.events = flag;
    int ret = epoll_ctl(_fd, epollAction, fd, &ev);
    if (ret < 0)
        throw std::runtime_error("epoll_ctl fail [" + std::string(strerror(errno)) + "]");
}



