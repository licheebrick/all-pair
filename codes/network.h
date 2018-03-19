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
const int rule_type = 10;   //假设有10种流量

class Network
{
public:
    Network();
    Network(int num);
    ~Network();

    void init();
    void add_link(uint64_t from_port, uint64_t to_port);
    void print_topology();
    void add_port_to_router(uint64_t port, uint32_t router);
    void print_port_to_router();

    //all pair reachability:
    void brutal_force();
    void dfs_search(uint32_t router, uint32_t destiny, std::set<uint64_t>* rules);
    void display_result(std::set<uint64_t>* rules);

    void warshall_with_path();

    void segment_based();
private:
    //map <port_id, another_port_id>
    std::map<uint64_t, uint64_t> topology;

    //map <port_id, router_id>
    std::map< uint64_t, uint32_t> port_to_router;

    //for dfs search
    bool have_been[router_max];

    uint32_t router_stack[router_max];
    uint32_t stack_place;
    Router routers[router_max];

    int r_num;//the number of routers

    static Reachability rmatrix[router_max][router_max];
    static Reachability rmatrix1[router_max][router_max];
    static Reachability rmatrix2[router_max][router_max];
    static Reachability rmatrix3[router_max][router_max];
}; 

#endif //WARSHALL_NO_PATH_NETWORK_H
