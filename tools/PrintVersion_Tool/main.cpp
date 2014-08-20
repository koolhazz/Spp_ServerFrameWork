#include <iostream>
#include <dlfcn.h>

static const char * VERSION_INFO="print version tool v1.0.0";
typedef void		(*spp_handle_version_t)();//¥Ú”°∞Ê±æ∫≈

using namespace std;

void Usage(const char * prog)
{
    cout<<VERSION_INFO<<"build time:"<<__DATE__<<endl;
    cout<<"Usage:"<<prog<<" filename"<<endl;
}
int main(int argc,char ** argv)
{
    if(argc < 2)
    {
        Usage(argv[0]);
        return 0;
    }
    const char * file = argv[1];
    void* handle = dlopen(file, RTLD_LAZY);
    if(handle == NULL)
    {
        cout<<"Error:Open so file fail!error info="<<dlerror()<<endl;
        return 0;
    }

    spp_handle_version_t spp_handle_version =NULL;

    spp_handle_version = (spp_handle_version_t)dlsym(handle, "print_version");

    if(spp_handle_version != NULL)
    {
        spp_handle_version();
    }
    else
    {
        cout<<"Error:function print_version not find!\n";
    }

    dlclose(handle);

    return 0;
    
}


