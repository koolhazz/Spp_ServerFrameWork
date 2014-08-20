#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "base64.h"

namespace comm
{
namespace util
{

void base64_encode(const char *src, int src_len, char *dst)
{
        int i = 0, j = 0;
        char base64_map[65] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

        for (; i < src_len - src_len % 3; i += 3) {
                dst[j++] = base64_map[(src[i] >> 2) & 0x3F];
                dst[j++] = base64_map[((src[i] << 4) & 0x30) + ((src[i + 1] >> 4) & 0xF)];
                dst[j++] = base64_map[((src[i + 1] << 2) & 0x3C) + ((src[i + 2] >> 6) & 0x3)];
                dst[j++] = base64_map[src[i + 2] & 0x3F];
        }

        if (src_len % 3 == 1) {
                 dst[j++] = base64_map[(src[i] >> 2) & 0x3F];
                 dst[j++] = base64_map[(src[i] << 4) & 0x30];
                 dst[j++] = '=';
                 dst[j++] = '=';
        }
        else if (src_len % 3 == 2) {
                dst[j++] = base64_map[(src[i] >> 2) & 0x3F];
                dst[j++] = base64_map[((src[i] << 4) & 0x30) + ((src[i + 1] >> 4) & 0xF)];
                dst[j++] = base64_map[(src[i + 1] << 2) & 0x3C];
                dst[j++] = '=';
        }

        dst[j] = '\0';
}

void base64_decode(const char *src, int src_len, char *dst)
{
        int i = 0, j = 0;
        char base64_decode_map[256] = {
             255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
             255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
             255, 255, 255, 62, 255, 255, 255, 63, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 255, 255,
             255, 0, 255, 255, 255, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14,
             15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 255, 255, 255, 255, 255, 255, 26, 27, 28,
             29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48,
             49, 50, 51, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
             255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
             255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
             255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
             255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
             255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
             255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255};

        for (; i < src_len; i += 4) {
                dst[j++] = base64_decode_map[src[i]] << 2 |
                        base64_decode_map[src[i + 1]] >> 4;
                dst[j++] = base64_decode_map[src[i + 1]] << 4 |
                        base64_decode_map[src[i + 2]] >> 2;
                dst[j++] = base64_decode_map[src[i + 2]] << 6 |
                        base64_decode_map[src[i + 3]];
        }

        dst[j] = '\0';
}


void base64_encode(const char *src, int src_len, std::string &dst)
{
   if(src_len <= 0)
   {
       return;
   }
   int dstlen = (src_len+2)/3 << 2;
   char *pdst = new char[dstlen+1];
   pdst[dstlen] = 0;
   base64_encode(src, src_len, pdst);
   dst = pdst;
   delete[] pdst;
}

void base64_decode(const char *src, int src_len, std::string &dst)
{
    if(src_len <= 0)
    {
        return;
    }
    int dstlen = (src_len >> 2) * 3;
    char *pdst = new char[dstlen + 1];
    base64_decode(src, src_len, pdst);
    pdst[dstlen] = 0;
    dst = pdst;
    delete[] pdst;
}

}
}




