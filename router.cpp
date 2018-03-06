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

// void Router::update_port_to_match(std::map< uint64_t, std::set<uint64_t> > new_one)
// {
//     port_to_match = new_one;
// }

// void Router::update_port_to_match(uint64_t port, std::set<uint64_t> match)
// {
//     port_to_match[port] = match;
// }

// std::map< uint64_t, std::set<uint64_t> > Router::get_port_to_match()
// {
//     return port_to_match;
// }