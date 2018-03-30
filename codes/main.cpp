#include "network.h"
#include "router.h"
#include "utils.h"
#include <json/json.h>
#include <iostream>
#include <fstream>
#include <dirent.h>
#include <string.h>
#include <time.h>

using namespace std;

bool show_detail = true;

void load_network_from_dir(string file_path, Network *N) {
    ifstream jsfile;
    Json::Value root;
    Json::Reader reader;
    int router_counter = 0;

    // load topology
    string file_name = file_path + "/" + "topology.json";
    jsfile.open(file_name.c_str());
    if (!jsfile.good())
    {
        cout << "Error opening the file " << file_name << endl;
    }
    reader.parse(jsfile, root, false);
    Json::Value topology = root["topology"];
    for (unsigned i = 0; i < topology.size(); i++)
    {
        N->add_link(topology[i]["src"].asUInt64(), topology[i]["dst"].asUInt64());
    }
    jsfile.close();

    // load ruleset
    struct dirent *ent;
    DIR *dir = opendir(file_path.c_str());
    if (dir != NULL) {
        while ((ent = readdir(dir)) != NULL) {
            long run_time = 0;
            file_name = string(ent->d_name);
            if (file_name.find(".rules.json") != string::npos ||
                file_name.find(".tf.json") != string::npos) {
                file_name = file_path + "/" + file_name;
                jsfile.open(file_name.c_str());
                reader.parse(jsfile, root, false);
                uint32_t router_id = root["id"].asUInt();
                N->add_then_load_router(router_id, &root);
                jsfile.close();
                router_counter++;
            }
        }
    }
    printf("-----------------------------------------------------------------\n");

    // pre-process ruleset
    N->make_atomic_predicates();
    N->convert_router_to_ap();
}

int main(int argc, char* argv[]) 
{
    // running configs
    bool do_run_test = true;
    int algr = 7;       // algorithm used for reachability calculation
    int dataset = 3;
    /*
        rule_num: REVISE WHEN CHANGE DATASET!!!
        simple_with_loop: 4; simple_no_loop: 5;
        fattree-4: 241;   stanford: 133
        internet2: 38
        router_max: REVISE WHEN CHANGE DATASET!!!
        simple_with_loop: 4; simple_no_loop: 4;
        fattree-4: 20;   stanford: 16
        internet2: 18
     */
    // test suite:
    string file_path[5] = {"./examples/simple_with_loop", "./examples/simple_no_loop", "./examples/FatTree-4",
            "./examples/stanford_ip_fwd", "./examples/internet2"};
    int hdr[5] = {1, 1, 16, 4, 6};

    string json_files_path = file_path[dataset];
    int hdr_len = hdr[dataset];    // network header length
    int var_num = 8 * hdr_len;    // BDD variable number

    // prepare bdd basics
    bdd_init(1000000, 1000000);
    bdd_setvarnum(var_num);

    Network network_example;
    network_example.set_hdr_len(hdr_len);

    load_network_from_dir(json_files_path, &network_example);

    //network_example.print_topology();

    clock_t startTime,endTime;
    clock_t inter_time1, inter_time2, inter_time3, inter_time4, inter_time5, inter_time6, inter_time7;  
    startTime = clock();

    switch (algr) {
        case 1:
            network_example.brutal_force_with_path(true, true);
            break;
        case 2:
            network_example.brutal_force(true);
            break;
        case 3:
            network_example.warshall_with_path(true);
            break;
        case 4:
            network_example.warshall_no_path(true);
            break;
        case 5:
            network_example.rule_based(true);
            break;
        case 6:
            network_example.segment_based(true);
            break;
        case 7:
            network_example.segment_no_path(true);
            break;
        case 8:
            network_example.brutal_force_with_path();
            inter_time1 = clock();
            printf("Brute force with path Total Time : %f s \n", (double)(inter_time1 - startTime) / CLOCKS_PER_SEC);
            network_example.refresh_rmatrix();

            inter_time1 = clock();
            network_example.brutal_force();
            inter_time2 = clock();
            printf("Brute force Total Time :           %f s \n", (double)(inter_time2 - inter_time1) / CLOCKS_PER_SEC);
            network_example.refresh_rmatrix();

            inter_time2 = clock();
            network_example.warshall_with_path();
            inter_time3 = clock();
            printf("Warshall with path Total Time :    %f s \n", (double)(inter_time3 - inter_time2) / CLOCKS_PER_SEC);
            network_example.refresh_rmatrix();

            inter_time3 = clock();
            network_example.segment_based();
            inter_time4 = clock();
            printf("Segment with path Total Time :     %f s \n", (double)(inter_time4 - inter_time3) / CLOCKS_PER_SEC);

            network_example.rule_based();
            inter_time5 = clock();
            printf("Rule Based Total Time :            %f s \n", (double)(inter_time5 - inter_time4) / CLOCKS_PER_SEC);

            network_example.warshall_no_path();
            inter_time6 = clock();
            printf("Warshall no path Total Time :      %f s \n", (double)(inter_time6 - inter_time5) / CLOCKS_PER_SEC);
            
            network_example.refresh_matrix();
            inter_time6 = clock();
            network_example.segment_no_path();
            inter_time7 = clock();
            printf("Segment no path Total Time :       %f s \n", (double)(inter_time7 - inter_time6) / CLOCKS_PER_SEC);
            break;
        default:
            printf("1: brutal_force_with_path; 2: brutal_force; 3: warshall_with_path; 4: warshall_no_path; 5: rule_based; 6:segment_based; 7:segment_no_path 8: all.\n");
    }

    bdd_done();

    endTime = clock();
    printf("Total Time : %f s \n", (double)(endTime - startTime) / CLOCKS_PER_SEC);

    return 0;
}