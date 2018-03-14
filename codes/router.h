//
// Created by mac on 2018/3/9.
//

#ifndef WARSHALL_NO_PATH_ROUTER_H
#define WARSHALL_NO_PATH_ROUTER_H

#include <stdint.h>
#include <iostream>
#include <map>
#include <vector>
#include <set>
using namespace std;

class Router
{
public:
    Router();
    Router(uint32_t id);
    ~Router();
    void routers_init(uint32_t id);
    uint32_t getid(){return router_id;}
    void print_port_to_patch();
    
    //map<port_id, match>
    std::map< uint64_t, std::set<uint64_t>* > port_to_match;
private:
    uint32_t router_id;
};

#endif //WARSHALL_NO_PATH_ROUTER_H
