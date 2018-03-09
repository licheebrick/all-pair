//
// Created by mac on 2018/3/9.
//

#include "network.h"

Network::Network()
{

}

Network::~Network()
{
    // clear topology
    topology.clear();

    //clean port_to_router
    port_to_router.clear();
}

void Network::init()
{
    //这块得等学姐的接口了
    uint64_t port1[] = {1,2};
    uint64_t port2[] = {2,3};
    uint64_t port4[] = {1,4};
    uint64_t port5[] = {1,4};
    uint64_t port8[] = {1,3};

    std::set<uint64_t> portset1 (port1, port1 + 2);
    routers[0].port_to_match[1] = portset1;
    std::set<uint64_t> portset2 (port2, port2 + 2);
    routers[0].port_to_match[2] = portset2;
    std::set<uint64_t> portset4 (port4, port4 + 2);
    routers[1].port_to_match[4] = portset4;
    std::set<uint64_t> portset5 (port5, port5 + 2);
    routers[1].port_to_match[5] = portset5;
    std::set<uint64_t> portset8 (port8, port8 + 2);
    routers[2].port_to_match[8] = portset8;

    Reachability reach1;
    Reachability reach2;
    Reachability reach3;
    reach3 = reach1 * reach2;
    reach3 = reach1 + reach2;

    printf("finish writing rules!!\n");
    
    //write port_to_router
    add_port_to_router(1,0);
    add_port_to_router(2,0);
    add_port_to_router(3,1);
    add_port_to_router(4,1);
    add_port_to_router(5,1);
    add_port_to_router(6,2);
    add_port_to_router(7,2);
    add_port_to_router(8,2);
    add_port_to_router(9,3);
    add_port_to_router(10,3);

    printf("finish initializing the network!!\n");
}

void Network::add_link(uint64_t from_port, uint64_t to_port)
{
    topology[from_port] = to_port;
}

void Network::print_topology()
{
    printf("topology: \n");
    std::map<uint64_t, uint64_t>::iterator it;
    it = topology.begin();
    while(it !=topology.end())
    {
        printf("from %llu to %llu\n", it->first, it->second);
        it ++;         
    }
}

void Network::add_port_to_router(uint64_t port, uint32_t router)
{
    port_to_router[port] = router;
}

void Network::print_port_to_router()
{
    printf("port_to_router: \n");
    std::map<uint64_t, uint32_t>::iterator it;
    it = port_to_router.begin();
    while(it !=port_to_router.end())
    {
        printf("from port %lu to router %lu\n", it->first, it->second);
        it ++;         
    }
}

void Network::all_pair_reachability()
{
    //Todo: finish warshall
}