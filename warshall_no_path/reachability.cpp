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

std::set<uint64_t> Reachability::get_rules()
{
    return rules;
}

void Reachability::show_rules()
{
    printf("~~ show the rules ~~\n");
    std::set<uint64_t>::iterator iter = rules.begin();  
      
    while(iter!=rules.end())  
    {  
        cout<<*iter<<endl;  
        ++iter;  
    }  
}

Reachability Reachability::operator*(const Reachability &reach) const
{
    Reachability insection;
    std::set<uint64_t> insection_result;
    std::set_intersection(rules.begin(), rules.end(), 
                            reach.rules.begin(), reach.rules.end(), 
                            std::inserter(insection_result, insection_result.begin()));
    insection.set_rules(insection_result);
    return insection;
}

Reachability Reachability::operator+(const Reachability &reach) const
{
    Reachability insection;
    std::set<uint64_t> insection_result;
    std::set_union(rules.begin(), rules.end(), 
                            reach.rules.begin(), reach.rules.end(), 
                            std::inserter(insection_result, insection_result.begin()));
    insection.set_rules(insection_result);
    return insection;
}

