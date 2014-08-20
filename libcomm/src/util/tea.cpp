
#include "tea.h"

namespace comm
{
namespace util
{
/*
    标准TEA算法，key是128bit, (16Byte)
    一次加密8Byte.
*/
void tea_encrypt(unsigned int *v0, unsigned int *v1, unsigned int k0, 
    unsigned int k1, unsigned int k2, unsigned int k3)
{
     unsigned int sum=0;
     unsigned int i = 0;
     unsigned int delta = 0x9e3779b9;

     for (i=0; i < 32; i++)
    {
         sum += delta;
         *v0 += ((*v1<<4) + k0) ^ (*v1 + sum) ^ ((*v1>>5) + k1);
         *v1 += ((*v0<<4) + k2) ^ (*v0 + sum) ^ ((*v0>>5) + k3);
     }
 }

/*  
    一次加密多个Byte, 非8的整数倍的部分不加密.
*/
void tea_encrypt(char * v, unsigned int len, unsigned int k0, 
    unsigned int k1, unsigned int k2, unsigned int k3)
{
    while( len >= 8 )
    {
            tea_encrypt((unsigned int *)v,(unsigned int *)(v+4), k0,k1,k2,k3);
            v += 8;
            len -= 8;
    }
}





void tea_decrypt(unsigned int* v0, unsigned int *v1, unsigned int k0,
    unsigned int k1, unsigned int k2, unsigned int k3)
{
     unsigned int sum=0xC6EF3720;
     unsigned int i = 0;
     unsigned int delta = 0x9e3779b9;
     for(i=0; i<32; i++)
     {
         *v1 -= ((*v0<<4) + k2) ^ (*v0 + sum) ^ ((*v0>>5) + k3);
         *v0 -= ((*v1<<4) + k0) ^ (*v1 + sum) ^ ((*v1>>5) + k1);
         sum -= delta;
     }
 }

void tea_decrypt(char* v, unsigned int len, unsigned int k0,
    unsigned int k1, unsigned int k2, unsigned int k3)
{
    while( len >= 8 )
    {
            tea_decrypt((unsigned int *)v, (unsigned int *)(v+4), k0, k1, k2, k3);
            v += 8;
            len -= 8;
    }
}


/*
int test_main(int argc,char **argv)
{
        if(argc != 6)
        {
                printf("Usage:%s src k0 k1 k2 k3\n", argv[0]);
                return 1;
        }

        char src[1024] = {0};
        strcpy(src, argv[1]);
        unsigned int k0 = atoi(argv[2]);
        unsigned int k1 = atoi(argv[3]);;
        unsigned int k2 = atoi(argv[4]);
        unsigned int k3 = atoi(argv[5]);

        int srclen = strlen(src);
        printf("src = %s\nkey=%d,%d,%d,%d\n", src, k0,k1,k2, k3);
        tea_encrypt(src, srclen, k0,k1,k2,k3);
        printf("After Encrypt=%s\n", src);

        tea_decrypt(src, srclen, k0,k1,k2,k3);

        printf("After Decrypt=%s\n", src);
        return 0;
}


*/

}
}



