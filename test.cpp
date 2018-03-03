#include <stdint.h>
#include <iostream>
#include <map>
#include <vector>
#include <set>

const int router_max = 1000;//total
int router_num = 10;        //routers num

class Router
{
public:
    Router(){}
    Router(uint32_t id){router_id = id;}
    void routers_init(uint32_t id);
    uint32_t getid(){return router_id;}
    //map<port_id, match>
    std::map< uint64_t, std::set<uint64_t> > port_to_match;
    //我觉得二进制是真的好，后面还可以直接计算，看起来方便不少，如果不多的话，干脆用一个unsigned啥的存算了
    //但是考虑到实际情况也不一定好，毕竟集合计算很多是跟fi搞
    //set也挺好，但是不知道它的求交好不好，不行就自己重新写一个
private:
    uint32_t router_id;//反正是const的，貌似加上const会出事
}routers[router_max];

void Router::routers_init(uint32_t id)//用来暴力初始化router的
{
    router_id = id;
}

class Network
{
public:
    Network(){}
    void init();
    void all_pair_reachability();
private:
    //map <port_id, another_port_id>
    std::map<uint64_t, uint64_t> topology;
    //map <port_id, router_id>
    std::map< uint64_t, uint32_t > port_to_router;
}; 

void Network::init()
{
    //write inport_to_router
}

void Network::all_pair_reachability()
{

}

void write_rules()
{
    uint64_t port1[] = {1,2,3};
    std::set<uint64_t> second (port1, port1 + 3);
    routers[0].port_to_match[1] = second;
}

int main()
{
    
}