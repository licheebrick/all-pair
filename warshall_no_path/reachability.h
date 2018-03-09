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
using namespace std;

class Reachability
{
public:
    Reachability();
    ~Reachability();

    void set_rules(std::set<uint64_t> set_rules);
    std::set<uint64_t> get_rules();

    Reachability operator * (const Reachability &reach);
    Reachability operator + (const Reachability &reach);
private:
    std::set<uint64_t> rules;
};

#endif //WARSHALL_NO_PATH_REACHABLILITY_H
