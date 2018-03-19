//
// Created by mac on 2018/3/9.
//

#include "network.h"
#include "utils.h"

Reachability Network::rmatrix[router_max][router_max];
Reachability Network::rmatrix1[router_max][router_max];
Reachability Network::rmatrix2[router_max][router_max];

Network::Network()
{
    r_num = 0;
}

Network::Network(int num)
{
    r_num = num;
}

Network::~Network()
{
    // clear topology
    topology.clear();

    // clean port_to_router
    port_to_router.clear();

    // delete atomic predicate list
    delete this->ap_bdd_list;
    for (int i = 0; i < this->r_num; i++) {
        delete predicate_map[i];
    }
}

void Network::init()
{
    uint64_t port1[] = {1,2};
    uint64_t port2[] = {2,3};
    uint64_t port4[] = {1,4};
    uint64_t port5[] = {1,4};
    uint64_t port8[] = {1,3};

    // TODO:: Why + 2 ?
    routers[0].port_to_match[1] = new std::set<uint64_t>; 
    routers[0].port_to_match[1]->insert(port1, port1 + 2);
    routers[0].port_to_match[2] = new std::set<uint64_t>; 
    routers[0].port_to_match[2]->insert(port2, port2 + 2);
    routers[1].port_to_match[4] = new std::set<uint64_t>; 
    routers[1].port_to_match[4]->insert(port4, port4 + 2);
    routers[1].port_to_match[5] = new std::set<uint64_t>; 
    routers[1].port_to_match[5]->insert(port5, port5 + 2);
    routers[2].port_to_match[8] = new std::set<uint64_t>; 
    routers[2].port_to_match[8]->insert(port8, port8 + 2);

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
    printf("Network topology: \n");
    std::map<uint64_t, uint64_t>::iterator it;
    it = topology.begin();
    while(it != topology.end())
    {
        printf("from %llu to %llu\n", it->first, it->second);
        it ++;         
    }
    printf("-----------------------------------------------------------------\n");
}

void Network::add_port_to_router(uint64_t port, int router)
{
    port_to_router[port] = router;
}

void Network::print_port_to_router()
{
    printf("port_to_router: \n");
    std::map<uint64_t, int>::iterator it;
    it = port_to_router.begin();
    while(it !=port_to_router.end())
    {
        printf("from port %llu to router %d\n", it->first, it->second);
        it ++;         
    }
}

void Network::add_then_load_router(uint32_t router_id, Json::Value *root) {
    struct timeval start, end;
    int router_count = this->r_num;
    if (router_id > 0) {
        gettimeofday(&start, NULL);
        // fill in router_id
        routers[router_count].set_router_id(router_id);

        // add port
        Json::Value ports = (*root)["ports"];
        for (uint32_t i = 0; i < ports.size(); i++) {
            add_port_to_router(ports[i].asUInt(), router_count);
        }

        // add bdd-represented rule
        this->dealt_bdd[router_count] = bdd_false();
        this->predicate_map[router_count] = new map< uint64_t, bdd >;
        map< uint64_t, bdd > *now_predicate_map = this->predicate_map[router_count];

        Json::Value rules = (*root)["rules"];
        for (uint32_t id = rules.size(); id != 0; id--) {
            uint32_t i = id - 1;
            string action = rules[i]["action"].asString();
            if (action == "fwd") {
                string match = rules[i]["match"].asString();
                // Here we just ignore the difference of inport
                for (uint32_t op_id = 0; op_id < rules[i]["out_ports"].size(); op_id++) {
                    uint64_t port_id = rules[i]["out_ports"][op_id].asUInt64();
                    bdd match_bdd = match2bdd(match, this->hdr_len);
                    match_bdd -= dealt_bdd[router_count];
                    if (now_predicate_map->count(port_id) == 0) {
                        // We never encountered this outport before
                        now_predicate_map->insert(make_pair(port_id, match_bdd));
                    } else {
                        // Encountered outport
                        now_predicate_map->at(port_id) |= match_bdd;
                    }
                    dealt_bdd[router_count] |= match_bdd;
                }
            } else {
                printf("Rule type %s encountered, ignore for now.\n", action.c_str());
            }
        }
        this->r_num++;
        gettimeofday(&end, NULL);
        printf("Finish loading router %u, time used: %ld us.\n", router_id,
               1000000 * (end.tv_sec - start.tv_sec) + end.tv_usec - start.tv_usec);
    }
    return;
}

