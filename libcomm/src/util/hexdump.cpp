#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <iomanip>
#include <fstream>
#include <ctime>
#include <iostream>
#include <hexdump.h>


using namespace std;

namespace comm
{
namespace util
{


void HexDumpImp(const void *pdata, unsigned int len)
{
       if(pdata == 0 || len == 0)
        {
                return;
        }
    
        int cnt = 0;
        int n = 0;
        int cnt2 = 0;
        const char *data = (const char *)pdata;
        cout<<"Address               Hexadecimal values                  Printable\n";
        cout<<"-------  -----------------------------------------------  -------------\n";
        cout << "\n";
        unsigned char buffer[20];
        unsigned int rpos = 0;

        while ( 1 )
        {
                if(len <= rpos)
                {
                        break;
                }
                if(len >= rpos + 16)
                {
                        memcpy(buffer, data + rpos, 16);
                        rpos += 16;
                        cnt = 16;
                }
                else
                {
                        memcpy(buffer, data + rpos, len - rpos);
                        cnt = len - rpos;
                        rpos = len;
                }
                if(cnt <= 0)
                {
                        return;
                }

                 cout << setw(7) << ( int ) rpos << "  ";

                cnt2 = 0;
                for ( n = 0; n < 16; n++ )
                {
                        cnt2 = cnt2 + 1;
                        if ( cnt2 <= cnt )
                        {
                                cout << hex << setw(2) << setfill ( '0' ) <<  (unsigned int)buffer[n];
                        }
                        else
                        {
                                cout << "  ";
                        }
                        cout << " ";
                }

                cout << setfill ( ' ' );

                cout << " ";
                cnt2 = 0;
                for ( n = 0; n < 16; n++ )
                {
                        cnt2 = cnt2 + 1;
                        if ( cnt2 <= cnt )
                        {
                                if ( buffer[n] < 32 || 126 < buffer[n] )
                                {
                                        cout << '.';
                                }
                                else
                                {
                                        cout << buffer[n];
                                }
                        }
                }
                cout << "\n";
                cout << dec;
        }

        return;
}

}
}

