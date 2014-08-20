

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "fifo.h"

using namespace comm::commu;

fifo_fail::fifo_fail(const string & s):runtime_error(s) {}
fifo_delay::fifo_delay(const string & s):runtime_error(s) {}

CFifo::CFifo()
{
    _fd = -1;
    _rorw = true;
}

CFifo::~CFifo()
{
    close();
}

void CFifo::close()
{
    if(_fd>0) 
    {
        ::close(_fd); 
        _fd = -1;
    }
}

inline void CFifo::set_nonblock()
{
    assert(_fd > 0);
    int val = fcntl(_fd, F_GETFL, 0);
    if (val == -1)
        throw fifo_fail(string("CFifo set_nonblock:")+strerror(errno));
    
    if (val & O_NONBLOCK)
        return;

    if (fcntl(_fd, F_SETFL, val | O_NONBLOCK | O_NDELAY) == -1)
        throw fifo_fail(string("CFifo set_nonblock:")+strerror(errno));
}

void CFifo::open(const char *pathname, bool rorw, mode_t mode) throw(fifo_fail)
{
    _rorw = rorw;
    _pathname = pathname;
    if(mkfifo(pathname,mode))
    {
        if(errno == EEXIST) 
        {
            //cout << "fifo: " << pathname << " exist" << endl;
        }
        else 
        {
            throw fifo_fail(string("mkfifo fifo ")+pathname+" fail");
        }
    }
    
    if(rorw)
    {
        _fd = ::open(pathname,O_NONBLOCK|O_RDONLY);
        if(_fd < 0)
        {
            throw fifo_fail(string("open read fifo fail")+strerror(errno));
        }
    } 
    else
    {
        _fd = ::open(pathname,O_NONBLOCK|O_WRONLY);
        if(_fd < 0) 
        {
            if(errno == ENXIO)
            {
            //cout << "open fifo for write but no reading" << endl;
            return;
            }
            throw fifo_fail(string("open write fifo fail:")+strerror(errno));
        }
    }
}

int CFifo::read(char * buffer,size_t max_size) throw(fifo_fail,fifo_delay)
{
    int iRetVal;
    for(;;)
    {
        iRetVal = ::read(_fd,buffer,max_size);
        if(iRetVal<0) 
        {
            if(errno == EINTR)
            {
                //cerr << "CFifo:read EINTR" << endl;
                continue;
            }
            else if(errno == EAGAIN) 
            { //
                //cerr << "CFifo:read EAGAIN" << endl;
                throw fifo_delay("CFifo read EAGAIN");
            }
            else 
            {
                throw fifo_fail(string("read fifo fail:")+strerror(errno));
            }
        } 
        else if(iRetVal==0) 
        {
            throw fifo_fail("fifo closed");
        }
        else 
        {
            break;
        }
    }

    return iRetVal;
}

void CFifo::write(const char *buffer,size_t buflen) throw(fifo_fail,fifo_delay)
{
    assert(buflen>0);

    if(_fd<0 && (!_rorw)) 
    {
        // re open fifo
        _fd = ::open(_pathname.c_str(),O_NONBLOCK|O_WRONLY);
        if(_fd < 0) 
        {
            if(errno == ENXIO) 
            {
                throw fifo_delay(string("CFifo: open write fifo fail:")+strerror(errno));
            }
            throw fifo_fail(string("CFifo: open write fifo fail:")+strerror(errno));
        }
    }

    for(;;)
    {
        int iRetVal = ::write(_fd,buffer,buflen);
        if(iRetVal<0)
        {
            if(errno == EINTR)
            {
                //	cerr << "CFifo:read EINTR" << endl;
                continue;
            }
            else if(errno == EAGAIN || errno == EPIPE)
            { //
                //				cerr << "CFifo:write EAGAIN" << endl;
                throw fifo_delay(string("CFifo write EAGAIN|EPIPE:")+strerror(errno));
            }
            else 
            {
                throw fifo_fail(string("write fifo fail:")+strerror(errno));
            }
        } 
        else if(iRetVal==0)
        {
            throw fifo_fail("fife closed");
        } 
        else 
        {
            break;
        }
    }
}

