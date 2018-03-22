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

void Reachability::set_path_to_packets(std::list<int>* set_list, std::set<uint64_t>* set_rules)
{
    std::list<int>* new_list;
    //new_list = (std::list<int>*)malloc(sizeof(std::list<int>));
    new_list = new std::list<int>;
    (*new_list) = (*set_list);
    std::set<uint64_t>* new_rules;
    new_rules = new std::set<uint64_t>;
    (*new_rules) = (*set_rules);
    path_to_packets[new_list] = new_rules;
}

void Reachability::show_path_to_packets()
{
    std::map<std::list<int>*, std::set<uint64_t>* >::iterator iter = path_to_packets.begin();  
       
    while(iter != path_to_packets.end())
    {
        printf("This path includes router: ");
        std::list<int> tmp = (*iter->first);
        std::list<int>::iterator it;
        for (it = tmp.begin(); it != tmp.end(); ++it)
            printf("%d ", *it);

        printf("with rules: ");
        std::set<uint64_t> tmp2 = (*iter->second);
        std::set<uint64_t>::iterator it2;
        for(it2 = tmp2.begin(); it2 != tmp2.end(); ++it2)
            printf("%llu ", *it2);
        printf("\n");
        iter ++;         
    }
}

bool Reachability::is_empty()
{    
    return path_to_packets.empty();
}

void Reachability::delete_all()
{
    while (!path_to_packets.empty())
        path_to_packets.erase(path_to_packets.begin());
}

Reachability Reachability::operator*(Reachability &reach)
{
    //TODO: 这个卡一下
    Reachability insection;
    if(path_to_packets.empty() || reach.is_empty())
        return insection;
    std::list<int>::iterator it_list;
    std::map<std::list<int>*, std::set<uint64_t>* >::iterator it1;
    std::map<std::list<int>*, std::set<uint64_t>* >::iterator it2;
    for(it1 = path_to_packets.begin(); it1 != path_to_packets.end(); ++ it1)
    {
        for(it2 = reach.path_to_packets.begin(); it2 != reach.path_to_packets.end(); ++ it2)
        {
            std::set<uint64_t> insection_result;
            //std::set<uint64_t> tmp1 = (*it1->second);
            //std::set<uint64_t> tmp2 = (*it2->second);
            std::set_intersection((*it1->second).begin(), (*it1->second).end(), 
                            (*it2->second).begin(), (*it2->second).end(), 
                            std::inserter(insection_result, insection_result.begin()));
            if(insection_result.empty())
                continue;
            else
            {
                std::list<int> tmp_list1 = (*it1->first);
                std::list<int> tmp_list2 = (*it2->first);
                it_list = tmp_list2.begin();
                it_list = tmp_list2.erase(it_list);
                tmp_list2.splice(it_list, tmp_list1);
                insection.set_path_to_packets(&tmp_list2, &insection_result);
            }
        }
    }   
    return insection;
}

Reachability Reachability::operator+(Reachability &reach)
{
    //TODO: 能不能卡一下？？
    Reachability union_result;
    union_result.path_to_packets = path_to_packets;
    //std::map<std::list<int>*, std::set<uint64_t>* >::iterator it1;
    std::map<std::list<int>*, std::set<uint64_t>* >::iterator it2;

    // for(it1 = path_to_packets.begin(); it1 != path_to_packets.end(); ++it1)
    //     union_result.path_to_packets[it1->first] = it1->second;
    for(it2 = reach.path_to_packets.begin(); it2 != reach.path_to_packets.end(); ++it2)
        union_result.path_to_packets[it2->first] = it2->second;

    return union_result;
}

