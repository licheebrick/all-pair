#include "network.h"
#include "router.h"
#include <json/json.h>
#include <iostream>
#include <fstream>
#include <string.h>

using namespace std;

const int router_num = 4;
string json_file_path = "./jsonfile";

int main(int argc, char* argv[]) 
{
    Network network_example(router_num);

    //read topology
    ifstream jsfile;
    Json::Value root;
    Json::Reader reader;
    string file_name = json_file_path + "/" + "topology.json";
    jsfile.open(file_name.c_str());
    if (!jsfile.good()) 
    {
        cout << "Error opening the file " << file_name << endl;
    }
    reader.parse(jsfile, root, false);
    Json::Value topology = root["topology"];
    for (unsigned i = 0; i < topology.size(); i++) 
    {
        network_example.add_link(topology[i]["src"].asUInt64(), topology[i]["dst"].asUInt64());
    }
    jsfile.close();

    network_example.print_topology();
    network_example.init();
    network_example.all_pair_reachability();
    
    printf("ready for it!\n");
    return 0;
}