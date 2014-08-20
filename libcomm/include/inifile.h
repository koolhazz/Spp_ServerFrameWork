#ifndef __COMM_BASE_INIFILE_H__
#define __COMM_BASE_INIFILE_H__

#include <memory.h>


namespace comm
{
namespace base
{


typedef unsigned char BYTE;

class CIniFile
{
private:
    char    *m_pszContent;      /* 配置文件的原始内容 */
    char    *m_pszShadow;       /* 配置文件的内容全部转换成小写 */
    size_t  m_nSize;            /* 配置文件内容的长度，不包括最后的NULL */
    short   m_bIsOpen;          /* 配置文件是否打开成功的标志 */

public:
    char    *m_pszFilename;     /* 存放需要读取的配置文件名 */
    CIniFile();  
    ~CIniFile();

    /*************************************************
      Function:     IsOpen
      Description:
            返回读取配置文件是否成功的标志
      Calls:
      Called By:    CConfigFile::GetItemValue,
                    CConfigFile::uT_main
      Input:
      Output:
      Return:       如果配置文件读取成功，返回true，否则返回false
      Others:
    *************************************************/
    unsigned int IsOpen();



    /*************************************************
      Function:     GetItemValue字符串
      Description:
            从内存缓冲区中找到KeyName，将值拷贝到指定的空间。
            如果返回值大于空间的大小，则对字符串进行截尾处理，
        并在缓冲区的最后一个字节加上NULL。
            当缓冲区的最后两个字符是汉字编码时，将自动加上两个结束符
      Calls:        CConfigFile::IsOpen,
                    CConfigFile::LocateKey,
                    CConfigFile::LocateSection
      Called By:    CConfigFile::uT_main
      Input:
            pszSectionName  - 以NULL结尾的字符串指针，指示包含Key的片断
            pszKeyName      - 以NULL结尾的字符串指针，指示需要返回值的Key
            nSize           - 指定接收缓冲区的大小
      Output:
            pszReturnedString - 指定用于接收结果的缓冲区地址
      Return:       返回缓冲区中的有效字符个数。不包括字符串结尾的NULL
      Others:       本函数不是UNICODE版本
    *************************************************/
    unsigned int GetItemValue( const char *pszSectionName, 
                        const char *pszKeyName, 
                        char *pszReturnedString, 
                        unsigned int nSize );



    /*************************************************
      Function:     SetItemValue数字
      Description:
            将指定的数字类型键值，并将结果同时更新内存和配置文件
      Calls:        CConfigFile::SetItemValue字符串
      Called By:    CConfigFile::uT_main
      Input:
            pszSectionName  - 以NULL结尾的字符串指针，指示包含Key的片断
            pszKeyName      - 以NULL结尾的字符串指针，指示需要设置的Key
            ulKeyValue      - 数字类型，指示需要设置的值
      Output:
      Return:       返回是否成功的标志。成功，返回true；否则返回false
      Others:       本函数不是UNICODE版本
    *************************************************/
    unsigned int SetItemValue( const char *pszSectionName, 
                        const char *pszKeyName, 
                        const char *pszKeyValue );




     /*************************************************
      Function:     GetItemValue数字
      Description:
            从内存中取指定的数字类型的键值
      Calls:        CConfigFile::GetItemValue字符串
      Called By:    CConfigFile::uT_main
      Input:
            pszSectionName  - 以NULL结尾的字符串指针，指示包含Key的片断
            pszKeyName      - 以NULL结尾的字符串指针，指示需要返回值的Key
      Output:
            ulReturnedValue - 指定用于接收结果的缓冲区地址
      Return:       成功返回true, 失败返回false
      Others:       本函数不是UNICODE版本
    *************************************************/
    unsigned int GetItemValue( const char *pszSectionName, 
                        const char *pszKeyName, 
                        int &lReturnedValue );


