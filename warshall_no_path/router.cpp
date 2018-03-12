//
// Created by mac on 2018/3/9.
//

#include "router.h" 

Router::Router()
{

}

Router::Router(uint32_t id)
{
    router_id = id;
}

Router::~Router()
{
    // clear port_to_match
    std::map< uint64_t, std::set<uint64_t> >::iterator it;
    for (it = port_to_match.begin(); it != port_to_match.end(); it++) 
    {
        port_to_match[it->first].clear();
    }
    port_to_match.clear();
}

void Router::routers_init(uint32_t id)//用来暴力初始化router的
{
    router_id = id;
}