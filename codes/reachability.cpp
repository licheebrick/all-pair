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
    path_to_packets[set_list] = set_rules;
}

void Reachability::set_rules(std::set<uint64_t>* set_rules)
{
    rules = *set_rules;
}

void Reachability::show_path_to_packets()
{
    std::map<std::list<int>*, std::set<uint64_t>* >::iterator iter = path_to_packets.begin();  
    //TODO: 更改这个是输出，变成router的id
    while(iter != path_to_packets.end())
    {
        printf("This path includes router: [");
        std::list<int> tmp = (*iter->first);
        std::list<int>::iterator it;
        for (it = tmp.begin(); it != tmp.end(); ++it)
            printf("%d -> ", *it);

        printf("] with header: ");
        std::set<uint64_t> tmp2 = (*iter->second);
        std::set<uint64_t>::iterator it2;
        for(it2 = tmp2.begin(); it2 != tmp2.end(); ++it2)
            printf("%llu ", *it2);
        printf("\n");
        iter ++;         
    }
}

void Reachability::show_rules()
{
    std::set<uint64_t>::iterator iter = rules.begin();  
      
    printf("Rules: ");
    while(iter!=rules.end())  
    {  
        printf("%llu ", *iter);
        ++iter;  
    }  
    printf("\n");
}

bool Reachability::is_empty()
{    
    return path_to_packets.empty();
}

void Reachability::delete_all()
{
    path_to_packets.clear();
}

Reachability Reachability::operator*(Reachability &reach)
{
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
            std::set<uint64_t>* insection_result;
            insection_result = new std::set<uint64_t>;
            std::set_intersection((*it1->second).begin(), (*it1->second).end(), 
                            (*it2->second).begin(), (*it2->second).end(), 
                            std::inserter(*insection_result, (*insection_result).begin()));
            if((*insection_result).empty())
                continue;
            else
            {
                std::list<int> tmp_list1 = (*it1->first);
                std::list<int>* tmp_list2; 
                tmp_list2 = new std::list<int>;
                (*tmp_list2) = (*it2->first);
                it_list = (*tmp_list2).begin();
                it_list = (*tmp_list2).erase(it_list);
                (*tmp_list2).splice(it_list, tmp_list1);
                insection.set_path_to_packets(&(*tmp_list2), &(*insection_result));
            }
        }
    }   
    return insection;
}

Reachability Reachability::operator+(Reachability &reach)
{
    Reachability union_result;
    union_result.path_to_packets = path_to_packets;
    std::map<std::list<int>*, std::set<uint64_t>* >::iterator it2;

    for(it2 = reach.path_to_packets.begin(); it2 != reach.path_to_packets.end(); ++it2)
        union_result.path_to_packets[it2->first] = it2->second;

    return union_result;
}

Reachability Reachability::operator/(Reachability &reach)
{
    Reachability insection;
    std::set<uint64_t> insection_result;
    std::set_intersection(rules.begin(), rules.end(), 
                            reach.rules.begin(), reach.rules.end(), 
                            std::inserter(insection_result, insection_result.begin()));
    insection.set_rules(&insection_result);
    return insection;
}

Reachability Reachability::operator-(Reachability &reach)
{
    Reachability insection;
    std::set<uint64_t> insection_result;
    std::set_union(rules.begin(), rules.end(), 
                            reach.rules.begin(), reach.rules.end(), 
                            std::inserter(insection_result, insection_result.begin()));
    insection.set_rules(&insection_result);
    return insection;
}
