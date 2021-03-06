//
// Created by mac on 2018/3/19.
//

#include "rulebased.h"

Rulebased::Rulebased()
{

}

Rulebased::~Rulebased()
{

}

void Rulebased::set_new_rule(string list_str, std::list<uint32_t>* tmp_list, uint64_t rule)
{
    std::hash<std::string> hash_fn;
    uint64_t hash_string = hash_fn(list_str);

    std::map<uint64_t, std::map<std::list<uint32_t>*, std::set<uint64_t>* >* >::iterator it;
    it = rule_map.find(hash_string);
    if(it != rule_map.end())//already have it
        (*((*(rule_map[hash_string])).begin())->second).insert(rule);
    else
    {
        std::map<std::list<uint32_t>*, std::set<uint64_t>* >* new_map;
        new_map = new std::map<std::list<uint32_t>*, std::set<uint64_t>* >;
        std::list<uint32_t>* new_list;
        new_list = new std::list<uint32_t>;
        *new_list = *tmp_list;
        std::set<uint64_t>* new_set;
        new_set = new std::set<uint64_t>;
        (*new_set).insert(rule);
        (*new_map)[new_list] = new_set;
        rule_map[hash_string] = new_map;
    }
}

void Rulebased::print_rule_map()
{
    std::map<uint64_t, std::map<std::list<uint32_t>*, std::set<uint64_t>* >* >::iterator it;

    for(it = rule_map.begin(); it != rule_map.end(); it++)
    {
        std::map<std::list<uint32_t>*, std::set<uint64_t>* > tmp = *it->second;
        std::map<std::list<uint32_t>*, std::set<uint64_t>* >::iterator iter;
        for(iter = tmp.begin(); iter != tmp.end(); iter++)
        {
            printf("One path found: [");
            //printf("This path includes router: ");
            std::list<uint32_t> tmp = *iter->first;
            std::list<uint32_t>::iterator it1;
            for (it1 = tmp.begin(); it1 != tmp.end(); ++it1)
                printf("%u -> ", *it1);

            printf("] with header: ");
            std::set<uint64_t> tmp2 = *iter->second;
            std::set<uint64_t>::iterator it2;
            for(it2 = tmp2.begin(); it2 != tmp2.end(); ++it2)
                printf("%llu ", *it2);
            printf("\n");
        }
    }
}

void Rulebased::clean_up()
{
    std::map<uint64_t, std::map<std::list<uint32_t>*, std::set<uint64_t>* >* >::iterator it;

    for(it = rule_map.begin(); it != rule_map.end(); it++)
    {
        std::map<std::list<uint32_t>*, std::set<uint64_t>* > tmp = *it->second;
        std::map<std::list<uint32_t>*, std::set<uint64_t>* >::iterator iter;
        for(iter = tmp.begin(); iter != tmp.end(); iter++)
        {
            (*iter->first).clear();
            (*iter->second).clear();
            delete iter->first;
            delete iter->second;
        }
        (*it->second).clear();
        delete it->second;
    }
    rule_map.clear();
}