
#ifndef TEA_H_INCLUDED
#define TEA_H_INCLUDED

namespace comm
{
namespace util
{
void tea_encrypt(unsigned int *v0, unsigned int *v1, unsigned int k0, 
    unsigned int k1, unsigned int k2, unsigned int k3);

/*  
    一次加密多个Byte, 非8的整数倍的部分不加密.
*/
void tea_encrypt(char * v, unsigned int len, unsigned int k0, 
    unsigned int k1, unsigned int k2, unsigned int k3);

void tea_decrypt(unsigned int* v0, unsigned int *v1, unsigned int k0,
    unsigned int k1, unsigned int k2, unsigned int k3);

void tea_decrypt(char* v, unsigned int len, unsigned int k0,
    unsigned int k1, unsigned int k2, unsigned int k3);
	
void TEA_Encrypt(void *src, unsigned int src_len,const void * key,unsigned int key_len)
{
    unsigned int *p = (unsigned int *)key;
    tea_encrypt((char *)src, src_len, p[0],p[1],p[2],p[3]);
}

void TEA_Decrype(void *src, unsigned int src_len, const void *key, unsigned int key_len)
{
    unsigned int *p = (unsigned int *)key;
    tea_decrypt((char *)src, src_len, p[0],p[1],p[2],p[3]);
}

}
}

#endif // TEA_H_INCLUDED