// pre-process rules to number set
void Network::make_atomic_predicates() {
    struct timeval time_start, time_end;
    gettimeofday(&time_start, NULL);
    list< bdd > ap_list;
    bdd true_bdd = bdd_true();
    ap_list.push_back(true_bdd);

    for (int i = 0; i < this->r_num; i++) {
        for (auto it = this->predicate_map[i]->begin(); it != this->predicate_map[i]->end(); it++) {
            bdd P = it->second;
            if (P != bddfalse && P != bddtrue) {
                uint64_t ori_size= ap_list.size();
                for (uint64_t j = 0; j < ori_size; j++) {
                    bool del_flag = false;
                    bdd bdd_now = ap_list.back();
                    bdd trueset = bdd_now & P;
                    bdd falseset = bdd_now & (!P);
                    if (trueset != bddfalse) {
                        ap_list.push_front(trueset);
                        del_flag = true;
                    }
                    if (falseset != bddfalse) {
                        ap_list.push_front(falseset);
                        del_flag = true;
                    }
                    ap_list.pop_back();
                    if (!del_flag) {
                        ap_list.push_front(bdd_now);
                    }
                }
            }
        }
    }
    gettimeofday(&time_end, NULL);
    printf("Finish making atomic predicates, total %lu atomic predicates, time used: %ld us.\n",
           ap_list.size(), 1000000 * (time_end.tv_sec - time_start.tv_sec) + time_end.tv_usec - time_start.tv_usec);
    vector<bdd>* ap_bdd_vec = new vector<bdd>{make_move_iterator(begin(ap_list)), make_move_iterator(end(ap_list))};
    this->ap_bdd_list = ap_bdd_vec;
    if (show_detail) {
        printf("These %lu atomic predicates are as follows:\n", this->ap_bdd_list->size());
        for (size_t i = 0; i < this->ap_bdd_list->size(); i++) {
            printf("Predicate %lu:\n", i);
            bdd_allsat(this->ap_bdd_list->at(i), allsatPrintHandler);
        }
    }
    printf("-----------------------------------------------------------------\n");
}

void Network::convert_router_to_ap() {
    struct timeval start, end;
    gettimeofday(&start, NULL);
    for (int i = 0; i < this->r_num; i++) {
        for (auto it = this->predicate_map[i]->begin(); it != this->predicate_map[i]->end(); it++) {
            this->routers[i].port_to_match[it->first] = new set< uint64_t >;
            for (uint64_t j = 0; j < this->ap_bdd_list->size(); j++) {
                if ((this->ap_bdd_list->at(j) & it->second) != bddfalse) {
                    this->routers[i].port_to_match[it->first]->insert(j);
                }
            }
        }
    }
    gettimeofday(&end, NULL);
    printf("Finish converting atomic predicates, time used: %ld us.\n",
           1000000 * (end.tv_sec - start.tv_sec) + end.tv_usec - start.tv_usec);
    if (show_detail) {
        printf("Converted rules are as follows:\n");
        for (int i = 0; i < this->r_num; i++) {
            this->routers[i].print_port_to_match();
        }
    }
    printf("-----------------------------------------------------------------\n");
}

void Network::brutal_force()
{
    //init
    uint64_t full_array[rule_type];
    for (uint64_t i = 0; i < rule_type; i++)
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
            // TODO: warning: use 'uint32_t' for signed values of type 'int'
            router_stack[stack_place++] = i;
            // TODO: warning: use 'uint32_t' for signed values of type 'int'
            dfs_search(i, j, &full_rules);
            // TODO: warning: use 'uint32_t' for signed values of type 'int'
            router_stack[stack_place--] = -1;
            printf("finish this one~\n");
        }
    }
}

void Network::dfs_search(int router, int destiny, std::set<uint64_t>* rules)
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
            continue;
        //如果到过了就GG
        std::set<uint64_t>* new_match;
        new_match = new std::set<uint64_t>;
        std::set_intersection((*rules).begin(), (*rules).end(), 
                                (*routers[router].port_to_match[port_num]).begin(), 
                                (*routers[router].port_to_match[port_num]).end(), 
                                std::inserter(*new_match, (*new_match).begin()));
        if((*new_match).empty())
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
            int router1 = port_to_router[port_num];
            int router2 = port_to_router[next_port_num];
            int router_array[] = {router1, router2};
            std::list<uint32_t> tmp;
            tmp.assign(router_array, router_array + 2);
            rmatrix[router1][router2].set_path_to_packets(&tmp, it->second);
            is_height[router1] = true;
            is_width[router2] = true;
            it ++;    
        }
    }
    printf("miao\n");
    
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
            rmatrix2[i][j] = rmatrix[i][j];
    for(int k = 1; k <= r_num; k++)
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

    if(r_num % 2 == 1)
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
    else if(r_num % 2 == 0)
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