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

bool show_detail = false;

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
    int algr = 5;       // algorithm used for reachability calculation
    int dataset = 3;

    // test suite:
    string file_path[4] = {"./examples/simple_with_loop", "./examples/FatTree-4",
            "./examples/stanford_ip_fwd", "./examples/internet2"};
    int hdr[4] = {1, 16, 4, 6};

    string json_files_path = file_path[dataset];
    int hdr_len = hdr[dataset];    // network header length
    int var_num = 8 * hdr_len;    // BDD variable number


    for (int i = 1; i < argc; i++) {
        if ( strcmp(argv[i] , "--help") == 0 ) {
            printf("Usage: all-pair [run option(s)][settings]\n");
            printf("  run options:\n");
            printf("\t --test  runs all the unit tests.\n");
            printf("\t --load <path> : load the rules from json files in the <path>.\n");
            printf("\t --algorithm <algr> : The algorithm used to calculate all-pair reachability.\n");
            printf("\t \t 1: brutal_force; 2: warshall_record_path.\n");

            printf("  settings:\n");
            printf("\t --hdr-len <length> : <length> of packet header (default is 1 byte).\n");
            printf("\t -detailed : show detailed router information in console.\n");
            break;
        }
        if ( strcmp(argv[i],"--test") == 0 ) {
            do_run_test = true;
        }

        if ( strcmp(argv[i],"--load") == 0)  {
            if (i+1 >= argc) {
                printf("Please specify path to json files after --load.\n");
                return -1;
            }
            json_files_path = string(argv[++i]);
        }

        if ( strcmp(argv[i],"--algorithm") == 0)  {
            if (i+1 >= argc) {
                printf("Please specify algorithm chosen after --load.\n");
                return -1;
            }
            algr = atoi(argv[++i]);
        }

        if ( strcmp(argv[i],"--hdr-len") == 0 ) {
            if (i+1 >= argc) {
                printf("Please specify length of header after --hdr-len.\n");
                return -1;
            }
            hdr_len = atoi(argv[++i]);
            var_num = 8 * hdr_len;
        }

        if ( strcmp(argv[i], "-detailed") == 0) {
            show_detail = true;
        }
    }

    // prepare bdd basics
    bdd_init(1000000, 1000000);
    bdd_setvarnum(var_num);

    Network network_example;
    network_example.set_hdr_len(hdr_len);

    load_network_from_dir(json_files_path, &network_example);

    //network_example.print_topology();
  
    clock_t startTime,endTime;
    clock_t inter_time1, inter_time2, inter_time3, inter_time4;  
    startTime = clock();
    // network_example.init();
    switch (algr) {
        case 1:
            network_example.brutal_force(true);
            break;
        case 2:
            network_example.warshall_with_path(true);
            break;
        case 3:
            network_example.segment_based(true);
            break;
        case 4:
            network_example.rule_based(true);
            break;
        case 5:
            network_example.brutal_force();
            inter_time1 = clock();
            printf("Total Time : %f s \n", (double)(inter_time1 - startTime) / CLOCKS_PER_SEC);
            network_example.warshall_with_path();
            inter_time2 = clock();
            printf("Total Time : %f s \n", (double)(inter_time2 - inter_time1) / CLOCKS_PER_SEC);
            network_example.refresh_matrix();
            inter_time2 = clock();
            network_example.segment_based();
            inter_time3 = clock();
            printf("Total Time : %f s \n", (double)(inter_time3 - inter_time2) / CLOCKS_PER_SEC);
            network_example.rule_based();
            inter_time4 = clock();
            printf("Total Time : %f s \n", (double)(inter_time4 - inter_time3) / CLOCKS_PER_SEC);
            break;
        default:
            printf("1: brutal_force; 2: warshall_record_path; 3: segment_based; 4: rule_based; 5: all.\n");
    }

    bdd_done();

    endTime = clock();
    printf("Total Time : %f s \n", (double)(endTime - startTime) / CLOCKS_PER_SEC);

    return 0;
}