
#ifndef __COMM_COMMU_FIFO_H__
#define __COMM_COMMU_FIFO_H__

#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <stdexcept>
#include <iostream>
#include <assert.h>

using namespace std;

namespace comm
{
namespace commu
{

struct fifo_fail: public runtime_error{ fifo_fail(const string& s);};
struct fifo_delay: public runtime_error{ fifo_delay(const string& s);};

class CFifo 
{
public:
    CFifo();
    ~CFifo();

public:
    void open(const char *pathname,bool rorw, mode_t mode=0777) throw(fifo_fail);
    void close();

    int fd() { return _fd;} 
    int read(char * buffer,size_t max_size) throw(fifo_fail,fifo_delay);
    void write(const char *buffer,size_t buflen) throw(fifo_fail,fifo_delay);
private:
    void set_nonblock();

private:
    string _pathname;
    int _fd;
    bool _rorw;
};

}
}

#endif

