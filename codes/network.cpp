//
// Created by mac on 2018/3/9.
//

#include "network.h"
#include "utils.h"
#include <fstream>
#include <sstream>

Reachability Network::rmatrix[router_max][router_max];
Reachability Network::rmatrix1[router_max][router_max];
Reachability Network::rmatrix2[router_max][router_max];
Reachability Network::rmatrix3[router_max][router_max];

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
    // delete this->ap_bdd_list;

    for (int i = 0; i < this->r_num; i++) {
        delete predicate_map[i];
    }
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
                // printf("Rule type %s encountered, ignore for now.\n", action.c_str());
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
    vector<bdd>* ap_bdd_vec = new vector<bdd>{make_move_iterator(begin(ap_list)), make_move_iterator(end(ap_list))};
    this->ap_bdd_list = ap_bdd_vec;

    this->ap_num = ap_bdd_list->size();
    printf("Finish making atomic predicates, total %llu atomic predicates, time used: %ld us.\n",
           ap_num, 1000000 * (time_end.tv_sec - time_start.tv_sec) + time_end.tv_usec - time_start.tv_usec);
    if (show_detail) {
        printf("These %llu atomic predicates are as follows:\n", this->ap_num);
        for (size_t i = 0; i < this->ap_bdd_list->size(); i++) {
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

void Network::dump_port_to_router_to_file(string file_path) {
    Json::Value root;
    struct timeval start, end;
    for (auto it = this->port_to_router.begin(); it != this->port_to_router.end(); it++) {
        Json::Value pair;
        pair["port"] = it->first;
        pair["router"] = it->second;
        root.append(pair);
    }

    std::ofstream file;
    file.open(file_path);
    Json::StyledWriter writer;
    file << writer.write(root);
    file.close();
}

void Network::load_port_to_router_from_file(string file_path) {
    ifstream jsfile;
    Json::Value root;
    Json::Reader reader;

    jsfile.open(file_path);
    if (!jsfile.good()) {
        printf("Error opening file %s\n", file_path.c_str());
    }
    reader.parse(jsfile, root, false);

    for (unsigned i = 0; i < root.size(); i++) {
        this->add_port_to_router(root[i]["port"].asUInt64(), root[i]["router"].asInt());
    }
    jsfile.close();
}

void Network::dump_ap_rules_to_file(string file_path) {
    Json::Value root;
    struct timeval start, end;
    gettimeofday(&start, NULL);
    for (int i = 0; i < this->r_num; i++) {
        Json::Value router;
        for (auto it = this->routers[i].port_to_match.begin(); it != this->routers[i].port_to_match.end(); it++) {
            Json::Value port_ap;
            port_ap["port"] = it->first;
            for (auto jt : (*it->second)) {
                port_ap["match"].append(jt);
            }
            router["rules"].append(port_ap);
        }
        router["index"] = i;
        router["id"] = routers[i].getid();
        root["routers"].append(router);
    }
    std::ofstream file;
    file.open(file_path);
    Json::StyledWriter writer;
    file << writer.write(root);
    file.close();
    gettimeofday(&end, NULL);
    printf("Finish dumping atomized rules to file, time used: %ld us.\n",
           1000000 * (end.tv_sec - start.tv_sec) + end.tv_usec - start.tv_usec);
}

void Network::load_ap_rules_from_file(string file_path) {
    ifstream jsfile;
    Json::Value root;
    Json::Reader reader;

    jsfile.open(file_path);
    if (!jsfile.good()) {
        printf("Error opening file %s\n", file_path.c_str());
    }
    reader.parse(jsfile, root, false);
    Json::Value rtrs = root["routers"];
    for (unsigned i = 0; i < rtrs.size(); i++) {
        this->r_num++;
        int idx = rtrs[i]["index"].asInt();
        Json::Value rules = rtrs[i]["rules"];
        this->routers[i].set_router_id(rtrs[i]["id"].asUInt());
        for (unsigned j = 0; j < rules.size(); j++) {
            uint64_t port_id = rules[j]["port"].asUInt64();
            add_port_to_router(port_id, idx);
            this->routers[i].port_to_match[port_id] = new set< uint64_t >;
            Json::Value match_list = rules[j]["match"];
            for (unsigned k = 0; k < match_list.size(); k++) {
                this->routers[i].port_to_match[port_id]->insert(match_list[k].asUInt64());
            }
        }
    }
    jsfile.close();
    this->ap_num = rule_type;
}

void Network::refresh_matrix()
{
    std::set<uint64_t> newone;
    for(int i = 0; i < router_max; i++)
    {
        for(int j = 0; j < router_max; j++)
        {
            b_matrix[i][j] = newone;
            matrix1[i][j] = newone;
            matrix2[i][j] = newone;
            matrix3[i][j] = newone;
        }
    }

    Reachability newone2;
    for(int i = 0; i < router_max; i++)
    {
        for(int j = 0; j < router_max; j++)
        {
            rmatrix[i][j] = newone2;
            rmatrix1[i][j] = newone2;
            rmatrix2[i][j] = newone2;
            rmatrix3[i][j] = newone2;
        }
    }

    rulebased.clean_up();
}

void Network::display_result(std::set<uint64_t>* rules)
{
    //display path and match
    printf("One path found: [");
    for(int i = 0; i < stack_place - 1; i++)
        printf("%u -> ", routers[router_stack[i]].getid());
    printf("%u], with header: ", routers[router_stack[stack_place - 1]].getid());
    std::set<uint64_t>::iterator it;
    for(it = (*rules).begin(); it != (*rules).end(); it++)
        printf("%llu ", *it);
    printf("\n");
}

void Network::brutal_force_with_path(bool need_print, bool need_loop)
{
    // init
    memset(have_been, false, router_max);
    memset(router_stack, 99999, router_max);
    stack_place = 0;
    std::set<uint64_t> full_rules;
    for (uint64_t i = 0; i < ap_num; i++) {
        full_rules.insert(i);
    }

    for (int i = 0; i < r_num; i++)
    {
        for(int j = 0; j < r_num; j++)
        {
            //if(need_print)
                //printf("Searching for path from router %u to router %u...\n", routers[i].getid(), routers[j].getid());
            have_been[i] = true;
            router_stack[stack_place++] = i;
            dfs_search_with_path(i, j, &full_rules, need_loop, need_print);
            router_stack[stack_place--] = -1;
            //if(need_print)
              //  printf("===\n");
            memset(have_been, false, router_max);  // clear state
        }
    }
    if(need_print) {
        printf("Now printing the result of brute force with path:\n");
        print_matrix(3);
    }
}

void Network::dfs_search_with_path(int router, int destiny, std::set<uint64_t>* rules, bool need_loop, bool need_print)
{
    set< uint64_t >* new_match;
    for (auto it = routers[router].port_to_match.begin(); it != routers[router].port_to_match.end(); it++) {
        if (topology.count(it->first) == 0)
            continue;
        new_match = new set<uint64_t >;
        uint64_t port_num = it->first;
        uint64_t next_port_num = topology[port_num];
        std::set_intersection((*rules).begin(), (*rules).end(),
                              (*routers[router].port_to_match[port_num]).begin(),
                              (*routers[router].port_to_match[port_num]).end(),
                              std::inserter(*new_match, new_match->begin()));
        // 如果是空集，直接就跳下一个port
        if (new_match->empty()) {
            delete new_match;
            new_match = NULL;
            continue;
        }
        // 如果不是空集...
        if (port_to_router[next_port_num] == destiny) { // 如果到了目的地，应该总是第一次到，否则在前边就会记录
            // 首先将这个match加入
            b_matrix[router_stack[0]][destiny].insert(new_match->begin(), new_match->end());
            router_stack[stack_place++] = destiny;
            if (need_print) {
                if (need_loop && (destiny == router_stack[0])) {
                    printf("Loop here, ");
                }
                display_result(new_match);
            }
            router_stack[stack_place--] = -1;
        } else {
            if (have_been[port_to_router[next_port_num]]) {  // 遇到了loop，直接continue
                delete new_match;
                new_match = NULL;
                continue;
            } // 否则继续
            router_stack[stack_place++] = port_to_router[next_port_num];
            have_been[port_to_router[next_port_num]] = true;
            dfs_search_with_path(port_to_router[next_port_num], destiny, new_match, need_loop, need_print);
            router_stack[stack_place--] = -1;
            have_been[port_to_router[next_port_num]] = false;
        }
        delete new_match;
        new_match = NULL;
    }
}

void Network::brutal_force(bool need_print) {
    // init
    memset(have_been, false, router_max);
    std::set<uint64_t> full_rules;
    for (uint64_t i = 0; i < ap_num; i++) {
        full_rules.insert(i);
    }

    for (int i = 0; i < r_num; i++) {
        for(int j = 0; j < r_num; j++) {
            have_been[i] = true;
            start = i;
            dfs_search(i, j, &full_rules, need_print);
            memset(have_been, false, router_max);  // clear state
        }
    }
    if(need_print) {
        printf("Now printing the result of brute force no path:\n");
        print_matrix(3);
    }
}

void Network::dfs_search(int router, int destiny, std::set<uint64_t>* rules, bool need_print) {
    set< uint64_t >* new_match;
    for (auto it = routers[router].port_to_match.begin(); it != routers[router].port_to_match.end(); it++) {
        if (topology.count(it->first) == 0)
            continue;
        new_match = new set<uint64_t >;
        uint64_t port_num = it->first;
        uint64_t next_port_num = topology[port_num];
        std::set_intersection((*rules).begin(), (*rules).end(),
                              (*routers[router].port_to_match[port_num]).begin(),
                              (*routers[router].port_to_match[port_num]).end(),
                              std::inserter(*new_match, new_match->begin()));
        // 如果是空集，直接就跳下一个port
        if (new_match->empty()) {
            delete new_match;
            new_match = NULL;
            continue;
        }
        // 如果不是空集...
        if (port_to_router[next_port_num] == destiny) { // 如果到了目的地，应该总是第一次到，否则在前边就会记录
            // 首先将这个match加入
            b_matrix[start][destiny].insert(new_match->begin(), new_match->end());
        } else {
            if (have_been[port_to_router[next_port_num]]) {  // 遇到了loop，直接continue
                delete new_match;
                new_match = NULL;
                continue;
            } // 否则继续
            have_been[port_to_router[next_port_num]] = true;
            dfs_search(port_to_router[next_port_num], destiny, new_match, need_print);
            have_been[port_to_router[next_port_num]] = false;
        }
        delete new_match;
        new_match = NULL;
    }
}

void print_set(set< uint64_t > *rule) {
    printf("{");
    for (auto it = rule->begin(); it != rule->end(); it++) {
        printf("%llu ", *it);
    }
    printf("}\n");
}

void Network::print_matrix(int k) {
    for (int i = 0; i < r_num; i++) {
        for (int j = 0; j < r_num; j++) {
            printf("Router %d to Router %d: ", routers[i].getid(), routers[j].getid());
            switch (k) {
                case 1:
                    print_set(&matrix1[i][j]);
                    break;
                case 2:
                    print_set(&matrix2[i][j]);
                    break;
                case 3:
                    print_set(&b_matrix[i][j]);
                    break;
                case 4:
                    print_set(&matrix3[i][j]);
                    break;
                default:
                    break;
            }
        }
    }
    printf("=======================\n");
}

void Network::warshall_no_path(bool need_print)
{
    bool is_height[router_max] = {false};
    bool is_width[router_max] = {false};
    for (int i = 0; i < r_num; i++) {
        for (auto it = routers[i].port_to_match.begin(); it != routers[i].port_to_match.end(); it++) {
            if (topology.count(it->first) == 0)
                continue;
            int router2 = port_to_router[topology[it->first]];
            this->matrix1[i][router2].insert(it->second->begin(), it->second->end());
            is_height[i] = true;
            is_width[router2] = true;
        }
    }
    int mini_matrix[router_max][router_max][2];
    int mini_height = 0;
    int mini_width = 0;
    for(int i = 0; i < r_num; i++)
    {
        if(!is_height[i])
            continue;

        mini_width = 0;
        for(int j = 0; j < r_num; j++)
        {
            if(!is_width[j])
                continue;
            else
            {
                mini_matrix[mini_height][mini_width][0] = i;
                mini_matrix[mini_height][mini_width][1] = j;
                mini_width++;
            }
        }
        mini_height++;
    }

    int rheight, rwidth;
    int k = 0;
    for (k = 0; k < r_num; k++) {
        if (k % 2 == 1) {
            for (int i = 0; i < mini_height; i++) {
                for (int j = 0; j < mini_width; j++) {
                    rheight = mini_matrix[i][j][0];
                    rwidth = mini_matrix[i][j][1];
                    matrix1[rheight][j].clear();
                    set_intersection(matrix2[rheight][k].begin(), matrix2[rheight][k].end(),
                                     matrix2[k][rwidth].begin(), matrix2[k][rwidth].end(),
                                     inserter(matrix1[rheight][rwidth], matrix1[rheight][rwidth].begin()));
                    set_union(matrix2[rheight][rwidth].begin(), matrix2[rheight][rwidth].end(),
                              matrix1[rheight][rwidth].begin(), matrix1[rheight][rwidth].end(),
                              inserter(matrix1[rheight][rwidth], matrix1[rheight][rwidth].begin()));
                    // matrix1[rheight][rwidth].insert(matrix2[rheight][rwidth].begin(), matrix2[rheight][rwidth].end());
                }
            }
        }
        else {
            for (int i = 0; i < mini_height; i++) {
                for (int j = 0; j < mini_width; j++) {
                    rheight = mini_matrix[i][j][0];
                    rwidth = mini_matrix[i][j][1];
                    matrix2[rheight][rwidth].clear();
                    set_intersection(matrix1[rheight][k].begin(), matrix1[rheight][k].end(),
                                     matrix1[k][rwidth].begin(), matrix1[k][rwidth].end(),
                                     inserter(matrix2[rheight][rwidth], matrix2[rheight][rwidth].begin()));
                    set_union(matrix2[rheight][rwidth].begin(), matrix2[rheight][rwidth].end(),
                              matrix1[rheight][rwidth].begin(), matrix1[rheight][rwidth].end(),
                              inserter(matrix2[rheight][rwidth], matrix2[rheight][rwidth].begin()));
                    // matrix2[rheight][rwidth].insert(matrix1[rheight][rwidth].begin(), matrix1[rheight][rwidth].end());
                }
            }
        }
    }
//    if (r_num % 2 == 1) {
//        for (int i = 0; i < r_num; i++) {
//            for (int j = 0; j < r_num; j++) {
//                matrix1[i][j] = matrix2[i][j];
//            }
//        }
//    }
    if (need_print) {
        printf("Now printing the result of warshall no path:\n");
        print_matrix(1);
    }
}

void Network::warshall_with_path(bool need_print)
{
    //先算一个基本的r0，然后推导到rk
    bool is_height[router_max] = {false};
    bool is_width[router_max] = {false};

    for(int i = 0; i < r_num; i++)
    {
        std::map< uint64_t, std::set<uint64_t>* >::iterator it;
        for(it = routers[i].port_to_match.begin(); it != routers[i].port_to_match.end(); it++)
        {
            if (topology.count(it->first) == 0)
                continue;
            int router1 = i;
            int router2 = port_to_router[topology[it->first]];
            std::list<uint32_t>* tmp;
            tmp = new std::list<uint32_t>;
            (*tmp).push_back(routers[router1].getid());
            (*tmp).push_back(routers[router2].getid());
            rmatrix[router1][router2].set_path_to_packets(tmp, it->second);
            is_height[router1] = true;
            is_width[router2] = true;
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
    int rheight, rwidth;
    for(int k = 0; k < r_num; k++)
    {
        if(k % 2 == 1)
        {
            for(int i = 0; i < height; i++)
            {
                for(int j = 0; j < width; j++)
                {
                    rheight = minimatrix[i][j][0];
                    rwidth = minimatrix[i][j][1];
                    // rheight = i;
                    // rwidth = j;
                    if(rheight == k || rwidth == k)
                    {
                        rmatrix1[rheight][rwidth] = rmatrix2[rheight][rwidth];
                        continue;
                    }
                    rmatrix1[rheight][rwidth] = rmatrix2[rheight][k] * rmatrix2[k][rwidth];
                    rmatrix1[rheight][rwidth] = rmatrix1[rheight][rwidth] + rmatrix2[rheight][rwidth];
                }
            }
        }
        else if(k % 2 == 0)
        {
            for(int i = 0; i < height; i++)
            {
                for(int j = 0; j < width; j++)
                {
                    rheight = minimatrix[i][j][0];
                    rwidth = minimatrix[i][j][1];
                    // rheight = i;
                    // rwidth = j;
                    if(rheight == k || rwidth == k)
                    {
                        rmatrix2[rheight][rwidth] = rmatrix1[rheight][rwidth];
                        continue;
                    }
                    rmatrix2[rheight][rwidth] = rmatrix1[rheight][k] * rmatrix1[k][rwidth];
                    rmatrix2[rheight][rwidth] = rmatrix2[rheight][rwidth] + rmatrix1[rheight][rwidth];
                }
            }
        }
    }
    if(need_print)
    {
        printf("Now printing the result of warshall with path:\n");
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
}

void Network::segment_no_path(bool need_print)
{
    // init_adj_matrix();
    bool is_height[router_max] = {false};
    bool is_width[router_max] = {false};
    for (int i = 0; i < r_num; i++) {
        for (auto it = routers[i].port_to_match.begin(); it != routers[i].port_to_match.end(); it++) {
            if (topology.count(it->first) == 0)
                continue;
            int router2 = port_to_router[topology[it->first]];
            this->matrix1[i][router2].insert(it->second->begin(), it->second->end());
            is_height[i] = true;
            is_width[router2] = true;
        }
    }

    int mini_matrix[router_max][router_max][2];
    int mini_height = 0;
    int mini_width = 0;
    for(int i = 0; i < r_num; i++)
    {
        if(!is_height[i])
            continue;

        mini_width = 0;
        for(int j = 0; j < r_num; j++)
        {
            if(!is_width[j])
                continue;
            else
            {
                mini_matrix[mini_height][mini_width][0] = i;
                mini_matrix[mini_height][mini_width][1] = j;
                mini_width++;
            }
        }
        mini_height++;
    }

    set< uint64_t > matrix[router_max][router_max];

    set< uint64_t > adder, muler;
    for (int i = 0; i < r_num; i++) {
        for (int j = 0; j < r_num; j++) {
            matrix3[i][j].insert(matrix1[i][j].begin(), matrix1[i][j].end());
            matrix[i][j].insert(matrix1[i][j].begin(), matrix1[i][j].end());
        }
    }
    // matrix1是原始的临接矩阵

    int rheight, rwidth;
    for (int k = 0; k < r_num; k++) {
        if (k % 2 == 0) {
            // k为偶数时算matrix2
            for (int i = 0; i < mini_height; i++) {
                for (int j = 0; j < mini_width; j++) {
                    // rheight = i;
                    // rwidth = j;
                    rheight = mini_matrix[i][j][0];
                    rwidth = mini_matrix[i][j][1];
                    matrix2[rheight][rwidth].clear();
                    for (int m = 0; m < r_num; m++) {
                        set_intersection(matrix[rheight][m].begin(), matrix[rheight][m].end(),
                                         matrix1[m][rwidth].begin(), matrix1[m][rwidth].end(),
                                         inserter(matrix2[rheight][rwidth], matrix2[rheight][rwidth].begin()));
                    }
                    matrix3[rheight][rwidth].insert(matrix2[rheight][rwidth].begin(), matrix2[rheight][rwidth].end());
                }
            }
//            if(need_print)
//                print_matrix(2);
        } else {
            // k为奇数时算matrix1
            for (int i = 0; i < mini_height; i++) {
                for (int j = 0; j < mini_width; j++) {
                    // rheight = i;
                    // rwidth = j;
                    rheight = mini_matrix[i][j][0];
                    rwidth = mini_matrix[i][j][1];
                    matrix1[i][j].clear();
                    for (int m = 0; m < r_num; m++) {
                        set_intersection(matrix[rheight][m].begin(), matrix[rheight][m].end(),
                                         matrix2[m][rwidth].begin(), matrix2[m][rwidth].end(),
                                         inserter(matrix1[rheight][j], matrix1[rheight][j].begin()));
                    }
                    matrix3[rheight][j].insert(matrix1[rheight][rwidth].begin(), matrix1[rheight][rwidth].end());
                }
            }
//            if(need_print)
//                print_matrix(1);
        }
    }
    if (need_print) {
        printf("Now printing the result of segment no path:\n");
        print_matrix(4);
    }
}

void Network::segment_based(bool need_print)
{
    //先算一个基本的r0，然后推导到rk
    bool is_height[router_max] = {false};
    bool is_width[router_max] = {false};

    for(int i = 0; i < r_num; i++)
    {
        std::map< uint64_t, std::set<uint64_t>* >::iterator it;
        for(it = routers[i].port_to_match.begin(); it != routers[i].port_to_match.end(); it++)
        {
            if (topology.count(it->first) == 0)
                continue;
            int router1 = i;
            int router2 = port_to_router[topology[it->first]];
            std::list<uint32_t>* tmp;
            tmp = new std::list<uint32_t>;
            (*tmp).push_back(routers[router1].getid());
            (*tmp).push_back(routers[router2].getid());
            rmatrix[router1][router2].set_path_to_packets(&(*tmp), it->second);
            is_height[router1] = true;
            is_width[router2] = true;
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
        {
            rmatrix2[i][j] = rmatrix[i][j];
            rmatrix3[i][j] = rmatrix[i][j];
        }

    Reachability tmp;
    int rheight, rwidth, rplace;
    //rmatrix3用来存总的，rmatrix1和rmatrix2是分的
    for(int k = 3; k <= r_num; k++)
    {
        if(k % 2 == 1)
        {
            for(int i = 0; i < height; i++)
            {
                for(int j = 0; j < width; j++)
                {
                    // rheight = i;
                    // rwidth = j;
                    rheight = minimatrix[i][j][0];
                    rwidth = minimatrix[i][j][1];
                    rmatrix1[rheight][rwidth].delete_all();
                    for(int place = 0; place < width; place++)
                    {
                        tmp.delete_all();
                        //rplace = place;
                        rplace = minimatrix[0][place][1];
                        if(rplace == rwidth)
                            continue;
                        tmp = rmatrix[rheight][rplace] * rmatrix2[rplace][rwidth];
                        rmatrix1[rheight][rwidth] = rmatrix1[rheight][rwidth] + tmp;
                    }
                    rmatrix3[rheight][rwidth] = rmatrix3[rheight][rwidth] + rmatrix1[rheight][rwidth];
                }
            }
        }
        else if(k % 2 == 0)
        {
            for(int i = 0; i < height; i++)
            {
                for(int j = 0; j < width; j++)
                {
                    // rheight = i;
                    // rwidth = j;
                    rheight = minimatrix[i][j][0];
                    rwidth = minimatrix[i][j][1];
                    rmatrix2[rheight][rwidth].delete_all();
                    for(int place = 0; place < width; place++)
                    {
                        tmp.delete_all();
                        //rplace = place;
                        rplace = minimatrix[0][place][1];
                        if(rplace == rwidth)
                            continue;
                        tmp = rmatrix[rheight][rplace] * rmatrix1[rplace][rwidth];
                        rmatrix2[rheight][rwidth] = rmatrix2[rheight][rwidth] + tmp;
                    }
                    rmatrix3[rheight][rwidth] = rmatrix3[rheight][rwidth] + rmatrix2[rheight][rwidth];
                }
            }
        }
    }
    if(need_print)
    {
        printf("Now printing the result of segment with path:\n");
        for(int i = 0; i < r_num; i++)
        {
            for(int j = 0; j < r_num; j++)
            {
                printf("matrix: %d : %d \n", i, j);
                rmatrix3[i][j].show_path_to_packets();
            }
        }
    }
}

void Network::rule_based(bool need_print)
{
    for(uint64_t i = 0; i < rule_type; i++)
    {
        uint64_t portnum = 0;
        int router_place = 0;
        for(int j = 0; j < r_num; j++)
        {
            router_place = j;

            string list_str = "";
            std::list<uint32_t> router_list;
            bool rhave_been[router_max] = {false};

            list_str = list_str + to_string(routers[j].getid());
            router_list.push_back(routers[j].getid());
            rhave_been[j] = true;

            while(1)
            {
                bool flag = true;
                std::map< uint64_t, std::set<uint64_t>* >::iterator it;
                it = routers[router_place].port_to_match.begin();
                while(it != routers[router_place].port_to_match.end())
                {
                    if((*it->second).count(i) != 0)
                    {
                        if(topology.count(it->first) == 0)
                        {
                            it++;
                            continue;
                        }
                        portnum = it->first;
                        flag = false;
                        break;
                    }
                    it++;
                }
                if(flag)//to the end
                    break;
                router_place = port_to_router[topology[portnum]];
                if(router_place == j)
                {
                    //loop!
                    list_str = list_str + to_string(routers[router_place].getid());
                    router_list.push_back(routers[router_place].getid());
                    rulebased.set_new_rule(list_str, &router_list, i);
                    break;
                }
                else if(rhave_been[router_place])
                {
                    //not loop~
                    break;
                }
                list_str = list_str + to_string(routers[router_place].getid());
                router_list.push_back(routers[router_place].getid());
                rhave_been[router_place] = true;
                rulebased.set_new_rule(list_str, &router_list, i);
            }
        }
    }

    if(need_print) {
        printf("Now printing the result of rule-based:\n");
        rulebased.print_rule_map();
    }
}
