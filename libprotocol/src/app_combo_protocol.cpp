
#include "app_combo_protocol.h"

using namespace protocol;

int AppComboRsp::ParseRsp(const char * buf, unsigned int buf_len)
{
    UnPacker upk;
    int ret = upk.Init(buf, buf_len);
    if(RET_OK != (unsigned int )ret)
    {
        return ret;
    }
    
    unsigned short retcode = upk.UnPackWord();
    if(retcode != 0)
    {    
        m_RetCode = DP_ERR_GENERAL;
        return -1;
    }
    
    m_RetCode = 0;
    m_RecordNum = upk.UnPackWord();
    unsigned int len = 0;
    const char *pRsp = 0;
    unsigned int cmd = 0;

    for(unsigned int i = 0; i < m_RecordNum; i++)
    {
        len = upk.UnPackDWord();
        pRsp = upk.UnPackBinary(len);
        if(!pRsp)
        {
            m_RetCode = DP_ERR_GENERAL;
            m_RecordNum = m_ComboRsp.size();
            return -1;
        }
        unsigned short main_cmd = 0;
        unsigned short sub_cmd = 0;
        int r = GetRspCmd(main_cmd, sub_cmd, pRsp, len);
        if(r != 0)
        {
            m_RetCode = DP_ERR_GENERAL;
            m_RecordNum = m_ComboRsp.size();
            return -1;
        }

        cmd = main_cmd;
        cmd = (cmd<<16) | sub_cmd;
        
        m_ComboRsp.push_back(std::make_pair<string, unsigned int>(string(pRsp,len), cmd));
    }

    return 0;
    
}


int AppComboRsp::GetSubRsp(char * buf, unsigned int buf_len, unsigned short idx)
{
    if(idx >= m_RecordNum)
    {
        return -1;
    }
    
    item_type &t = m_ComboRsp[idx];
    string & rsp = t.first;
    size_t rsp_len = rsp.size();
    if(buf_len < rsp_len )
        return -2;
    memcpy(buf,rsp.data(),rsp_len);
    return (int)rsp_len;
}

int AppComboRsp::GetSubRsp(string& subpack,unsigned short idx)
{
    if(idx >= m_RecordNum)
    {
        return -1;
    }
    
    item_type &t = m_ComboRsp[idx];
    subpack = t.first;
    return (int)subpack.size();
}

int AppComboRsp::GetSubRspCmd(unsigned short & main_cmd, unsigned short & sub_cmd, unsigned short idx)
{
    if(idx >= m_RecordNum)
    {
        return -1;
    }
    
    item_type &t = m_ComboRsp[idx];
    unsigned int cmd = t.second;
    main_cmd = cmd>>16;
    sub_cmd = cmd;
    return 0;
}

