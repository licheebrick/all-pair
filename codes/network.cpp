//
// Created by mac on 2018/3/9.
//

#include "network.h"

Reachability Network::rmatrix[router_max][router_max];
Reachability Network::rmatrix1[router_max][router_max];
Reachability Network::rmatrix2[router_max][router_max];
Reachability Network::rmatrix3[router_max][router_max];

Network::Network()
{
    r_num = router_max;
}

Network::Network(int num)
{
    r_num = num;
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
    uint64_t port1[] = {3,4};
    uint64_t port2[] = {1,2};
    uint64_t port4[] = {2,3};
    uint64_t port5[] = {1,4};
    uint64_t port8[] = {1,2,3};

    routers[0].port_to_match[1] = new std::set<uint64_t>; 
    routers[0].port_to_match[1]->insert(port1, port1 + 2);
    routers[0].port_to_match[2] = new std::set<uint64_t>; 
    routers[0].port_to_match[2]->insert(port2, port2 + 2);
    routers[1].port_to_match[4] = new std::set<uint64_t>; 
    routers[1].port_to_match[4]->insert(port4, port4 + 2);
    routers[1].port_to_match[5] = new std::set<uint64_t>; 
    routers[1].port_to_match[5]->insert(port5, port5 + 2);
    routers[2].port_to_match[8] = new std::set<uint64_t>; 
    routers[2].port_to_match[8]->insert(port8, port8 + 3);

    printf("finish writing rules!!\n");
    
    //write port_to_router
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
    while(it != topology.end())
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
        printf("from port %llu to router %d\n", it->first, it->second);
        it ++;         
    }
}

void Network::brutal_force()
{
    //init
    uint64_t full_array[rule_type];
    for (int i = 0; i < rule_type; i++)
        full_array[i] = i + 1;
    memset(have_been, false, router_max);
    memset(router_stack, 99999, router_max);
    stack_place = 0;

    std::set<uint64_t> full_rules (full_array, full_array + rule_type);
    for (int i = 0; i < r_num; i++)
    {
        for(int j = 0; j < r_num; j++)
        {
            //from i to j
            printf("~~~~~~~~ search for the path from %d to %d ~~~~~~~~\n", i, j);
            if(i != j)
                have_been[i] = true;
            router_stack[stack_place++] = i;
            dfs_search(i, j, &full_rules);
            router_stack[stack_place--] = -1;
            printf("finish this one~\n");
        }
    }
}

void Network::dfs_search(uint32_t router, uint32_t destiny, std::set<uint64_t>* rules)
{
    //我们默认它进入这个函数是不可能相同的
    
    std::map< uint64_t, std::set<uint64_t>* >::iterator it;
    it = routers[router].port_to_match.begin();
    while(it != routers[router].port_to_match.end())
    {
        uint64_t port_num = it->first;
        uint64_t next_port_num = topology[port_num];
        //有这条边
        if(have_been[port_to_router[next_port_num]])
        {
            it++;
            continue;
        }
            
        //如果到过了就GG
        std::set<uint64_t>* new_match;
        new_match = new std::set<uint64_t>;
        std::set_intersection((*rules).begin(), (*rules).end(), 
                                (*routers[router].port_to_match[port_num]).begin(), 
                                (*routers[router].port_to_match[port_num]).end(), 
                                std::inserter(*new_match, (*new_match).begin()));
        if((*new_match).empty())
        {
            it++;
            continue;
        }     
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
        delete new_match;
        new_match = NULL;
        it ++;         
    }
}

void Network::display_result(std::set<uint64_t>* rules)
{
    //display the path and match
    printf("This path includes router: ");
    for(int i = 0; i < stack_place; i++)
        printf("%d ", router_stack[i]);

    printf("with rules: ");
    std::set<uint64_t>::iterator it;
    for(it = (*rules).begin(); it != (*rules).end(); it++)
        printf("%llu ", *it);
    printf("\n");
}

void Network::warshall_with_path()
{
    //先算一个基本的r0，然后推导到rk
    bool is_height[router_max] = {false};
    bool is_width[router_max] = {false};

    for(int i = 0; i < r_num; i++)
    {
        std::map< uint64_t, std::set<uint64_t>* >::iterator it;
        it = routers[i].port_to_match.begin();
        while(it != routers[i].port_to_match.end())
        {
            uint64_t port_num = it->first;
            uint64_t next_port_num = topology[port_num];
            uint32_t router1 = port_to_router[port_num];
            uint32_t router2 = port_to_router[next_port_num];
            uint32_t router_array[] = {router1, router2};
            std::list<uint32_t> tmp;
            tmp.assign(router_array, router_array + 2);
            rmatrix[router1][router2].set_path_to_packets(&tmp, it->second);
            is_height[router1] = true;
            is_width[router2] = true;
            it ++;    
        }
    }
    
    //进行一个矩阵变换
    int minimatrix[router_max][router_max][2];
    int height = 0;
    int width = 0;
    for(int i = 0; i < r_num; i++)
    {
        if(!is_height[i])
            continue;

        width = 0;
        for(int j = 0; j < r_num; j++)
        {
            if(!is_width[j])
                continue;
            else
            {
                minimatrix[height][width][0] = i;
                minimatrix[height][width][1] = j;
                width++;
            }   
        }
        height++;
    }
    
    for(int i = 0; i < r_num; i++)
        for(int j = 0; j < r_num; j++)
            rmatrix1[i][j] = rmatrix[i][j];
    for(int k = 0; k < r_num; k++)
    {   
        if(k % 2 == 1)
        {
            for(int i = 0; i < height; i++)
            {
                for(int j = 0; j < width; j++)
                {
                    rmatrix1[minimatrix[i][j][0]][minimatrix[i][j][1]] = rmatrix2[minimatrix[i][j][0]][k] * rmatrix2[k][minimatrix[i][j][1]];
                    rmatrix1[minimatrix[i][j][0]][minimatrix[i][j][1]] = rmatrix1[minimatrix[i][j][0]][minimatrix[i][j][1]] + rmatrix2[minimatrix[i][j][0]][minimatrix[i][j][1]]; 
                }
            }
        }
        else if(k % 2 == 0)
        {
            for(int i = 0; i < height; i++)
            {
                for(int j = 0; j < width; j++)
                {
                    rmatrix2[minimatrix[i][j][0]][minimatrix[i][j][1]] = rmatrix1[minimatrix[i][j][0]][k] * rmatrix1[k][minimatrix[i][j][1]];
                    rmatrix2[minimatrix[i][j][0]][minimatrix[i][j][1]] = rmatrix2[minimatrix[i][j][0]][minimatrix[i][j][1]] + rmatrix1[minimatrix[i][j][0]][minimatrix[i][j][1]];
                }
            }
        }
    }

    if(r_num % 2 == 0)
    {
        for(int i = 0; i < r_num; i++)
        {
            for(int j = 0; j < r_num; j++)
            {
                printf("matrix: %d : %d \n", i, j);
                rmatrix1[i][j].show_path_to_packets();
            }
        }
    }
    else if(r_num % 2 == 1)
    {
        for(int i = 0; i < r_num; i++)
        {
            for(int j = 0; j < r_num; j++)
            {
                printf("matrix: %d : %d \n", i, j);
                rmatrix2[i][j].show_path_to_packets();
            }
        }
    }
}

