//
// Created by mac on 2018/3/19.
//

#ifndef WARSHALL_NO_PATH_RULEBASED_H
#define WARSHALL_NO_PATH_RULEBASED_H

#include <stdint.h>
#include <iostream>
#include <map>
#include <vector>
#include <set>
#include <list>
#include <functional>
using namespace std;

class Rulebased
{
public:
    Rulebased();
    ~Rulebased();

    void set_new_rule(string list_str, std::list<int>* tmp_list, uint64_t rule);
    void print_rule_map();

    std::hash< std::string > list_string;
    std::map<uint32_t, std::map<std::list<int>*, std::set<uint64_t>* >* > rule_map;
};

#endif //WARSHALL_NO_PATH_RULEBASED_H
