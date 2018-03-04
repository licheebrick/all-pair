#include <stdint.h>
#include <iostream>
#include <map>
#include <vector>
#include <set>
using namespace std;

const int router_max = 1000;//total
const int rule_type = 10;   //假设有10种流量
int router_num = 4;        //routers num

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
    std::vector<uint64_t> router_to_port;//我认为它包含着所有存在的port
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
    void dfs_search(uint32_t router, uint32_t destiny, std::set<uint64_t> rules);
    void display_result(std::set<uint64_t> rules);
    void display_rules(std::set<uint64_t> rules);
    void display_data(int type);//for debug
private:
    //map <port_id, another_port_id>
    std::map<uint64_t, uint64_t> topology;
    //map <port_id, router_id>
    std::map< uint64_t, uint32_t> port_to_router;
    //for dfs search
    bool have_been[router_max] = {false};
    uint32_t router_stack[router_max] = {-1};
    uint32_t stack_place = 0;
}; 

void Network::init()
{
    //write port_to_router
    topology[1] = 3;
    topology[2] = 6;
    topology[4] = 7;
    topology[5] = 10;
    topology[8] = 9;
    // topology[1] = 3;
    // topology[1] = 3;
    // topology[1] = 3;

    port_to_router[1] = 0;
    port_to_router[2] = 0;
    port_to_router[3] = 1;
    port_to_router[4] = 1;
    port_to_router[5] = 1;
    port_to_router[6] = 2;
    port_to_router[7] = 2;
    port_to_router[8] = 2;
    port_to_router[9] = 3;
    port_to_router[10] = 3;

    printf("finish initializing the network!!\n");
}

void Network::all_pair_reachability()
{
    //还是递归吧
    uint64_t full_array[rule_type];
    for (int i = 0; i < rule_type; i++)
    {
        full_array[i] = i + 1;
    }
    std::set<uint64_t> full_rules (full_array, full_array + rule_type);
    for (int i = 0; i < router_num; i++)
    {
        for(int j = 0; j < router_num; j++)
        {
            //from i to j
            printf("~~~~~~~~ search for the path from %d to %d ~~~~~~~~\n", i, j);
            if(i != j)
                have_been[i] = true;
            router_stack[stack_place++] = i;
            dfs_search(i, j, full_rules);
            router_stack[stack_place--] = -1;
            printf("finish this one~\n");
        }
    }
}

void Network::dfs_search(uint32_t router, uint32_t destiny, std::set<uint64_t> rules)
{
    //我们默认它进入这个函数是不可能相同的
    int count = routers[router].router_to_port.size();
    if(count == 0)
        return;

    //display_data(0);
    //display_rules(rules);
    for(int i = 0; i < count; i++)
    {
        uint64_t port_num = routers[router].router_to_port.at(i);
        if(topology.find(port_num) != topology.end())
        {
            uint64_t next_port_num = topology[port_num];
            //有这条边
            if(have_been[port_to_router[next_port_num]])
                continue;
            //如果到过了就GG
            std::set<uint64_t> new_match;
            std::set_intersection(rules.begin(), rules.end(), 
                                    routers[router].port_to_match[port_num].begin(), 
                                    routers[router].port_to_match[port_num].end(), 
                                    std::inserter(new_match, new_match.begin()));
            //printf("anybody here?\n");
            if(new_match.empty())
                continue;
            //如果不为空则可达
            if(port_to_router[next_port_num] == destiny)
            {
                router_stack[stack_place++] = destiny;
                display_result(new_match);
                router_stack[stack_place--] = -1;
            }
            else
            {
                router_stack[stack_place++] = port_to_router[next_port_num];
                have_been[port_to_router[next_port_num]] = true;
                dfs_search(port_to_router[next_port_num], destiny, new_match);
                router_stack[stack_place--] = -1;
                have_been[port_to_router[next_port_num]] = false;
            }
            //如果刚好到了终点则输出
        }
    }    
}

void Network::display_result(std::set<uint64_t> rules)
{
    //display the path and match
    printf("This path includes router: ");
    for(int i = 0; i < stack_place; i++)
    {
        printf("%d ", router_stack[i]);
    }
    printf("with rules: ");
    std::set<uint64_t>::iterator it;
    for(it = rules.begin(); it != rules.end(); it++)
    {
        cout<< *it << " ";
    } 
    printf("\n");
}

void Network::display_rules(std::set<uint64_t> rules)
{
    printf("Rules: \n");
    std::set<uint64_t>::iterator it;
    for(it = rules.begin(); it != rules.end(); it++)
    {
        cout<< *it << " ";
    } 
    printf("\n");
}

void Network::display_data(int type)
{
    printf("---> display <---\n");
    if(type == 0)
    {
        printf("topology: \n");
        std::map<uint64_t, uint64_t>::iterator it;
        it = topology.begin();
        while(it !=topology.end())
        {
            //it->first;
            //it->second;
            printf("from %d to %d\n", it->first, it->second);
            it ++;         
        }
    }
    if(type == 2)
    {
        printf("stack: \n");
        for(int i = 0; i < stack_place; i++)
        {
            printf("%d ", router_stack[i]);
        }
        printf("\n");
    }
}

void write_rules()
{
    uint64_t port1[] = {1,2};
    uint64_t port2[] = {2,3};
    uint64_t port4[] = {1,4};
    uint64_t port5[] = {1,4};
    uint64_t port8[] = {1,3};

    std::set<uint64_t> portset1 (port1, port1 + 2);
    routers[0].port_to_match[1] = portset1;
    std::set<uint64_t> portset2 (port2, port2 + 2);
    routers[0].port_to_match[2] = portset2;
    std::set<uint64_t> portset4 (port4, port4 + 2);
    routers[1].port_to_match[4] = portset4;
    std::set<uint64_t> portset5 (port5, port5 + 2);
    routers[1].port_to_match[5] = portset5;
    std::set<uint64_t> portset8 (port8, port8 + 2);
    routers[2].port_to_match[8] = portset8;

    routers[0].router_to_port.push_back(1);
    routers[0].router_to_port.push_back(2);
    routers[1].router_to_port.push_back(3);
    routers[1].router_to_port.push_back(4);
    routers[1].router_to_port.push_back(5);
    routers[2].router_to_port.push_back(6);
    routers[2].router_to_port.push_back(7);
    routers[2].router_to_port.push_back(8);
    routers[3].router_to_port.push_back(9);
    routers[3].router_to_port.push_back(10);
    printf("finish writing rules!!\n");
}

int main()
{
    write_rules();
    Network network_example;
    network_example.init();
    network_example.all_pair_reachability();   
}