//
// Created by mac on 2018/3/9.
//

#ifndef WARSHALL_NO_PATH_NETWORK_H
#define WARSHALL_NO_PATH_NETWORK_H

#include "router.h"
#include "reachability.h"
#include <stdint.h>
#include <iostream>
#include <map>
#include <vector>
#include <set>
using namespace std;

const int router_max = 1000;//total
const int router_num = 4;   //routers num
const int rule_type = 10;   //假设有10种流量

class Network
{
public:
    Network();
    ~Network();

    void init();
    void add_link(uint64_t from_port, uint64_t to_port);
    void print_topology();
    void add_port_to_router(uint64_t port, uint32_t router);
    void print_port_to_router();

    void all_pair_reachability();
private:
    //map <port_id, another_port_id>
    std::map<uint64_t, uint64_t> topology;

    //map <port_id, router_id>
    std::map< uint64_t, uint32_t> port_to_router;

    //for dfs search
    bool have_been[router_max] = {false};

    uint32_t router_stack[router_max] = {999999};
    uint32_t stack_place = 0;
    Router routers[router_max];
}; 

#endif //WARSHALL_NO_PATH_NETWORK_H
