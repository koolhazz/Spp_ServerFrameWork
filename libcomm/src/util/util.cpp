
#include "util.h"

namespace comm
{
namespace util
{

bool IsDirExist(const char *pszDir)
{
    struct stat s;
    int r = stat(pszDir, &s);
    if((r == 0) && (s.st_mode & S_IFDIR))
    {
        return true;
    }
    return false;
}

bool IsFileExist(const char *pszName)
{
    struct stat s;
    int r = stat(pszName, &s);
    return(r==0);
}

bool MkDir(const char *pszName, mode_t mode)
{
    if(IsDirExist(pszName))
    {
        return 0;
    }

    if(IsFileExist(pszName))
    {
        return -2;                     
    }


    const char *p = pszName;
    if(*p == '/')
    {
        p++;
    }

    std::string strDir;

    while(*p != 0)
    {
        while((*p != '/') && (*p != 0))
        {
            p++;
        }
        
        strDir.assign(pszName, p - pszName);
        if(false == IsDirExist(strDir.c_str()))
        {
            if(-1 == mkdir(strDir.c_str(),mode))
            {
                return -1;
            }
        }
        
        if(*p == 0)
        {
            break;
        }
        p++;
    }
    return 0;
}



char *strlower(char *s)
{
    char *p = s;
    for(;*s;s++)*s=(*s >= 'A' && *s <= 'Z')?(*s += 'a' - 'A'):(*s);
    return p;
}



char *strupper(char *s)
{
    char *p = s;
    for(;*s;s++)*s=(*s >= 'a' && *s <= 'z')?(*s -= 'a' - 'A'):(*s);
    return p;
}


}
}

