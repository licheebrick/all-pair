//
// Created by mac on 2018/3/9.
//

#include "network.h"

Reachability Network::rmatrix[router_max][router_max];
Reachability Network::rmatrix1[router_max][router_max];
Reachability Network::rmatrix2[router_max][router_max];

Network::Network()
{
    r_num = 1000;
}

Network::Network(int num)
{
    r_num = num;
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
    //先算一个基本的r0，然后推导到rk
    //后面可以有一个强行减小矩阵的优化
    for(int i = 0; i < r_num; i++)
    {
        std::map< uint64_t, std::set<uint64_t> >::iterator it;
        it = routers[i].port_to_match.begin();
        while(it != routers[i].port_to_match.end())
        {
            uint64_t port_num = it->first;
            uint64_t next_port_num = topology[port_num];
            uint32_t router1 = port_to_router[port_num];
            uint32_t router2 = port_to_router[next_port_num];
            rmatrix[router1][router2].set_rules(it->second);
            it ++;    
        }
    }
    
    //Todo:进行一个矩阵变换
    
    for(int i = 0; i < r_num; i++)
    {
        for(int j = 0; j < r_num; j++)
        {
            rmatrix2[i][j] = rmatrix[i][j];
        }
    }
    for(int k = 1; k <= r_num; k++)
    {
        if(k % 2 == 1)
        {
            for(int i = 0; i < r_num; i++)
            {
                for(int j = 0; j < r_num; j++)
                {
                    rmatrix1[i][j] = rmatrix2[i][j] + (rmatrix2[i][k] * rmatrix2[k][j]);
                }
            }
        }
        else if(k % 2 == 0)
        {
            for(int i = 0; i < r_num; i++)
            {
                for(int j = 0; j < r_num; j++)
                {
                    rmatrix2[i][j] = rmatrix1[i][j] + (rmatrix1[i][k] * rmatrix1[k][j]);
                }
            }
        }
    }

    if(r_num % 2 == 1)
    {
        for(int i = 0; i < r_num; i++)
        {
            for(int j = 0; j < r_num; j++)
            {
                printf("matrix: %d : %d \n", i, j);
                rmatrix1[i][j].show_rules();
            }
        }
    }
    else if(r_num % 2 == 0)
    {
        for(int i = 0; i < r_num; i++)
        {
            for(int j = 0; j < r_num; j++)
            {
                printf("matrix: %d : %d \n", i, j);
                rmatrix2[i][j].show_rules();
            }
        }
    }
}