/******************************************
*文件名:main.cpp
*功能:清理ipc的工具主函数
*创建者:钟何明
*创建时间:2009.07.16
*******************************************/

#include <ftw.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <string>
#include <unistd.h>
#include <errno.h>

using namespace std;

const unsigned char  TYPE_SHM  = 1;
const unsigned char TYPE_SEM = 2;
const unsigned char TYPE_MQUEUE = 3;

static char g_sMax_Proj_ID = 0xff;

static int do_print_key(const char * pszpathname,unsigned char type)
{
    //char cmd_buf[64] = {0};    
    //string strCmd;
    for(unsigned char index = 0;index < g_sMax_Proj_ID; ++index)
    {
        key_t key = ftok(pszpathname,index);
        printf("filename=%s,proj_id=%d,type=%d,key=0x%x\n",pszpathname,index,type,key);
        /*
        int ncount = snprintf(cmd_buf, sizeof(cmd_buf) - 1, "ipcrm -%c 0x%x > /dev/null 2>&1;", type,key);
        cmd_buf[ncount] = 0;
        strCmd += cmd_buf;
        */
    }
    //system(strCmd.c_str());
    return 0;
}

/*
FTW_F        fpath is a normal file.
FTW_D        fpath is a directory.
FTW_DNR   fpath is a directory which can’t be read.
FTW_NS    The stat(2) call failed on fpath, which is not a symbolic link.
*/
/*
*功能:ftw的回调函数
*fpath:文件路径
*sb:指向stat的指针
*typeflag:FTW_*旗标
*返回值:
*0成功,非0失败
*/
static int func_ipc_key_print(const char *fpath, const struct stat *sb,int typeflag)
{
    unsigned char  chType = 0;
    
    if(typeflag == FTW_F )
    { 
         char * pExt = strrchr(fpath,'.');
        if(pExt == NULL )
        {
            printf("file:%s is a invalid  file type\n",fpath);
            return 0;
        }

        string strExt = pExt;
        if(strExt == ".mq")
            chType = 'Q';
        else if(strExt == ".shm")
            chType = 'M';
         else if(strExt == ".sem")
            chType = 'S';

        if(chType == 0 ) 
        {
            printf("file:%s is a invalid  file type\n",fpath);
            return 0;
        }

        do_print_key(fpath,chType);      
     
    }
    return 0;
}



static int usage(char * argv[])
{
    printf("%s path Max_Proj_ID\n",argv[0]);   
    printf("example:%s /root/dat/key/ 255 \n",argv[0]);
    return 0;
}


int  main(int argc, char *argv[])
{
    if(argc < 3)
        return usage(argv);

    char * path = argv[1];
    g_sMax_Proj_ID = atoi(argv[2]);
    printf("Max_Proj_ID=%d\n",g_sMax_Proj_ID);
    
   int flags = 0;
   int ret = ftw(path, func_ipc_key_print, flags);
   if(ret == -1)
   {
        printf("ftw ret=%d,errno=%d,errmsg=%s\n",ret,errno,strerror(errno));
   }
    return 0;
}
