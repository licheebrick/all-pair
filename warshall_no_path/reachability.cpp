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

Reachability Reachability::operator*(const Reachability &reach)
{
    Reachability insection;
    std::set<uint64_t> insection_result;
    //Todo: 求交
    printf("qiu jiao\n");
    insection.set_rules(insection_result);
    return insection;
}

Reachability Reachability::operator+(const Reachability &reach)
{
    Reachability insection;
    std::set<uint64_t> insection_result;
    //Todo: 求并
    printf("qiu bing\n");
    insection.set_rules(insection_result);
    return insection;
}