    /*************************************************
      Function:     SetItemValue数字
      Description:
            将指定的数字类型键值，并将结果同时更新内存和配置文件
      Calls:        CConfigFile::SetItemValue字符串
      Called By:    CConfigFile::uT_main
      Input:
            pszSectionName  - 以NULL结尾的字符串指针，指示包含Key的片断
            pszKeyName      - 以NULL结尾的字符串指针，指示需要设置的Key
            ulKeyValue      - 数字类型，指示需要设置的值
      Output:
      Return:       返回是否成功的标志。成功，返回true；否则返回false
      Others:       本函数不是UNICODE版本
    *************************************************/
    unsigned int SetItemValue( const char *pszSectionName, 
                        const char *pszKeyName, 
                        int lKeyValue );

    /*************************************************
      Function:     GetItemValue数字
      Description:
            从内存中取指定的数字类型的键值，如果不存在，则使用指定的缺省值
      Calls:        CConfigFile::GetItemValue字符串
      Called By:    CConfigFile::uT_main
      Input:
            pszSectionName  - 以NULL结尾的字符串指针，指示包含Key的片断
            pszKeyName      - 以NULL结尾的字符串指针，指示需要返回值的Key
            lDefaultValue   - 取值失败后使用的缺省值
      Output:
            ulReturnedValue - 指定用于接收结果的缓冲区地址
      Return:       成功返回true, 失败返回false
      Others:       本函数不是UNICODE版本
    *************************************************/
    unsigned int GetItemValue( const char *pszSectionName, 
                        const char *pszKeyName, 
                        int &lReturnedValue,
                        int lDefaultValue );


     /*************************************************
      Function:     GetItemValue字符串
      Description:
            从内存中取指定的字符串类型的键值，如果不存在，则使用指定的缺省值
      Calls:        CConfigFile::GetItemValue字符串
      Called By:    CConfigFile::uT_main
      Input:
            pszSectionName  - 以NULL结尾的字符串指针，指示包含Key的片断
            pszKeyName      - 以NULL结尾的字符串指针，指示需要返回值的Key
            nSize           - 指定接收缓冲区的大小
            pszDefaultValue - 取值失败后使用的缺省值
      Output:
            pszReturnedString - 指定用于接收结果的缓冲区地址
      Return:       返回缓冲区中的有效字符个数。不包括字符串结尾的NULL
      Others:       本函数不是UNICODE版本
    *************************************************/
    unsigned int GetItemValue( const char *pszSectionName, 
                        const char *pszKeyName, 
                        char *pszReturnedString, 
                        unsigned int nSize,
                        const char *pszDefaultValue );

    /*************************************************
      Function:     OpenFile
      Description:
            读取指定的配置文件。
            如果文件读取成功，设置m_bIsOpen为true
      Calls:
      Called By:    CConfigFile::CConfigFile
      Input:
            pszFilename     - 以NULL结尾的配置文件名
      Output:
      Return:成功:0,否则失败
      Others:
    *************************************************/
    int  OpenFile(const char *pszFilename);

    void CloseFile();

private:
    
    unsigned int LocateSection(const char *pszSectionName, 
                        char * &pszSectionBegin, 
                        char * &pszSectionEnd);
    unsigned int LocateKeyRange(const char *pszKeyName, 
                        const char *pszSectionBegin, 
                        const char *pszSectionEnd, 
                        char * &pszKeyBegin, 
                        char * &pszKeyEnd);
    unsigned int LocateKeyValue(const char *pszKeyName, 
                        const char *pszSectionBegin, 
                        const char *pszSectionEnd, 
                        char * &pszValueBegin, 
                        char * &pszValueEnd);
    char *LocateStr(    const char *pszCharSet, 
                        const char *pszBegin, 
                        const char *pszEnd );
    char *SearchMarchStr(const char *pszBegin, const char *pszCharSet);

    char *MapToContent(const char *p);
    char *MapToShadow(const char *p);

    void ToLower( char * pszSrc, size_t len);

};

}
}
#endif

