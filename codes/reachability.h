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

// std::list<int> reach_list[100000];
// std::set<uint64_t> reach_set[100000];
// int r_place = 0;

class Reachability
{
public:
    Reachability();
    ~Reachability();

    void set_path_to_packets(std::list<int>* set_list, std::set<uint64_t>* set_rules);
    void show_path_to_packets();
    bool is_empty();
    void delete_all();

    Reachability operator * (Reachability &reach);
    Reachability operator + (Reachability &reach);
    //static int r_place;
private:
    std::map<std::list<int>*, std::set<uint64_t>* > path_to_packets;

    // static std::list<int> reach_list[100000];
    // static std::set<uint64_t> reach_set[100000];

};

#endif //WARSHALL_NO_PATH_REACHABLILITY_H
