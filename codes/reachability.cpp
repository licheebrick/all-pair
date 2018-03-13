//
// Created by mac on 2018/3/9.
//

#include "reachability.h"

Reachability::Reachability()
{

}

Reachability::~Reachability()
{

}

void Reachability::set_rules(std::set<uint64_t> set_rules)
{
    rules = set_rules;
}

void Reachability::set_path_to_rules(std::list<uint32_t> set_list, std::set<uint64_t> set_rules)
{
    path_to_rules[set_list] = set_rules;
}

std::set<uint64_t> Reachability::get_rules()
{
    return rules;
}

void Reachability::show_rules()
{
    std::map<std::list<uint32_t>, std::set<uint64_t> >::iterator iter = path_to_rules.begin();  
       
    while(iter != path_to_rules.end())
    {
        printf("This path includes router: ");
        std::list<uint32_t> tmp = iter->first;
        std::list<uint32_t>::iterator it;
        for (it = tmp.begin(); it != tmp.end(); ++it)
            printf("%d ", *it);

        printf("with rules: ");
        std::set<uint64_t> tmp2 = iter->second;
        std::set<uint64_t>::iterator it2;
        for(it2 = tmp2.begin(); it2 != tmp2.end(); ++it2)
            printf("%llu ", *it2);
        printf("\n");
        iter ++;         
    }
}

Reachability Reachability::operator*(Reachability &reach)
{
    Reachability insection;
    std::set<uint64_t> insection_result;

    std::list<uint32_t>::iterator it_list;
    std::map<std::list<uint32_t>, std::set<uint64_t> >::iterator it1;
    std::map<std::list<uint32_t>, std::set<uint64_t> >::iterator it2;
    for(it1 = path_to_rules.begin(); it1 != path_to_rules.end(); ++ it1)
    {
        for(it2 = reach.path_to_rules.begin(); it2 != reach.path_to_rules.end(); ++ it2)
        {
            std::set<uint64_t> tmp1 = it1->second;
            std::set<uint64_t> tmp2 = it2->second;
            std::set_intersection(tmp1.begin(), tmp1.end(), 
                            tmp2.begin(), tmp2.end(), 
                            std::inserter(insection_result, insection_result.begin()));

            if(insection_result.empty())
                continue;
            else
            {
                std::list<uint32_t> tmp_list1 = it1->first;
                std::list<uint32_t> tmp_list2 = it2->first;
                it_list = tmp_list2.begin();
                it_list = tmp_list2.erase(it_list);
                tmp_list2.splice(it_list, tmp_list1);
                insection.set_path_to_rules(tmp_list2, insection_result);
            }
        }
    }   
    return insection;
}

Reachability Reachability::operator+(Reachability &reach)
{
    Reachability union_result;
    std::map<std::list<uint32_t>, std::set<uint64_t> >::iterator it1;
    std::map<std::list<uint32_t>, std::set<uint64_t> >::iterator it2;

    for(it1 = path_to_rules.begin(); it1 != path_to_rules.end(); ++it1)
        union_result.path_to_rules[it1->first] = it1->second;
    for(it2 = reach.path_to_rules.begin(); it2 != reach.path_to_rules.end(); ++it2)
        union_result.path_to_rules[it2->first] = it2->second;

    return union_result;
}

