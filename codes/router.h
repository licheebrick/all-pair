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
#include <sys/time.h>

using namespace std;

class Router
{
public:
    Router();
    ~Router();
    void routers_init(uint32_t id);

    void set_router_id(uint32_t id) {router_id=id;}
    uint32_t getid(){return router_id;}

    void print_port_to_match();

    // map<port_id, match>
    std::map< uint64_t, std::set<uint64_t>* > port_to_match;
private:
    uint32_t router_id;
};

#endif //WARSHALL_NO_PATH_ROUTER_H
