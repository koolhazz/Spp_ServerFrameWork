
#include "commumng.h"

namespace comm
{
namespace sockcommu
{
CCommuMng::CCommuMng()
{
}

CCommuMng::~CCommuMng()
{
    GroupListIter grp_itr = grp_list_.begin(),grp_last = grp_list_.end();
    for(;grp_itr != grp_last;grp_itr ++ )
    {
        NodeList * nodelist = grp_itr->second;
        if(nodelist != NULL )
        {			
            delete grp_itr->second;
        }
    }

    grp_list_.clear();

/*
    GroupListIter drop_itr = drop_conn_list_.begin(),drop_last = drop_conn_list_.end();
    for(;drop_itr != drop_last;++drop_itr)
    {
        NodeList * nodelist = drop_itr->second;
        if(nodelist != NULL )
        {
            delete nodelist;
        }
    }

    drop_conn_list_.clear();
    */

}

int CCommuMng::AddConn(unsigned int serv_type, unsigned int node_id, unsigned int flow)
{
#ifdef OPEN_PRINT
    printf("%s:serv_type=%u,node_id=%u,flow=%u\n",__FUNCTION__,serv_type,node_id,flow);
#endif

    GroupListIter grp_itr = grp_list_.find(serv_type);
    NodeList * nodelist = NULL;	
    if(grp_itr != grp_list_.end() )
    {
        nodelist = grp_itr->second;
        if(nodelist != NULL )
        {
            //RMDropItem(serv_type,node_id); //从断开连接列表中移除
            (*nodelist)[node_id] = flow;
            return 0;//OK
        }		
    }


    nodelist = new NodeList;	
    assert(nodelist != NULL );

    grp_list_[serv_type] = nodelist;//add group
    (*nodelist)[node_id] = flow;

    //RMDropItem(serv_type,node_id);//从断开连接列表中移除
    return 0;//OK
}

unsigned int CCommuMng::GetFlow(unsigned int serv_type, unsigned int node_id)
{
    GroupListIter grp_itr = grp_list_.find(serv_type);
    if(grp_itr != grp_list_.end())
    {
        NodeList * nodelist = grp_itr->second;
        if(nodelist == NULL )
        {
            return  0;//Node Not Exist
        }

        NodeListIter node_itr = nodelist->find(node_id);
        if(node_itr == nodelist->end() )
        {
            return  0;//Node Not Exist
        }
        return  node_itr->second;//ok
    }

    return 0;//error ,not found
}

int CCommuMng::RMConn(unsigned int flow)
{
#ifdef OPEN_PRINT
    printf("%s:flow=%u\n",__FUNCTION__,flow);
#endif
    GroupListIter grp_itr = grp_list_.begin(),grp_last = grp_list_.end();
    for(;grp_itr != grp_last;grp_itr ++ )
    {
        NodeList * nodelist = grp_itr->second;
        if(nodelist == NULL )
            continue;
        NodeListIter node_itr = nodelist->begin(),node_last = nodelist->end();
        for(;node_itr != node_last;node_itr ++ )
        {
            if(node_itr->second == flow )
            {
                //AddDropItem(grp_itr->first,node_itr->first);//加到断开连接列表
                nodelist->erase(node_itr);
                return 0;
            }
        }
    }
    return -1;//Not Exist
}

unsigned int CCommuMng::GetServerTypeByFlow(unsigned int flow)
{
    GroupListIter grp_itr = grp_list_.begin(),grp_last = grp_list_.end();
    for(;grp_itr != grp_last;grp_itr ++ )
    {
        NodeList * nodelist = grp_itr->second;
        if(nodelist == NULL )
            continue;
        NodeListIter node_itr = nodelist->begin(),node_last = nodelist->end();
        for(;node_itr != node_last;node_itr ++ )
        {
            if(flow == node_itr->second)
                return grp_itr->first;//OK,return serv_type
        }
    }

    return 0;//Not Exist
}

#if 0
int CCommuMng::AddDropItem(unsigned int serv_type, unsigned int node_id)
{
#ifdef OPEN_PRINT
    printf("%s:serv_type=%u,node_id=%u\n",__FUNCTION__,serv_type,node_id);
#endif
    GroupListIter grp_itr = drop_conn_list_.find(serv_type);
    NodeList * nodelist = NULL;	
    if(grp_itr != drop_conn_list_.end() )
    {
        nodelist = grp_itr->second;
        if(nodelist != NULL )
        {
            (*nodelist)[node_id] = 0;
            return 0;//OK
        }
    }

    nodelist = new NodeList;
    assert(nodelist != NULL );

    drop_conn_list_[serv_type] = nodelist;//add group
    (*nodelist)[node_id] = 0;
    return 0;//OK
}

int CCommuMng::RMDropItem(unsigned int serv_type, unsigned int node_id)
{
#ifdef OPEN_PRINT
        printf("%s:serv_type=%u,node_id=%u\n",__FUNCTION__,serv_type,node_id);
#endif
    GroupListIter grp_itr = drop_conn_list_.begin(),grp_last = drop_conn_list_.end();
    for(;grp_itr != grp_last; ++grp_itr )
    {
    	NodeList * nodelist = grp_itr->second;
    	if(nodelist != NULL )
    	{
    	    NodeListIter node_itr = nodelist->find(node_id);
    	    if(node_itr != nodelist->end() )
    	    {
    	        nodelist->erase(node_itr);
    	        return 0;
    	    }
    	}
    	
    }
    return -1;//Not Exist
}



int CCommuMng::GetDropNodeList( std::vector < unsigned int > & NodeIDList, unsigned int serv_type)
{
    GroupListIter grp_itr = drop_conn_list_.find(serv_type);
    if(grp_itr != drop_conn_list_.end() )
    {
        NodeList * nodelist = grp_itr->second;
        if(nodelist != NULL )
        {
            NodeListIter node_itr = nodelist->begin(),last = nodelist->end();
            for(;node_itr != last;++node_itr)
            {
                NodeIDList.push_back(node_itr->first);
            }
        }
        
    }

    return 0;
    
}

int CCommuMng::GetDropServerList( std::vector < unsigned int > & ServerIDList)
{
    GroupListIter grp_itr = drop_conn_list_.begin(),last = drop_conn_list_.end();
    for(;grp_itr != last; ++grp_itr)
    {
        ServerIDList.push_back(grp_itr->first);
    }

    return 0;
}

#endif
}
}

