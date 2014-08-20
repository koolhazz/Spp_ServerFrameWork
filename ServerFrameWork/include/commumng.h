/***************************************
*文件名:commumng.h
*功能:后端通讯连接管理
*作者:钟何明
*创建时间:2009.06.11
***************************************/

#ifndef _COMMU_MNG_H_
#define _COMMU_MNG_H_
#include <string>
#include <vector>
#include <map>
#include <assert.h>

using namespace std;

namespace comm
{
namespace sockcommu
{

//后端通讯管理
class CCommuMng
{
public:	
    typedef std::map<unsigned int,unsigned int> NodeList;//nodeid->flow
    typedef NodeList::iterator NodeListIter;

    typedef std::map<unsigned int,NodeList *> GroupList;//serv_type,Node list
    typedef GroupList::iterator GroupListIter;

    CCommuMng();
    ~CCommuMng();

    //新增一个连接
    //serv_type:服务类型
    //node_id:节点ID
    //flow:连接句柄
    //返回值:0:成功，否则失败
    int AddConn(unsigned int serv_type,unsigned node_id,unsigned int flow);

    //移除连接
    //flow:连接的唯一标记
    //返回值:成功:0,否则失败
    int RMConn(unsigned int flow);

    //获取连接句柄
    //serv_type:服务类型
    //node_id:节点ID
    //返回值:
    //0:失败,>0表示连接句柄flow
    unsigned int GetFlow(unsigned int serv_type,unsigned int node_id);


    //通过flow取得关联的server_type
    //[out] server_type:服务类型
    //[in] flow:通讯唯一标识
    //返回值:
    //0失败，>0表示与flow关联的server_type
    unsigned int GetServerTypeByFlow(unsigned int flow);


    /*
    *添加断开连接的节点信息
    *[in] serv_type:服务类型
    *[in] node_id:节点ID
    */
    //int AddDropItem(unsigned int serv_type,unsigned int node_id);

    /*
    *移除断开连接的节点信息
    *[in] serv_type:服务类型
    *[in] node_id:节点ID
    */
    //int RMDropItem(unsigned int serv_type,unsigned int node_id);

    //int GetDropNodeList(std::vector<unsigned int > & NodeIDList,unsigned int serv_type);

    //int GetDropServerList(std::vector<unsigned int> & ServerIDList);

protected:

    GroupList grp_list_;//连接句柄列表	
    //GroupList drop_conn_list_;//断开的连接列表

};

}
}


#endif

