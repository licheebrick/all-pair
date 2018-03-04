#ifndef ROUTER_H
#define ROUTER_H

#include <stdint.h>
#include <iostream>
#include <map>
#include <vector>
#include <set>
using namespace std;

class Router
{
public:
    Router(){}
    Router(uint32_t id){router_id = id;}
    void routers_init(uint32_t id);
    void update_port_to_match(std::map< uint64_t, std::set<uint64_t> > new_one);
    void update_port_to_match(uint64_t port, std::set<uint64_t> match);
    std::map< uint64_t, std::set<uint64_t> > get_port_to_match();
    uint32_t getid(){return router_id;}
    //map<port_id, match>
    std::map< uint64_t, std::set<uint64_t> > port_to_match;
    //我觉得二进制是真的好，后面还可以直接计算，看起来方便不少，如果不多的话，干脆用一个unsigned啥的存算了
    //但是考虑到实际情况也不一定好，毕竟集合计算很多是跟fi搞
    //set也挺好，但是不知道它的求交好不好，不行就自己重新写一个
private:
    uint32_t router_id;//反正是const的，貌似加上const会出事
};

#endif //ROUTER_H