//
// Created by mac on 2018/3/9.
//

#ifndef WARSHALL_NO_PATH_NETWORK_H
#define WARSHALL_NO_PATH_NETWORK_H

#include "utils.h"
#include "router.h"
#include "reachability.h"
#include "rulebased.h"
#include <stdint.h>
#include <iostream>
#include <map>
#include <vector>
#include <set>
#include <json/json.h>
#include <bdd.h>
#include <string>

using namespace std;

const uint32_t router_max = 1000;//total
const uint64_t rule_type = 4;   //假设有10种流量

class Network
{
public:
    Network();
    Network(int num);
    ~Network();

    void init();
    void set_hdr_len(int len) {hdr_len = len;}
    void add_link(uint64_t from_port, uint64_t to_port);
    void print_topology();

    void add_port_to_router(uint64_t port, int router);
    void print_port_to_router();
    void add_then_load_router(uint32_t router_id, Json::Value *root);

    // pre-process rules to number set
    void make_atomic_predicates();
    void convert_router_to_ap();

    // all pair reachability:
    void brutal_force();
    void dfs_search(int router, int destiny, std::set<uint64_t>* rules);
    void display_result(std::set<uint64_t>* rules);

    void warshall_with_path();

    void segment_based();

    void rule_based();
private:
    // map <port_id, another_port_id>
    std::map<uint64_t, uint64_t> topology;

    // map <port_id, router_number>
    std::map< uint64_t, int> port_to_router;

    // list to place atomic predicates represented in bdd;
    vector< bdd >* ap_bdd_list;

    // for dfs search
    bool have_been[router_max];

    int router_stack[router_max];
    int stack_place;

    // TODO:: use Router* ?
    Router routers[router_max];

    // for pre-processing rules
    bdd dealt_bdd[router_max];
    map< uint64_t, bdd > *predicate_map[router_max];

    // the number of routers
    int r_num;

    // header length
    int hdr_len;

    static Reachability rmatrix[router_max][router_max];
    static Reachability rmatrix1[router_max][router_max];
    static Reachability rmatrix2[router_max][router_max];
    static Reachability rmatrix3[router_max][router_max];
}; 

#endif //WARSHALL_NO_PATH_NETWORK_H
