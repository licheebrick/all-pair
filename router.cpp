#include "router.h" 

void Router::routers_init(uint32_t id)//用来暴力初始化router的
{
    router_id = id;
}

void Router::update_port_to_match(std::map< uint64_t, std::set<uint64_t> > new_one)
{
    port_to_match = new_one;
}

void Router::update_port_to_match(uint64_t port, std::set<uint64_t> match)
{
    port_to_match[port] = match;
}

std::map< uint64_t, std::set<uint64_t> > Router::get_port_to_match()
{
    return port_to_match;
}