void Network::segment_based()
{
    //先算一个基本的r0，然后推导到rk
    bool is_height[router_max] = {false};
    bool is_width[router_max] = {false};
    
    for(int i = 0; i < r_num; i++)
    {
        std::map< uint64_t, std::set<uint64_t>* >::iterator it;
        it = routers[i].port_to_match.begin();
        while(it != routers[i].port_to_match.end())
        {
            uint64_t port_num = it->first;
            uint64_t next_port_num = topology[port_num];
            uint32_t router1 = port_to_router[port_num];
            uint32_t router2 = port_to_router[next_port_num];
            uint32_t router_array[] = {router1, router2};
            std::list<uint32_t> tmp;
            tmp.assign(router_array, router_array + 2);
            rmatrix[router1][router2].set_path_to_packets(&tmp, it->second);
            is_height[router1] = true;
            is_width[router2] = true;
            it ++;    
        }
    }

    for(int i = 0; i < r_num; i++)
        for(int j = 0; j < r_num; j++)
        {
            rmatrix2[i][j] = rmatrix[i][j];
            rmatrix3[i][j] = rmatrix[i][j];
        }
            
    Reachability tmp;
    //rmatrix3用来存总的，rmatrix1和rmatrix2是分的
    for(int k = 3; k <= r_num; k++)
    {   
        if(k % 2 == 1)
        {
            for(int i = 0; i < r_num; i++)
            {
                if(!is_height[i])
                    continue;
                for(int j = 0; j < r_num; j++)
                {
                    if(!is_width[j])
                        continue;
                    rmatrix1[i][j] = rmatrix2[i][0] * rmatrix[0][j];
                    for(int place = 0; place < r_num; place++)
                    {
                        tmp = rmatrix2[i][place] * rmatrix[place][j];
                        rmatrix1[i][j] = rmatrix1[i][j] + tmp;
                    }
                    rmatrix3[i][j] = rmatrix3[i][j] + rmatrix1[i][j];
                }
            }
        }
        else if(k % 2 == 0)
        {
            for(int i = 0; i < r_num; i++)
            {
                if(!is_height[i])
                    continue;
                for(int j = 0; j < r_num; j++)
                {
                    if(!is_width[j])
                        continue;
                    rmatrix2[i][j] = rmatrix1[i][0] * rmatrix[0][j];
                    for(int place = 0; place < r_num; place++)
                    {
                        tmp = rmatrix1[i][place] * rmatrix[place][j];
                        rmatrix2[i][j] = rmatrix2[i][j] + tmp;
                    }
                    rmatrix3[i][j] = rmatrix3[i][j] + rmatrix2[i][j];
                }
            }
        }
    }

    for(int i = 0; i < r_num; i++)
    {
        for(int j = 0; j < r_num; j++)
        {
            printf("matrix: %d : %d \n", i, j);
            rmatrix3[i][j].show_path_to_packets();
        }
    }
}

void Network::rule_based()
{
    Rulebased rulebased;
    for(uint64_t i = 1; i <= rule_type; i++)
    {
        uint64_t portnum = 0;
        uint32_t router_place = 0;
        for(uint32_t j = 0; j < r_num; j++)
        {       
            router_place = j;
            string list_str;
            std::list<uint32_t> router_list;
            list_str = list_str + to_string(j);
            router_list.push_back(j);
            while(1)
            {
                printf("%d %d \n", i, router_place);
                bool flag = true;
                std::map< uint64_t, std::set<uint64_t>* >::iterator it;
                it = routers[router_place].port_to_match.begin();
                while(it != routers[router_place].port_to_match.end())
                {
                    std::set<uint64_t>::iterator iter;
                    iter = (*it->second).find(i);
                    if(iter != (*it->second).end())
                    {
                        portnum = it->first;
                        flag = false;
                        break;
                    }
                    it++;
                }
                if(flag)//to the end
                    break;
                router_place = port_to_router[topology[portnum]];
                list_str = list_str + to_string(router_place);
                router_list.push_back(router_place);
                rulebased.set_new_rule(list_str, &router_list, i);
            }
        }
    }

    rulebased.print_rule_map();
}