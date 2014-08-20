#ifndef MD5_H
#define MD5_H

#include <string>
#include <fstream>


namespace comm
{
namespace util
{
class MD5Sum_test
{
protected:
    unsigned long m_state[4];
    unsigned long m_count[2];
    unsigned char m_buf[64];
    unsigned int  m_bpos;
    bool          m_committed;
    unsigned char m_md5[16];

    void init();
    void update();
    void commit();
    bool isCommitted() const;

public:
    MD5Sum_test();
    MD5Sum_test(const char* sum);

    void put(const char* buf, unsigned int size);

    operator const unsigned char*();
    std::string toString();
    std::string toTempString();

    bool operator==(const MD5Sum_test& sum);

    static std::string toString(const unsigned char* md5);
};

bool getFileMd5Value(const char *fileName,std::string &md5Ret);

std::string getStringMD5Value(const char *str);

void Md5Sum(char *output, const void *input, unsigned int len);

}
}


#endif /*MD5_H*/
