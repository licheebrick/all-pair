//
// Created by mac on 2018/3/9.
//

#ifndef WARSHALL_NO_PATH_REACHABLILITY_H
#define WARSHALL_NO_PATH_REACHABLILITY_H

#include <stdint.h>
#include <iostream>
#include <map>
#include <vector>
#include <set>
#include <list>
using namespace std;

class Reachability
{
public:
    Reachability();
    ~Reachability();

    void set_path_to_packets(std::list<uint32_t> set_list, std::set<uint64_t> set_rules);
    void show_path_to_packets();

    Reachability operator * (Reachability &reach);
    Reachability operator + (Reachability &reach);
private:
    std::map<std::list<uint32_t>, std::set<uint64_t> > path_to_packets;
};

#endif //WARSHALL_NO_PATH_REACHABLILITY_H
