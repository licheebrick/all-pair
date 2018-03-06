#include "network.h"

Network::Network()
{

}

Network::~Network()
{
    // clear topology
    topology.clear();

    //clean port_to_router
    port_to_router.clear();
}

void Network::init()
{
    //这块得等学姐的接口了
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

    printf("finish writing rules!!\n");
    
    //write port_to_router
    add_link(1,3);
    add_link(2,6);
    add_link(4,7);
    add_link(5,10);
    add_link(8,9);

    add_port_to_router(1,0);
    add_port_to_router(2,0);
    add_port_to_router(3,1);
    add_port_to_router(4,1);
    add_port_to_router(5,1);
    add_port_to_router(6,2);
    add_port_to_router(7,2);
    add_port_to_router(8,2);
    add_port_to_router(9,3);
    add_port_to_router(10,3);

    printf("finish initializing the network!!\n");
}

void Network::add_link(uint64_t from_port, uint64_t to_port)
{
    topology[from_port] = to_port;
}

void Network::print_topology()
{
    printf("topology: \n");
    std::map<uint64_t, uint64_t>::iterator it;
    it = topology.begin();
    while(it !=topology.end())
    {
        printf("from %llu to %llu\n", it->first, it->second);
        it ++;         
    }
}

void Network::add_port_to_router(uint64_t port, uint32_t router)
{
    port_to_router[port] = router;
}

void Network::print_port_to_router()
{
    printf("port_to_router: \n");
    std::map<uint64_t, uint32_t>::iterator it;
    it = port_to_router.begin();
    while(it !=port_to_router.end())
    {
        printf("from port %llu to router %lu\n", it->first, it->second);
        it ++;         
    }
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

    std::map< uint64_t, std::set<uint64_t> >::iterator it;
    it = routers[router].port_to_match.begin();
    while(it != routers[router].port_to_match.end())
    {
        uint64_t port_num = it->first;
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

        it ++;         
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