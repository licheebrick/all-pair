#include "network.h"
#include "router.h"
#include "utils.h"
#include <json/json.h>
#include <iostream>
#include <fstream>
#include <dirent.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

using namespace std;

bool show_detail = false;

void load_topology_from_file(string file_path, Network *N) {
    ifstream jsfile;
    Json::Value root;
    Json::Reader reader;
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
}

void load_network_from_dir(string file_path, Network *N) {
    ifstream jsfile;
    Json::Value root;
    Json::Reader reader;
    int router_counter = 0;
    string file_name;
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

set< uint64_t > Network::matrix1[router_max][router_max];
set< uint64_t > Network::matrix2[router_max][router_max];
set< uint64_t > Network::matrix3[router_max][router_max];
set< uint64_t > Network::b_matrix[router_max][router_max];

int main(int argc, char* argv[])
{
    /*
    rule_num: REVISE WHEN CHANGE DATASET!!!
    simple_with_loop: 4;    simple_no_loop: 5;
    fattree-4: 241;         stanford: 133;
    internet2: 38;          fattree-8: 16001;
    router_max: REVISE WHEN CHANGE DATASET!!!
    simple_with_loop: 4;    simple_no_loop: 4;
    fattree-4: 20;          stanford: 16;
    internet2: 18;          fattree-8: 80;
    */
    int mode = 1;
    int algr = 1;
    string json_files_path;
    int hdr_len = 1;    // network header length
    int var_num;    // BDD variable number
    bool print_result = false;

    for (int i = 1; i < argc; i++) {
        if ( strcmp(argv[i] , "--help") == 0 ) {
            printf("Usage: all-pair [run option(s)][settings]\n");
            printf("  run options:\n");
            printf("\t --mode <mode> : running mode.\n");
            printf("\t \t 1: load non-atomic rules; 2: load atomic rules; 3: generate atomic rules to file.\n");
            printf("\t --load <path> : load the rules from json files in the <path>. Don't forget to revise router_max and rule_type in network.h and rebuild when change test ruleset.\n");
            printf("\t --algorithm <algr> : The algorithm used to calculate all-pair reachability.\n");
            printf("\t \t 1: brutal_force_with_path; 2: brute_force_without_path.\n");
            printf("\t \t 3: warshall_with_path; 4: warshall_without_path.\n");
            printf("\t \t 5: segment_with_path; 6: segment_without_path.\n");
            printf("\t \t 7: rule_based.\n");

            printf("  settings:\n");
            printf("\t --hdr-len <length> : <length> of packet header (default is 1 byte).\n");
            printf("\t -detailed : show detailed router information in console.\n");
            return -1;
            break;
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
                printf("Please specify algorithm chosen after --algorithm.\n");
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
        }

        if ( strcmp(argv[i],"--mode") == 0 ) {
            if (i+1 >= argc) {
                printf("Please specify mode of the program after --mode.\n");
                return -1;
            }
            mode = atoi(argv[++i]);
        }

        if ( strcmp(argv[i], "-detailed") == 0) {
            print_result = true;
        }
    }

    var_num = 8 * hdr_len;

    Network network_example;
    network_example.set_hdr_len(hdr_len);

    string atomic_file = json_files_path + "/rules.json";
    string port2rtr_file = json_files_path + "/ports.json";

    load_topology_from_file(json_files_path, &network_example);

    if (mode == 1)
    {
        // prepare bdd basics
        bdd_init(10000000, 10000000);
        bdd_setvarnum(var_num);
        load_network_from_dir(json_files_path, &network_example);
    }

    if (mode == 3)
    {
        // use next three lines to generate atomized rules.
        load_network_from_dir(json_files_path, &network_example);
        network_example.dump_ap_rules_to_file(atomic_file);
        network_example.dump_port_to_router_to_file(port2rtr_file);
        printf("Finish generate and dump atomic rules to file.\n");
    }

    if (mode == 2) {
        network_example.load_port_to_router_from_file(port2rtr_file);
        network_example.load_ap_rules_from_file(atomic_file);
    }

    clock_t startTime,endTime;
    clock_t inter_time1, inter_time2, inter_time3, inter_time4, inter_time5, inter_time6, inter_time7;
    startTime = clock();

    switch (algr) {
        case 1:
            inter_time1 = clock();
            network_example.brutal_force_with_path(print_result, false);
            inter_time2 = clock();
            printf("Brute force with path Total Time : %f s \n", (double)(inter_time2 - inter_time1) / CLOCKS_PER_SEC);
            break;
        case 2:
            inter_time1 = clock();
            network_example.brutal_force(print_result);
            inter_time2 = clock();
            printf("Brute force Total Time :           %f s \n", (double)(inter_time2 - inter_time1) / CLOCKS_PER_SEC);
            break;
        case 3:
            inter_time1 = clock();
            network_example.warshall_with_path(print_result);
            inter_time2 = clock();
            printf("Warshall with path Total Time :    %f s \n", (double)(inter_time2 - inter_time1) / CLOCKS_PER_SEC);
            break;
        case 4:
            inter_time1 = clock();
            network_example.warshall_no_path(print_result);
            inter_time2 = clock();
            printf("Warshall no path Total Time :      %f s \n", (double)(inter_time2 - inter_time1) / CLOCKS_PER_SEC);
            break;
        case 5:
            inter_time1 = clock();
            network_example.segment_based(print_result);
            inter_time2 = clock();
            printf("Segment with path Total Time :     %f s \n", (double)(inter_time2 - inter_time1) / CLOCKS_PER_SEC);
            break;
        case 6:
            inter_time1 = clock();
            network_example.segment_no_path(print_result);
            inter_time2 = clock();
            printf("Segment no path Total Time :       %f s \n", (double)(inter_time2 - inter_time1) / CLOCKS_PER_SEC);
            break;
        case 7:
            inter_time1 = clock();
            network_example.rule_based(print_result);
            inter_time2 = clock();
            printf("Rule Based Total Time :            %f s \n", (double)(inter_time2 - inter_time1) / CLOCKS_PER_SEC);            printf("%f s \n", (double)(inter_time2 - inter_time1) / CLOCKS_PER_SEC);
            break;
        case 8:
            clock_t inter_time11;
            inter_time11 = clock();
            network_example.brutal_force_with_path();
            clock_t inter_time12;
            inter_time12 = clock();
            printf("Brute force with path Total Time : %f s \n", (double)(inter_time12 - inter_time11) / CLOCKS_PER_SEC);
            network_example.refresh_matrix();

            clock_t inter_time21;
            inter_time21 = clock();
            network_example.brutal_force();
            clock_t inter_time22;
            inter_time22 = clock();
            printf("Brute force Total Time :           %f s \n", (double)(inter_time22 - inter_time21) / CLOCKS_PER_SEC);
            network_example.refresh_matrix();

            clock_t inter_time31;
            inter_time31 = clock();
            network_example.warshall_with_path();
            clock_t inter_time32;
            inter_time32 = clock();
            printf("Warshall with path Total Time :    %f s \n", (double)(inter_time32 - inter_time31) / CLOCKS_PER_SEC);
            network_example.refresh_matrix();

            clock_t inter_time41;
            inter_time41 = clock();
            network_example.warshall_no_path();
            clock_t inter_time42;
            inter_time42 = clock();
            printf("Warshall no path Total Time :      %f s \n", (double)(inter_time42 - inter_time41) / CLOCKS_PER_SEC);
            network_example.refresh_matrix();

            clock_t inter_time51;
            inter_time51 = clock();
            network_example.segment_based();
            clock_t inter_time52;
            inter_time52 = clock();
            printf("Segment with path Total Time :     %f s \n", (double)(inter_time52 - inter_time51) / CLOCKS_PER_SEC);
            network_example.refresh_matrix();

            clock_t inter_time71;
            inter_time71 = clock();
            network_example.segment_no_path();
            clock_t inter_time72;
            inter_time72 = clock();
            printf("Segment no path Total Time :       %f s \n", (double)(inter_time72 - inter_time71) / CLOCKS_PER_SEC);
            network_example.refresh_matrix();

            clock_t inter_time61;
            inter_time61 = clock();
            network_example.rule_based();
            clock_t inter_time62;
            inter_time62 = clock();
            printf("Rule Based Total Time :            %f s \n", (double)(inter_time62 - inter_time61) / CLOCKS_PER_SEC);
            network_example.refresh_matrix();
            break;
        default:
            printf("1: brutal_force_with_path; 2: brutal_force; 3: warshall_with_path; 4: warshall_no_path; 5: rule_based; 6:segment_based; 7:segment_no_path 8: all.\n");
    }

    endTime = clock();
    printf("Total Time : %f s \n", (double)(endTime - startTime) / CLOCKS_PER_SEC);

    if (mode == 1)
        bdd_done();

    return 0;
}
